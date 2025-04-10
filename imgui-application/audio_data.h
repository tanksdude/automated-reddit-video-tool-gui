#pragma once
#include <string>
#include <vector>

struct AudioData {
	/*
	struct AudioCodecInformation {
		char name[16];
		bool lossless;
		std::vector<std::string> supportedContainers;
	};
	*/

	/* Passing voice engine information to the Python script:
	 * The Python script will read the string character for character. It will
	 * translate each voice engine name to the correct program.
	 */
#ifdef _WIN32
	static const char* voiceEngineArray[4];
	static const char* voiceEngineArray_exeForUpdatingVoiceList[4]; //note: internal use only
#else
	static const char* voiceEngineArray[5];
	static const char* voiceEngineArray_exeForUpdatingVoiceList[5]; //note: internal use only
#endif
	int voiceEngineArray_current = 0;

	char** voiceArray = nullptr;
	int voiceArray_current;
	int voiceArray_length;

	static void getVoiceListFromExe_Balabolka(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList);
	static void getVoiceListFromExe_Espeak(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList);

	/* Passing codec information to the Python script:
	 * The Python script will read the string character for character. It will
	 * translate each codec name to the correct library.
	 */
	static const char* audioEncoderArray[5];
	int audioEncoderArray_current = 0;

	std::uint16_t audio_bitrate_v = 192;
	std::uint16_t audio_bitrate_min = 60;
	std::uint16_t audio_bitrate_max = 300;
	std::uint16_t audio_bitrate_step = 4; //TODO: unused because ImGui sliders don't support stepping

	inline std::string get_voiceEngine() const { return std::string(voiceEngineArray[voiceEngineArray_current]); }
	inline std::string get_voice() const {
		if (voiceArray_current < 0) { return ""; }
		return std::string(voiceArray[voiceArray_current]);
	}
	inline std::string get_audioEncoder() const { return std::string(audioEncoderArray[audioEncoderArray_current]); }
	inline std::string get_audioBitrate() const { return std::to_string(audio_bitrate_v) + "k"; }

	void update_voiceArray(); // updates voiceArray and its related data

	AudioData() {
		update_voiceArray();
	}

	~AudioData() {
		if (voiceArray != nullptr) [[likely]] {
			for (int i = 0; i < voiceArray_length; i++) {
				delete voiceArray[i];
			}
			delete[] voiceArray;
		}
	}
};
