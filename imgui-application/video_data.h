#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring> //strcpy, strcmp
#include <cstdint>
#include "av_shared_info.h"

struct VideoData {
	struct CrfData {
		int8_t starting_value;
		int8_t min_value; //"sane" min
		int8_t max_value; //"sane" max
		int8_t codec_default_value; //unused
		int8_t codec_min_value;     //unused
		int8_t codec_max_value;     //unused
	};

	/* Passing codec information to the Python script:
	 * The Python script will ignore everything after the first space. This
	 * means "H.265 / HEVC" will be interpreted the same as "H.265".
	 */
	static const char* videoEncoderArray[7];
	static const char* videoEncoderArrayExtended[13];
	static inline const char** get_videoEncoderArray(bool extended) {
		return extended ? videoEncoderArrayExtended : videoEncoderArray;
	}
	static inline size_t get_videoEncoderArraySize(bool extended) {
		return extended ? sizeof(videoEncoderArrayExtended) / sizeof(*videoEncoderArrayExtended)
		                : sizeof(videoEncoderArray)         / sizeof(*videoEncoderArray);
	}
	/* Passing preset information to the Python script:
	 * The Python script will ignore everything after the first space. This
	 * means "medium (default)" will be interpreted the same as "medium".
	 */
	static std::vector<const char*> videoPresetArray_H264_preset;
	static std::vector<const char*> videoPresetArray_VP9_deadline;
	static std::vector<const char*> videoPresetArray_AV1_usage;
	static std::vector<const char*> videoPresetArray_VP9_cpu_used;
	static std::vector<const char*> videoPresetArray_UtVideo_prediction;
	static std::vector<const char*> videoPresetArray_empty; //placeholder for the hashmap lookups
	static std::vector<const char*> videoPresetArray_CineForm_quality;
	static std::vector<const char*> videoPresetArray_VVC_preset;
	static std::vector<const char*> videoPresetArray_EVC_preset;

	static const std::unordered_map<std::string, CodecPresetInformation> codecToPresetArray1;
	static const std::unordered_map<std::string, CodecPresetInformation> codecToPresetArray2;
	static const std::unordered_map<std::string, CrfData> codecToCrf;
	static const std::unordered_map<std::string, VideoCodecMiscInformation> codecMiscInformation;

	static const char* videoContainerArray[6];

	bool use_speech_text = false;
	char video_replacement_numbers_input[64];
	bool audio_only_option_input = false;

	int videoEncoderArray_current;
	CodecRecommendedLevel get_videoEncoderRecommendation() const;
	bool get_videoEncoderIsLossless() const;
	bool get_videoEncoderSupportsAlpha() const;
	std::string get_videoEncoderInformationText() const;

	//preset counts: FFV1 zero, H.264 one, VP9 two
	bool videoCodec_hasPreset1;
	bool videoCodec_hasPreset2;
	int videoPresetArray1_current;
	int videoPresetArray2_current;
	std::string videoCodec_preset1Term;
	std::string videoCodec_preset2Term;
	const char** get_videoPresetArray1() const;
	const char** get_videoPresetArray2() const;
	int get_videoPresetArray1_size() const;
	int get_videoPresetArray2_size() const;
	//extending this to an arbitrary amount is a lot of effort

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

	int8_t crf_v;
	int8_t crf_min;
	int8_t crf_max;

	inline std::string get_video_replacement_numbers_input() const { return std::string(video_replacement_numbers_input); }
	inline std::string get_videoEncoder() const { return std::string(videoEncoderArrayExtended[videoEncoderArray_current]); } //TODO: hacky to not check whether to use the extra codecs
	inline std::string get_videoContainer() const { return std::string(videoContainerArray[videoContainerArray_current]); }
	inline std::string get_fps() const { return fractionalFps ? std::string(fps_numerator_input) + "/" + std::string(fps_denominator_input) : std::string(fpsArray[fpsArray_current]); }
	inline std::string get_videoCrf() const { return std::to_string(crf_v); }
	std::string get_videoPreset1() const;
	std::string get_videoPreset2() const;

	void update_videoCrfValues();
	void update_videoPresetArray();

	VideoData() {
		videoEncoderArray_current = 0;

		strcpy(video_replacement_numbers_input, "");
		strcpy(fps_numerator_input, "2997");
		strcpy(fps_denominator_input, "100");
		update_videoCrfValues();
		update_videoPresetArray();
	}
};
