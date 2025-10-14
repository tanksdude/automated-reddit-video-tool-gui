#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include "av_codecs.h"

struct AudioData {
#ifdef _WIN32
	static const std::array<const char*, 3> speechEngineArray;
	static const std::array<const char*, 3> speechEngineArray_exeForUpdatingVoiceList; //note: internal use only
	int speechEngineArray_current = 0;
#else
	static const std::array<const char*, 5> speechEngineArray;
	static const std::array<const char*, 5> speechEngineArray_exeForUpdatingVoiceList; //note: internal use only
	int speechEngineArray_current = 1;
#endif

	char** voiceArray = nullptr;
	int voiceArray_current = -1;
	int voiceArray_length = 0;
	void update_voiceArray(); // Call this when changing the speech engine!

	inline std::string get_speechEngine() const { return std::string(speechEngineArray[speechEngineArray_current]); }
	inline std::string get_voiceName() const {
		if (voiceArray_current < 0) { return ""; }
		return std::string(voiceArray[voiceArray_current]);
	}

	static void getVoiceListFromExe_Balabolka(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList);
	static void getVoiceListFromExe_Espeak(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList);

	static const std::array<const AudioCodecData*, 7>  audioEncoderArray;
	static const std::array<const AudioCodecData*, 13> audioEncoderArrayExtended;
	static inline size_t get_audioEncoderArraySize(bool extended) {
		return extended ? audioEncoderArrayExtended.size() : audioEncoderArray.size();
	}

	int audioEncoderArray_current;
	int audioEncoder_preset1_current;
	void set_encoder_idx(int idx) {
		audioEncoderArray_current = idx;
		audioEncoder_preset1_current = 0;
		update_audioEncoderValues();
	}
	void set_encoder_preset1_idx(int idx) {
		audioEncoder_preset1_current = idx;
	}

	inline const AudioCodecData* get_audioEncoder() const { return audioEncoderArrayExtended[audioEncoderArray_current]; }
	inline std::string get_audioPreset1_currentValue() const {
		const GenericCodecPreset& ac_p1 = get_audioEncoder()->preset1;
		return ac_p1.displayValues.empty() ? "default" : ac_p1.internalValues[audioEncoder_preset1_current];
	}

	int16_t audio_bitrate_v;
	int16_t audio_bitrate_min;
	int16_t audio_bitrate_max;
	int16_t audio_bitrate_step = 4; //TODO: unused because ImGui sliders don't support stepping

	inline std::string get_audioBitrate() const { return std::to_string(audio_bitrate_v) + "k"; }
	void set_audioBitrate(int16_t val); // Used by the INI file, not really needed otherwise
	void update_audioEncoderValues(); // [Internal] Called when changing the audio encoder

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

	// Make sure to call update_voiceArray() to finish initialization! (Assuming you want a voice list populated.)
	AudioData() {
		audioEncoderArray_current = 1;
		audioEncoder_preset1_current = 0;
		update_audioEncoderValues();
	}
	~AudioData() {
		voiceArray_free();
	}
};
