#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring> //strcpy, strcmp
#include <array>
#include "av_codecs.h"

struct VideoData {
	static const std::array<const VideoCodecData*, 13> videoEncoderArrayExtended;
	static inline size_t get_videoEncoderArraySize(bool extended) {
		return extended ? videoEncoderArrayExtended.size() : 7;
	}

	int videoEncoder_idx;
	int videoEncoder_preset1_idx;
	int videoEncoder_preset2_idx;
	void set_encoder_idx(int idx) {
		videoEncoder_idx = idx;
		videoEncoder_preset1_idx = 0;
		videoEncoder_preset2_idx = 0;
		update_videoEncoderValues();
	}
	void set_encoder_preset1_idx(int idx) {
		videoEncoder_preset1_idx = idx;
		//TODO: in the future this would be used when the codec's second preset changes its range depending on its first preset (like VP9)
	}
	void set_encoder_preset2_idx(int idx) {
		videoEncoder_preset2_idx = idx;
	}

	inline const VideoCodecData* get_videoEncoder() const { return videoEncoderArrayExtended[videoEncoder_idx]; }
	inline std::string get_videoPreset1_currentValue() const {
		const GenericCodecPreset& vc_p1 = get_videoEncoder()->preset1;
		return vc_p1.displayValues.empty() ? "default" : vc_p1.internalValues[videoEncoder_preset1_idx];
	}
	inline std::string get_videoPreset2_currentValue() const {
		const GenericCodecPreset& vc_p2 = get_videoEncoder()->preset2;
		return vc_p2.displayValues.empty() ? "default" : vc_p2.internalValues[videoEncoder_preset2_idx];
	}

	bool use_speech_text = false;
	char video_replacement_numbers_input[64];
	bool audio_only_option_input = false;

	static const std::array<const char*, 6> videoContainerArray;
	int videoContainerArray_current = 0;
	bool faststart_flag = false;
	inline bool get_faststart_available() const {
		return strcmp(videoContainerArray[videoContainerArray_current], ".mp4") == 0 ||
		       strcmp(videoContainerArray[videoContainerArray_current], ".mov") == 0 ||
		       strcmp(videoContainerArray[videoContainerArray_current], ".m4v") == 0; //.m4a also supports the flag
	}

	bool fractionalFps = false;
	//integers
	static const std::array<const char*, 10> fpsArray;
	int fpsArray_current = 5;
	//fraction
	char fps_numerator_input[32];
	char fps_denominator_input[32];

	int8_t video_crf_v;
	int8_t video_crf_min;
	int8_t video_crf_max;

	inline std::string get_video_replacement_numbers_input() const { return std::string(video_replacement_numbers_input); }
	inline std::string get_videoContainer() const { return std::string(videoContainerArray[videoContainerArray_current]); }
	inline std::string get_fps() const { return fractionalFps ? std::string(fps_numerator_input) + "/" + std::string(fps_denominator_input) : std::string(fpsArray[fpsArray_current]); }
	inline std::string get_videoCrf() const { return std::to_string(video_crf_v); }

	void set_videoCrf(int8_t val); // Used by the INI file, not really needed otherwise
	void update_videoEncoderValues(); // [Internal] Called when changing the video encoder

	VideoData() {
		videoEncoder_idx = 0;
		videoEncoder_preset1_idx = 0;
		videoEncoder_preset2_idx = 0;
		update_videoEncoderValues();

		strcpy(video_replacement_numbers_input, "");
		strcpy(fps_numerator_input, "2997");
		strcpy(fps_denominator_input, "100");
	}
};
