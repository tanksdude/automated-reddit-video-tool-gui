#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring> //strcpy, strcmp
#include <array>
#include "av_codecs.h"

struct VideoData {
	static std::array<const VideoCodecData*, 7>  videoEncoderArray;
	static std::array<const VideoCodecData*, 13> videoEncoderArrayExtended;
	static inline size_t get_videoEncoderArraySize(bool extended) {
		return extended ? videoEncoderArrayExtended.size() : videoEncoderArray.size();
	}

	int videoEncoderArray_current;
	int videoEncoder_preset1_current;
	int videoEncoder_preset2_current;
	void set_encoder_idx(int idx) {
		videoEncoderArray_current = idx;
		videoEncoder_preset1_current = 0;
		videoEncoder_preset2_current = 0;
		update_videoEncoderValues();
	}
	void set_encoder_preset1_idx(int idx) {
		videoEncoder_preset1_current = idx;
		//TODO: in the future this would be used when the codec's second preset changes its range depending on its first preset (like VP9)
	}
	void set_encoder_preset2_idx(int idx) {
		videoEncoder_preset2_current = idx;
	}

	inline const VideoCodecData* get_videoEncoder() const { return videoEncoderArrayExtended[videoEncoderArray_current]; }
	inline std::string get_videoPreset1_currentValue() const {
		const GenericCodecPreset& vc_p1 = get_videoEncoder()->preset1;
		return vc_p1.displayValues.empty() ? "default" : vc_p1.internalValues[videoEncoder_preset1_current];
	}
	inline std::string get_videoPreset2_currentValue() const {
		const GenericCodecPreset& vc_p2 = get_videoEncoder()->preset2;
		return vc_p2.displayValues.empty() ? "default" : vc_p2.internalValues[videoEncoder_preset2_current];
	}

	bool use_speech_text = false;
	char video_replacement_numbers_input[64];
	bool audio_only_option_input = false;

	static const char* videoContainerArray[6];
	int videoContainerArray_current = 0;
	bool faststart_flag = false;
	inline bool get_faststart_available() const {
		return strcmp(videoContainerArray[videoContainerArray_current], ".mp4") == 0 ||
		       strcmp(videoContainerArray[videoContainerArray_current], ".mov") == 0 ||
		       strcmp(videoContainerArray[videoContainerArray_current], ".m4v") == 0; //.m4a also supports the flag
	}

	bool fractionalFps = false;
	//integers
	static const char* fpsArray[9];
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

	void update_videoEncoderValues(); // [Internal] Called when changing the video encoder
	void set_videoCrf(int8_t val); // Used by the INI file, not really needed otherwise

	VideoData() {
		videoEncoderArray_current = 0;
		videoEncoder_preset1_current = 0;
		videoEncoder_preset2_current = 0;
		update_videoEncoderValues();

		strcpy(video_replacement_numbers_input, "");
		strcpy(fps_numerator_input, "2997");
		strcpy(fps_denominator_input, "100");
	}
};
