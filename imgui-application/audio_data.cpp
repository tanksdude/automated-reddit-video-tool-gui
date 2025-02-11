#include "audio_data.h"
#include "helpers.h"
#include <cstring> //strcpy
#include <cstdlib> //system()
#include <fstream>
#include <filesystem>
#include <vector>
#include <iostream>

void AudioData::update_voiceArray() {
	//TODO: first poll if exe exists

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
		case 0: //Balabolka
			{
			// Balabolka lists the voices under "SAPI" with a space, so it's a voice if it's indented
			// yes, that's a dumb way of checking, but it works

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
			break;

		case 1: //eSpeak
			[[fallthrough]];
		case 2: //eSpeak NG
			{
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
				do { pos++; } while (l[pos] == ' '); //Language column
				pos = l.find_first_of(" ", pos);
				do { pos++; } while (l[pos] == ' '); //Age/Gender column
				pos = l.find_first_of(" ", pos);
				do { pos++; } while (l[pos] == ' '); //VoiceName column
				size_t endPos = l.find_first_of(" ", pos);

				voiceList.push_back(l.substr(pos, endPos - pos));
			}
			}
			break;
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
