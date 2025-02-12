#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring> //strcpy, strcmp

struct VideoData {
	/*
	static const std::unordered_map<std::string, std::pair<int, int>> codecToCrf = {
		{ "H.264", { 0, 51 } }, //23 default, 18 "lossless", 17-28 "sane"
		{ "H.265 / HEVC", { 0, 51 } }, //28 default
		{ "VP8", { 4, 63 } }, //10 default?
		{ "VP9", { 0, 63 } }, //15-35 "sane", 31 "recommended"; requires "-b:v 0" to trigger crf mode; also there's lossless mode, check it out; also there's something like presets
		{ "AV1", { 0, 63 } }  //35 default, 23 "lossless"; requires FFmpeg 4.3+ to not require "-b:v 0", 4.4+ to avoid a lossless bug
	};
	*/

	bool use_speech_text = false;
	char video_replacement_numbers_input[64];
	bool audio_only_option_input = false;

	/* Passing codec information to the Python script:
	 * The Python script will read the string character for character. It will
	 * interpret "H.265 / HEVC" as H.265.
	 */
	const char* videoEncoderArray[5] = { "H.264", "H.265 / HEVC", "VP8 (TODO)", "VP9 (TODO)", "AV1 (TODO)" }; //TODO: fill based on the hashmap above?
	int videoEncoderArray_current = 0;

	/* Passing preset information to the Python script:
	 * The Python script will ignore everything after the first space. This
	 * means "medium (default)" will be interpreted the same as "medium".
	 */
	//TODO: either separate preset array or dynamically change this based on the codec
	const char* videoPresetArray[11] = { "default", "ultrafast", "superfast", "veryfast", "faster", "fast", "medium (default)" , "slow", "slower", "veryslow", "placebo (not recommended)" };
	int videoPresetArray_current = 0;

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

	std::int8_t crf_v = 18;
	std::int8_t crf_min = 0;
	std::int8_t crf_max = 63;

	inline std::string get_video_replacement_numbers_input() const { return std::string(video_replacement_numbers_input); }
	inline std::string get_videoEncoder() const { return std::string(videoEncoderArray[videoEncoderArray_current]); }
	inline std::string get_videoPreset() const { return std::string(videoPresetArray[videoPresetArray_current]); }
	inline std::string get_videoContainer() const { return std::string(videoContainerArray[videoContainerArray_current]); }

	VideoData() {
		strcpy(video_replacement_numbers_input, "");
	}
};
