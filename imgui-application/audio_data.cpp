#include "audio_data.h"
#include "arvt_helpers.h"
#include <cstring> //memcpy
#include <fstream>
#include <filesystem>
#include <algorithm> //std::clamp, std::replace

#ifdef _WIN32
const std::array<const char*, 3> AudioData::speechEngineArray = { "Balabolka", "eSpeak", "eSpeak NG" };
const std::array<const char*, 3> AudioData::speechEngineArray_exeForUpdatingVoiceList = { "\"balcon\" -l", "\"espeak\" --voices=en", "\"espeak-ng\" --voices=en" }; //note: internal use only
#else
const std::array<const char*, 5> AudioData::speechEngineArray = { "eSpeak", "eSpeak NG", "say (TODO)", "spd-say (TODO)", "Festival (TODO)" }; //TODO: https://askubuntu.com/questions/501910/how-to-text-to-speech-output-using-command-line/501917#501917
const std::array<const char*, 5> AudioData::speechEngineArray_exeForUpdatingVoiceList = { "espeak --voices", "espeak-ng --voices", "", "", "" }; //note: internal use only
#endif

const std::array<const AudioCodecData*, 7>  AudioData::audioEncoderArray         = { &CODEC_AUDIO_copypcm, &CODEC_AUDIO_AAC, &CODEC_AUDIO_Opus, &CODEC_AUDIO_FLAC, &CODEC_AUDIO_Vorbis, &CODEC_AUDIO_MP3, &CODEC_AUDIO_ALAC };
const std::array<const AudioCodecData*, 13> AudioData::audioEncoderArrayExtended = { &CODEC_AUDIO_copypcm, &CODEC_AUDIO_AAC, &CODEC_AUDIO_Opus, &CODEC_AUDIO_FLAC, &CODEC_AUDIO_Vorbis, &CODEC_AUDIO_MP3, &CODEC_AUDIO_ALAC, &CODEC_AUDIO_AC3, &CODEC_AUDIO_EAC3, &CODEC_AUDIO_Speex, &CODEC_AUDIO_TTA, &CODEC_AUDIO_WMA2, &CODEC_AUDIO_MP2 };

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
	// IMPORTANT: SAPI voices will not appear in eSpeak's command line version: https://sourceforge.net/p/espeak/discussion/538921/thread/257f8ce6/
	// Also see function espeak_ListVoices() at https://github.com/espeak-ng/espeak-ng/blob/master/src/include/espeak-ng/speak_lib.h

	// The first line is formatting information, everything else is the languages
	// Columns are separated by spaces, simply go through until the 4th column
	// VoiceName does not have spaces, as they are replaced with underscores when printed: see function DisplayVoices() at https://github.com/espeak-ng/espeak-ng/blob/master/src/espeak-ng.c
	// When specifying a VoiceName to eSpeak, the underscores have to be replaced back by spaces, so do it here because this part is visible to the user
	// Theoretically there could be a voice that has underscores *and* spaces, but that let's say that's user error because that's above my pay grade to handle

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
		std::string voice = l.substr(pos, endPos - pos);

		//replace and push to list
		std::replace(voice.begin(), voice.end(), '_', ' ');
		voiceList.push_back(voice);
	}
}

void AudioData::update_voiceArray() {
	// Step 1: poll available voices
	int result = ARVT::system_helper((std::string(speechEngineArray_exeForUpdatingVoiceList[speechEngineArray_current]) + " > temp.txt").c_str(), false);

	//temp.txt will be created even if the executable doesn't exist (on most systems)
	//most systems return an error code if piping to a file fails
	if (result) {
		if (std::filesystem::exists("temp.txt")) [[likely]] {
			std::filesystem::remove("temp.txt");
		}
		voiceArray_free();
		voiceArray_setToBlank();
		return;
	}

	std::vector<std::string> file_lines;
	std::string line;
	std::ifstream exe_output("temp.txt");

	while (std::getline(exe_output, line)) {
		file_lines.push_back(line);
	}
	exe_output.close();
	std::filesystem::remove("temp.txt"); //TODO: this can throw, also technically it's possible for it to get deleted earlier but whatever

	// Step 2: read the available voices

	std::vector<std::string> voiceList;

	switch (speechEngineArray_current) {
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

	voiceArray_free();

	if (voiceList.empty()) {
		//some error
		voiceArray_setToBlank();
	} else {
		voiceArray = new char*[voiceList.size()];
		for (int i = 0; i < voiceList.size(); i++) {
			voiceArray[i] = new char[voiceList[i].size()+1];
			memcpy(voiceArray[i], voiceList[i].c_str(), voiceList[i].size()+1);
		}
		voiceArray_current = -1;
		voiceArray_length = voiceList.size();
	}
}

void AudioData::update_audioEncoderValues() {
	// Bitrate:
	const AudioCodecData::BitrateData& data = get_audioEncoder()->bitrateInfo;
	audio_bitrate_v = data.starting_value;
	audio_bitrate_min = data.min_value;
	audio_bitrate_max = data.max_value;
	//audio_bitrate_step = 4; //TODO: unused because ImGui sliders don't support stepping
}

void AudioData::set_audioBitrate(int16_t val) {
	const AudioCodecData::BitrateData& data = get_audioEncoder()->bitrateInfo;
	audio_bitrate_v = std::clamp(val, data.codec_min_value, data.codec_max_value);
}
