#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <cstring> //strcpy
#include "av_codecs.h"

struct AudioData {

	/*   SPEECH   */

#ifdef _WIN32
	static const std::array<const char*, 4> speechEngineArray;
	int speechEngineArray_current = 2;
#else
	static const std::array<const char*, 2> speechEngineArray;
	int speechEngineArray_current = 1;
#endif
	char speech_language_input[32];
	std::string getExeForUpdatingVoiceList();

	char** voiceArray = nullptr;
	int voiceArray_current = -1;
	int voiceArray_length = 0;
	int update_voiceArray(); // Call this when changing the speech engine! Returns 0 on success

	inline std::string get_speechEngine() const { return std::string(speechEngineArray[speechEngineArray_current]); }
	inline std::string get_voiceName() const {
		if (voiceArray_current < 0) { return ""; }
		return std::string(voiceArray[voiceArray_current]);
	}

	static void getVoiceListFromExe_Balabolka(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList);
	static void getVoiceListFromExe_Espeak(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList);
	static void getVoiceListFromExe_Wsay(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList);

	// [Internal] Called during update_voiceArray():
	inline void voiceArray_free() {
		if (voiceArray != nullptr) [[likely]] {
			for (int i = 0; i < voiceArray_length; i++) {
				delete voiceArray[i];
			}
			delete[] voiceArray;
		}
	}
	inline void voiceArray_setToBlank() {
		voiceArray = nullptr;
		voiceArray_current = -1;
		voiceArray_length = 0;
	}

	/*   AUDIO   */

	static const std::array<const AudioCodecData*, 13> audioEncoderArrayExtended;
	static inline size_t get_audioEncoderArraySize(bool extended) {
		return extended ? audioEncoderArrayExtended.size() : 7;
	}

	int audioEncoder_idx;
	int audioEncoder_preset1_idx;
	void set_encoder_idx(int idx) {
		audioEncoder_idx = idx;
		audioEncoder_preset1_idx = 0;
		update_audioEncoderValues();
	}
	void set_encoder_preset1_idx(int idx) {
		audioEncoder_preset1_idx = idx;
	}

	inline const AudioCodecData* get_audioEncoder() const { return audioEncoderArrayExtended[audioEncoder_idx]; }
	inline std::string get_audioPreset1_currentValue() const {
		const GenericCodecPreset& ac_p1 = get_audioEncoder()->preset1;
		return ac_p1.displayValues.empty() ? "default" : ac_p1.internalValues[audioEncoder_preset1_idx];
	}

	uint16_t audio_bitrate_v;
	uint16_t audio_bitrate_min;
	uint16_t audio_bitrate_max;
	uint16_t audio_bitrate_step = 4; //TODO: unused because ImGui sliders don't support stepping

	inline std::string get_audioBitrate() const { return std::to_string(audio_bitrate_v) + "k"; }
	void set_audioBitrate(uint16_t val); // Used by the INI file, not really needed otherwise
	void update_audioEncoderValues(); // [Internal] Called when changing the audio encoder

	// Make sure to call update_voiceArray() to finish initialization! (Assuming you want a voice list populated.)
	AudioData() {
		audioEncoder_idx = 1;
		audioEncoder_preset1_idx = 0;
		update_audioEncoderValues();
		strcpy(speech_language_input, "en");
	}
	~AudioData() {
		voiceArray_free();
	}
};
