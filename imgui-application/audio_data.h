#pragma once
#include <string>
#include <cstring> //strcpy

struct AudioData {
	/* Passing voice engine information to the Python script:
	 * The Python script will read the string character for character. It will
	 * translate each voice engine name to the correct program.
	 */
#if defined(WIN32) || defined(_WIN32)
	const char* voiceEngineArray[4] = { "Balabolka", "Espeak", "Espeak NG", "Windows Narrator (TODO)" };
	const char* voiceEngineArray_exeForUpdatingVoiceList[4] = { "\"..\\balcon\" -l", "\"espeak\" --voices=en", "\"espeak-ng\" --voices=en", "" }; //note: internal use only
	//TODO: look into whether nvaccess/nvda has compatible command line support
#else
	const char* voiceEngineArray[4] = { "Espeak", "say (TODO)", "spd-say (TODO)", "Festival (TODO)" }; //TODO: https://askubuntu.com/questions/501910/how-to-text-to-speech-output-using-command-line/501917#501917
	const char* voiceEngineArray_exeForUpdatingVoiceList[4] = { "../espeak --voices", "", "", "" }; //note: internal use only
#endif
	int voiceEngineArray_current = 0;

	char** voiceArray = nullptr;
	int voiceArray_current;
	int voiceArray_length;

	/* Passing codec information to the Python script:
	 * The Python script will read the string character for character. It will
	 * translate each codec name to the correct library.
	 */
	//TODO: maybe this goes in VideoData?
	const char* audioEncoderArray[5] = { "copy", "AAC", "Opus", "FLAC", "Vorbis" };
	int audioEncoderArray_current = 0;

	//TODO: this should probably be a slider input, [60,300] step=4
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
