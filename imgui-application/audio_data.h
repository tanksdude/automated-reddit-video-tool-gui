#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "av_shared_info.h"

struct AudioData {
	struct BitrateData {
		// Bitrate in kbps
		int16_t starting_value;
		int16_t min_value; //"sane" min
		int16_t max_value; //"sane" max
		int16_t codec_default_value; //unused
		int16_t codec_min_value;     //unused
		int16_t codec_max_value;     //unused
	};

	/* Passing voice engine information to the Python script:
	 * The Python script will read the string character for character. It will
	 * translate each voice engine name to the correct program.
	 */
#ifdef _WIN32
	static const char* voiceEngineArray[4];
	static const char* voiceEngineArray_exeForUpdatingVoiceList[4]; //note: internal use only
	int voiceEngineArray_current = 0;
#else
	static const char* voiceEngineArray[5];
	static const char* voiceEngineArray_exeForUpdatingVoiceList[5]; //note: internal use only
	int voiceEngineArray_current = 1;
#endif

	char** voiceArray = nullptr;
	int voiceArray_current = -1;
	int voiceArray_length = 0;

	static void getVoiceListFromExe_Balabolka(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList);
	static void getVoiceListFromExe_Espeak(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList);

	/* Passing codec information to the Python script:
	 * The Python script will ignore everything after the first space. This
	 * means "copy (pcm)" will be interpreted the same as "copy".
	 */
	static const char* audioEncoderArray[7];
	static const char* audioEncoderArrayExtended[13];
	static inline const char** get_audioEncoderArray(bool extended) {
		return extended ? audioEncoderArrayExtended : audioEncoderArray;
	}
	static inline size_t get_audioEncoderArraySize(bool extended) {
		return extended ? sizeof(audioEncoderArrayExtended) / sizeof(*audioEncoderArrayExtended)
		                : sizeof(audioEncoderArray)         / sizeof(*audioEncoderArray);
	}
	/* Passing "preset" information to the Python script:
	 * The Python script will ignore everything after the first space. This
	 * means "5 (default)" will be interpreted the same as "5".
	 */
	static std::vector<const char*> audioPresetArray_AAC;
	static std::vector<const char*> audioPresetArray_Opus;
	static std::vector<const char*> audioPresetArray_FLAC;
	static std::vector<const char*> audioPresetArray_MP3;
	static std::vector<const char*> audioPresetArray_empty; //placeholder for the hashmap lookups
	//other possibilities: flac lpc_coeff_precision, libopus application, aac profile ("low complexity" default, "main", "scalable sampling rate")

	static const std::unordered_map<std::string, CodecPresetInformation> codecToPresetArray;
	static const std::unordered_map<std::string, BitrateData> codecToBitrate;
	static const std::unordered_map<std::string, AudioCodecMiscInformation> codecMiscInformation;

	int audioEncoderArray_current;
	CodecRecommendedLevel get_audioEncoderRecommendation() const;
	bool get_audioEncoderIsLossless() const;
	std::string get_audioEncoderInformationText() const;

	bool audioCodec_hasPreset;
	int audioPresetArray_current;
	std::string audioCodec_presetTerm;
	const char** get_audioPresetArray() const;
	int get_audioPresetArray_size() const;

	int16_t audio_bitrate_v;
	int16_t audio_bitrate_min;
	int16_t audio_bitrate_max;
	int16_t audio_bitrate_step = 4; //TODO: unused because ImGui sliders don't support stepping

	inline std::string get_voiceEngine() const { return std::string(voiceEngineArray[voiceEngineArray_current]); }
	inline std::string get_voiceName() const {
		if (voiceArray_current < 0) { return ""; }
		return std::string(voiceArray[voiceArray_current]);
	}
	inline std::string get_audioEncoder() const { return std::string(audioEncoderArrayExtended[audioEncoderArray_current]); } //TODO: hacky to not check whether to use the extra codecs
	inline std::string get_audioBitrate() const { return std::to_string(audio_bitrate_v) + "k"; }
	std::string get_audioPreset() const;

	void update_voiceArray(); // Call this when changing the speech engine!
	void update_audioEncoderValues(); // Call this when changing the audio encoder!
	void set_audioBitrate(int16_t val); // Used by the INI file, not really needed otherwise

	// Called during update_voiceArray() (don't call these yourself):
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
		update_audioEncoderValues();
	}
	~AudioData() {
		voiceArray_free();
	}
};
