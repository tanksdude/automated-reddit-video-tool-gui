#pragma once
#include <string>
#include <cstring> //strcpy

struct AudioData {
#if defined(WIN32) || defined(_WIN32)
	const char* voiceEngineArray[2] = { "Balabolka", "Espeak" };
	const char* voiceEngineArray_exeForUpdatingVoiceList[2] = { "\"..\\balcon\" -l", "\"..\\espeak\" --voices" }; //note: internal use only
#else
	const char* voiceEngineArray[1] = { "Espeak" };
	const char* voiceEngineArray_exeForUpdatingVoiceList[1] = { "../espeak --voices" }; //note: internal use only
#endif
	int voiceEngineArray_current = 0;

	char** voiceArray = nullptr;
	int voiceArray_current;
	int voiceArray_length;

	//TODO: maybe this goes in VideoData?
	const char* audioEncoderArray[5] = { "copy", "AAC", "Opus", "FLAC", "Vorbis" };
	int audioEncoderArray_current = 0;

	char audio_bitrate_input[32]; //used only if codec is not copy

	inline std::string get_voiceEngine() const { return std::string(voiceEngineArray[voiceEngineArray_current]); }
	inline std::string get_voice() const {
		if (voiceArray_current < 0) { return ""; }
		return std::string(voiceArray[voiceArray_current]);
	}
	inline std::string get_audioEncoder() const { return std::string(audioEncoderArray[audioEncoderArray_current]); }
	inline std::string get_audio_bitrate_input() const { return std::string(audio_bitrate_input); }

	void update_voiceArray(); // updates voiceArray and its related data

	AudioData() {
		strcpy(audio_bitrate_input, "256k");
		update_voiceArray();
	}

	~AudioData() {
		//TODO: clear voiceArray
	}
};
