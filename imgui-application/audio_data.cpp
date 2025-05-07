#include "audio_data.h"
#include "helpers.h"
#include <cstring> //strcpy
#include <cstdlib> //system()
#include <fstream>
#include <filesystem>
#include <vector>
#include <iostream>

#ifdef _WIN32
	const char* AudioData::voiceEngineArray[4] = { "Balabolka", "Espeak", "Espeak NG", "Windows Narrator (TODO)" };
	const char* AudioData::voiceEngineArray_exeForUpdatingVoiceList[4] = { "\"balcon\" -l", "\"espeak\" --voices=en", "\"espeak-ng\" --voices=en", "" }; //note: internal use only
	//TODO: look into whether nvaccess/nvda has compatible command line support
#else
	const char* AudioData::voiceEngineArray[5] = { "Espeak", "Espeak NG", "say (TODO)", "spd-say (TODO)", "Festival (TODO)" }; //TODO: https://askubuntu.com/questions/501910/how-to-text-to-speech-output-using-command-line/501917#501917
	const char* AudioData::voiceEngineArray_exeForUpdatingVoiceList[5] = { "espeak --voices", "espeak-ng --voices", "", "", "" }; //note: internal use only
#endif

const char* AudioData::audioEncoderArray[7] = { "copy (pcm)", "AAC", "Opus", "FLAC", "Vorbis", "MP3", "ALAC" };
std::vector<const char*> AudioData::audioPresetArray_AAC =  { "default", "anmr (not recommended) (NOT SUPPORTED)", "twoloop (default)", "fast" };
std::vector<const char*> AudioData::audioPresetArray_Opus = { "default", "0 (fast)", "1", "2", "3", "4", "5",           "6", "7", "8", "9", "10 (default)" };
std::vector<const char*> AudioData::audioPresetArray_FLAC = { "default", "0 (fast)", "1", "2", "3", "4", "5 (default)", "6", "7", "8", "9", "10", "11", "12" };
std::vector<const char*> AudioData::audioPresetArray_MP3 =  { "default", "0 (slow)", "1", "2", "3", "4", "5 (default)", "6", "7", "8", "9" }; //default is 5 according to its API
std::vector<const char*> AudioData::audioPresetArray_empty = {};

const std::unordered_map<std::string, CodecPresetInformation> AudioData::codecToPresetArray = {
	{ "copy (pcm)", { "", audioPresetArray_empty } },
	{ "AAC",        { "Encoding Method", audioPresetArray_AAC } },
	{ "Opus",       { "Compression Level", audioPresetArray_Opus } },
	{ "FLAC",       { "Compression Level", audioPresetArray_FLAC } },
	{ "Vorbis",     { "", audioPresetArray_empty } },
	{ "MP3",        { "Compression Level", audioPresetArray_MP3 } },
	{ "ALAC",       { "", audioPresetArray_empty } },
};

//the recommended range is mostly a guess; maybe quality values should be used instead
const std::unordered_map<std::string, AudioData::BitrateData> AudioData::codecToBitrate {
	{ "copy (pcm)", {  -1,  -1,  -1,  -1,  -1,  -1 } },
	{ "AAC",        { 128,  30, 192, 128,   0, INT16_MAX } }, //codec clamps bitrate when too high/low
	{ "Opus",       {  96,  30, 128,  64,   1,  256 } }, //actual minimum is .5k
	{ "FLAC",       {  -1,  -1,  -1,  -1,  -1,  -1 } },
	{ "Vorbis",     {  90,  45,  90,  40,  16,  90 } }, //default bitrate found empirically (sidenote: ffprobe couldn't read the bitrate from a .mkv, that's annoying); also Vorbis itself seems to support basically any birtate, but libvorbis errors outside that range
	{ "MP3",        { 128,  64, 256, 128,   0, INT16_MAX } }, //codec clamps bitrate when too high/low
	{ "ALAC",       {  -1,  -1,  -1,  -1,  -1,  -1 } },
};

const std::unordered_map<std::string, AudioCodecMiscInformation> AudioData::codecMiscInformation = {
	{ "copy (pcm)", { .recommendation=CodecRecommendedLevel::Okay,  .lossless=true,  .information_text="Uncompressed raw audio. Not very useful." } },
	{ "AAC",        { .recommendation=CodecRecommendedLevel::Good,  .lossless=false, .information_text="Very widespread. Opus is faster and higher quality though." } },
	{ "Opus",       { .recommendation=CodecRecommendedLevel::Best,  .lossless=false, .information_text="Currently the best lossy audio codec. Very unlikely to encounter compatibility issues." } },
	{ "FLAC",       { .recommendation=CodecRecommendedLevel::Best,  .lossless=true,  .information_text="It's lossless. Smaller filesizes than raw audio." } },
	{ "Vorbis",     { .recommendation=CodecRecommendedLevel::Okay,  .lossless=false, .information_text="Succeeded by Opus. Not much reason to use it." } }, //I was originally very against Vorbis because it had poor container support, but it's actually supported in every container exposed by this program; it's VP8 that's the problem
	{ "MP3",        { .recommendation=CodecRecommendedLevel::Awful, .lossless=false, .information_text="There are better options; at least use its successor AAC." } },
	{ "ALAC",       { .recommendation=CodecRecommendedLevel::Good,  .lossless=true,  .information_text="It's lossless. FLAC probably has better support." } },
};

CodecRecommendedLevel AudioData::get_audioEncoderRecommendation() const {
	return codecMiscInformation.at(get_audioEncoder()).recommendation;
}
bool AudioData::get_audioEncoderIsLossless() const {
	return codecMiscInformation.at(get_audioEncoder()).lossless;
}
std::string AudioData::get_audioEncoderInformationText() const {
	return codecMiscInformation.at(get_audioEncoder()).information_text;
}

const char** AudioData::get_audioPresetArray() const {
	return codecToPresetArray.at(get_audioEncoder()).presetArray.data();
}

int AudioData::get_audioPresetArray_size() const {
	const std::vector<const char*>& codecArr = codecToPresetArray.at(get_audioEncoder()).presetArray;
	return codecArr.size();
}

std::string AudioData::get_audioPreset() const {
	const std::vector<const char*>& codecArr = codecToPresetArray.at(get_audioEncoder()).presetArray;
	return codecArr.empty() ? "default" : codecArr[audioPresetArray_current];
}

void AudioData::getVoiceListFromExe_Balabolka(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList) {
	// Balabolka lists the voices under "SAPI" with a space, so it's a voice if it's indented
	// Yes, that's a dumb way of checking, but it works

	size_t pos;
	for (std::string& l : file_lines) {
		if (l.empty()) [[unlikely]] {
			continue;
		}
		if (l[0] == ' ') {
			// scrub whitespace
			pos = l.find_last_not_of(" \r\n");
			if (pos+1 < l.size()) {
				l.erase(pos+1);
			}
			pos = l.find_first_not_of(" ");
			if (pos != 0) {
				l.erase(0, pos);
			}

			// push to list
			if (!l.empty()) {
				voiceList.push_back(l);
			}
		}
	}
}

void AudioData::getVoiceListFromExe_Espeak(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList) {
	// The first line is formatting information, everything else is the languages
	// Columns are separated by spaces, simply go through until the 4th column
	// Assumes every VoiceName does not have spaces. The docs suggest it's not a requirement but every included voice seems to follow it.

	file_lines.erase(file_lines.begin()); //remove formatting information
	for (std::string& l : file_lines) {
		while (!l.empty() && l[0] == ' ') {
			l.erase(0, 1);
		}
		if (l.empty()) [[unlikely]] {
			//sanity check
			continue;
		}

		//find start and end of VoiceName column (TODO: would regex be easier? basically ([^\s]+[\s]+){3} to get the start of the name)
		size_t pos = l.find_first_of(" ");
		pos = l.find_first_not_of(" ", pos); //Language column
		pos = l.find_first_of(" ", pos);
		pos = l.find_first_not_of(" ", pos); //Age/Gender column
		pos = l.find_first_of(" ", pos);
		pos = l.find_first_not_of(" ", pos); //VoiceName column
		size_t endPos = l.find_first_of(" ", pos);

		voiceList.push_back(l.substr(pos, endPos - pos));
	}
}

void AudioData::update_voiceArray() {
	// Step 1: poll available voices
	//ARVT::system_helper((std::string(voiceEngineArray_exeForUpdatingVoiceList[voiceEngineArray_current]) + " > temp.txt").c_str(), true);
	system((std::string(voiceEngineArray_exeForUpdatingVoiceList[voiceEngineArray_current]) + " > temp.txt").c_str());

	if (!std::filesystem::exists("temp.txt")) {
		//TODO
		std::cout << "could not write or overwrite temp.txt" << std::endl;
	}

	std::vector<std::string> file_lines;
	std::string line;
	std::ifstream exe_output("temp.txt");

	while (std::getline(exe_output, line)) {
		file_lines.push_back(line);
	}
	exe_output.close();
	std::filesystem::remove("temp.txt"); //TODO: this can throw, also handle if it didn't exist

	// Step 2: read the available voices

	std::vector<std::string> voiceList;

	switch (voiceEngineArray_current) {
	#ifdef _WIN32
		case 0: //Balabolka
			getVoiceListFromExe_Balabolka(file_lines, voiceList);
			break;

		case 1: //eSpeak
			[[fallthrough]];
		case 2: //eSpeak NG
			getVoiceListFromExe_Espeak(file_lines, voiceList);
			break;

	#else
		//TODO
		case 0: //eSpeak
			[[fallthrough]];
		case 1: //eSpeak NG
			getVoiceListFromExe_Espeak(file_lines, voiceList);
			break;
	#endif
	}

	// Step 3: update internals

	if (voiceArray != nullptr) {
		for (int i = 0; i < voiceArray_length; i++) {
			delete voiceArray[i];
		}
		delete[] voiceArray;
	}

	if (voiceList.empty()) {
		//some error
		voiceArray = new char*[1];
		voiceArray[0] = new char[1];
		voiceArray[0][0] = '\0';
		voiceArray_current = -1;
		voiceArray_length = 1;
	} else {
		voiceArray = new char*[voiceList.size()];
		for (int i = 0; i < voiceList.size(); i++) {
			voiceArray[i] = new char[voiceList[i].size()+1];
			strcpy(voiceArray[i], voiceList[i].c_str());
		}
		voiceArray_current = -1;
		voiceArray_length = voiceList.size();
	}
}

void AudioData::update_audioBitrateValues() {
	const AudioData::BitrateData& data = codecToBitrate.at(get_audioEncoder());
	audio_bitrate_v = data.starting_value;
	audio_bitrate_min = data.min_value;
	audio_bitrate_max = data.max_value;
	//audio_bitrate_step = 4; //TODO: unused because ImGui sliders don't support stepping
}

void AudioData::update_audioPresetArray() {
	audioPresetArray_current = 0;

	const std::string codec = get_audioEncoder();
	audioCodec_hasPreset = !codecToPresetArray.at(codec).presetArray.empty();
	audioCodec_presetTerm = codecToPresetArray.at(codec).term;
}
