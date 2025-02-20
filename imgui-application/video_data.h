#pragma once
#include <string>
#include <vector>
#include <unordered_map>
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
	static const std::unordered_map<std::string, CrfData> codecToCrf;

	bool use_speech_text = false;
	char video_replacement_numbers_input[64];
	bool audio_only_option_input = false;

	/* Passing codec information to the Python script:
	 * The Python script will ignore everything after the first space. This
	 * means "H.265 / HEVC" will be interpreted the same as "H.265".
	 */
	//TODO: make this static
	const char* videoEncoderArray[6] = { "H.264", "H.265 / HEVC", "VP8", "VP9", "AV1", "FFV1" }; //TODO: fill based on the hashmap above?
	int videoEncoderArray_current = 0;
	bool videoEncoderIsLossless = false; //TODO: would allow FFV1 to hide the CRF slider; update this when updating the preset information //also every codec here has a lossless mode but only FFV1's will be supported
	bool videoEncoderSupportsAlpha = false; //TODO: display "doesn't support alpha" if some color is "transparent", though yes that wouldn't detect "rgba(...)" colors; also check which codecs need some kind of specific flag to enable alpha encoding, add checkbox for that
	//TODO: do those bools need to be mutable? pretty sure they're going to get updated from update_videoPresetArray(), so no

	//TODO: VP8 and FFV1 don't support .mp4; either ignore that (print error to some console), disable something, or drop support

	/* Passing preset information to the Python script:
	 * The Python script will ignore everything after the first space. This
	 * means "medium (default)" will be interpreted the same as "medium".
	 */
	//TODO: either separate preset array or dynamically change this based on the codec
	const char* videoPresetArray[11] = { "default", "ultrafast", "superfast", "veryfast", "faster", "fast", "medium (default)" , "slow", "slower", "veryslow", "placebo (not recommended)" };
	int videoPresetArray_current = 0;

	const char* videoPresetArray_VP9_deadline[4] = { "default", "good (default)", "best", "realtime" };
	const char* videoPresetArray_VP9_cpu_used[10] = { "default", "0", "1 (default)", "2", "3", "4", "5", "6", "7", "8" };
	const char* videoPresetArray_AV1[10] = { "default", "0", "1 (default)", "2", "3", "4", "5", "6", "7", "8" }; //valid: 0-8, 1 default; with "-usage realtime" the range is 7-10

	const char* videoContainerArray[3] = { ".mp4", ".mkv", ".mov" };
	int videoContainerArray_current = 0;
	bool faststart_flag = false;
	inline bool get_faststart_available() const {
		return strcmp(videoContainerArray[videoContainerArray_current], ".mp4") == 0 ||
		       strcmp(videoContainerArray[videoContainerArray_current], ".mov") == 0;
	}

	//TODO
	std::uint16_t fps_v = 60;
	std::uint16_t fps_min = 1;
	std::uint16_t fps_max = 120;

	std::int8_t crf_v;
	std::int8_t crf_min;
	std::int8_t crf_max;

	inline std::string get_video_replacement_numbers_input() const { return std::string(video_replacement_numbers_input); }
	inline std::string get_videoEncoder() const { return std::string(videoEncoderArray[videoEncoderArray_current]); }
	inline std::string get_videoPreset() const { return std::string(videoPresetArray[videoPresetArray_current]); }
	inline std::string get_videoContainer() const { return std::string(videoContainerArray[videoContainerArray_current]); }

	void update_videoCrfValues();
	void update_videoPresetArray(); //TODO

	VideoData() {
		strcpy(video_replacement_numbers_input, "");
		update_videoCrfValues();
	}
};
