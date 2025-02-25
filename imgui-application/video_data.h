#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <utility> //std::pair
#include <cstring> //strcpy, strcmp

struct VideoData {
	struct CrfData {
		std::int8_t starting_value;
		std::int8_t min_value; //"sane" min
		std::int8_t max_value; //"sane" max
		std::int8_t codec_default_value; //unused
		std::int8_t codec_min_value;     //unused
		std::int8_t codec_max_value;     //unused
		CrfData(std::int8_t start, std::int8_t min, std::int8_t max, std::int8_t c_def, std::int8_t c_min, std::int8_t c_max) :
			starting_value(start), min_value(min), max_value(max), codec_default_value(c_def), codec_min_value(c_min), codec_max_value(c_max) {}
		//CrfData() {} //TODO?
	};

	/*
	struct VideoCodecInformation {
		char name[16];
		bool lossless;
		bool supportsAlpha;
		std::vector<std::string> supportedContainers;
	};
	*/

	/* Passing codec information to the Python script:
	 * The Python script will ignore everything after the first space. This
	 * means "H.265 / HEVC" will be interpreted the same as "H.265".
	 */
	static const char* videoEncoderArray[6];
	/* Passing preset information to the Python script:
	 * The Python script will ignore everything after the first space. This
	 * means "medium (default)" will be interpreted the same as "medium".
	 */
	static std::vector<const char*> videoPresetArray_H264;
	static std::vector<const char*> videoPresetArray_VP9_deadline;
	static std::vector<const char*> videoPresetArray_VP9_cpu_used;
	static std::vector<const char*> videoPresetArray_empty; //placeholder for the hashmap lookups

	static const std::unordered_map<std::string, std::pair<std::string, std::vector<const char*>&>> codecToPresetArray1; //preset term ("preset"/"deadline"), then list of options
	static const std::unordered_map<std::string, std::pair<std::string, std::vector<const char*>&>> codecToPresetArray2;
	static const std::unordered_map<std::string, CrfData> codecToCrf;

	static const char* videoContainerArray[6];

	bool use_speech_text = false;
	char video_replacement_numbers_input[64];
	bool audio_only_option_input = false;

	int videoEncoderArray_current = 0;
	bool get_videoEncoderIsLossless() { return false; } //TODO: would allow FFV1 to hide the CRF slider; update this when updating the preset information //also every codec here has a lossless mode but only FFV1's will be supported
	bool get_videoEncoderSupportsAlpha() { return false; } //TODO: display "doesn't support alpha" if some color is "transparent", though yes that wouldn't detect "rgba(...)" colors; also check which codecs need some kind of specific flag to enable alpha encoding, add checkbox for that

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

	std::int8_t crf_v;
	std::int8_t crf_min;
	std::int8_t crf_max;

	inline std::string get_video_replacement_numbers_input() const { return std::string(video_replacement_numbers_input); }
	inline std::string get_videoEncoder() const { return std::string(videoEncoderArray[videoEncoderArray_current]); }
	inline std::string get_videoContainer() const { return std::string(videoContainerArray[videoContainerArray_current]); }
	inline std::string get_fps() const { return fractionalFps ? std::string(fps_numerator_input) + "/" + std::string(fps_denominator_input) : std::string(fpsArray[fpsArray_current]); }
	std::string get_videoPreset1() const;
	std::string get_videoPreset2() const;

	void update_videoCrfValues();
	void update_videoPresetArray();

	VideoData() {
		strcpy(video_replacement_numbers_input, "");
		strcpy(fps_numerator_input, "2997");
		strcpy(fps_denominator_input, "100");
		update_videoCrfValues();
		update_videoPresetArray();
	}
};
