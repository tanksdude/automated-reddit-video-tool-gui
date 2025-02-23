#include "audio_data.h"
#include "helpers.h"
#include <cstring> //strcpy
#include <cstdlib> //system()
#include <fstream>
#include <filesystem>
#include <vector>
#include <iostream>

#if defined(WIN32) || defined(_WIN32)
	const char* AudioData::voiceEngineArray[4] = { "Balabolka", "Espeak", "Espeak NG", "Windows Narrator (TODO)" };
	const char* AudioData::voiceEngineArray_exeForUpdatingVoiceList[4] = { "\"..\\balcon\" -l", "\"espeak\" --voices=en", "\"espeak-ng\" --voices=en", "" }; //note: internal use only
	//TODO: look into whether nvaccess/nvda has compatible command line support
#else
	const char* AudioData::voiceEngineArray[5] = { "Espeak", "Espeak NG", "say (TODO)", "spd-say (TODO)", "Festival (TODO)" }; //TODO: https://askubuntu.com/questions/501910/how-to-text-to-speech-output-using-command-line/501917#501917
	const char* AudioData::voiceEngineArray_exeForUpdatingVoiceList[5] = { "../espeak --voices", "../espeak-ng --voices", "", "", "" }; //note: internal use only
#endif

const char* AudioData::audioEncoderArray[5] = { "copy", "AAC", "Opus", "FLAC", "Vorbis" };

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
	#if defined(WIN32) || defined(_WIN32)
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
