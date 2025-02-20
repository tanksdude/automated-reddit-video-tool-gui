#include "video_data.h"

//note: video_data.cpp *needs* to be included before main.cpp to initialize this, otherwise update_videoCrfValues() in the constructor will be called when there's zero elements
//apart from H.264, the recommended starting value and range are mostly guesses
const std::unordered_map<std::string, VideoData::CrfData> VideoData::codecToCrf = {
	{ "H.264",        { 18, 17, 28, 23,  0, 51 } },
	{ "H.265 / HEVC", { 24, 23, 34, 28,  0, 51 } }, //+6 to H.264
	{ "VP8",          { 12,  5, 30, 32,  4, 63 } }, //0 is the true crf minimum but 4 is the default minimum
	{ "VP9",          { 31, 15, 35, 32,  0, 63 } },
	{ "AV1",          { 23, 20, 35, 32,  0, 63 } },
	{ "FFV1",         { -1, -1, -1, -1, -1, -1 } }, //TODO: seems like FFV1 is lossless-only
	//supports alpha: qtrle, vp8, vp9, ffv1
};

/*
//Kdenlive range:
const std::unordered_map<std::string, VideoData::CrfData> VideoData::codecToCrf = {
	{ "H.264",        { 23, 15, 45, ... } },
	{ "H.265 / HEVC", { 23, 15, 45, ... } },
	{ "VP8",          { 15,  5, 45, ... } },
	{ "VP9",          { 25, 15, 45, ... } },
	{ "AV1",          { 25, 15, 45, ... } },
};
*/

//note: quotes must be escaped
const std::unordered_map<std::string, std::vector<std::string>> VideoData::codecExtraArgs = {
	{ "H.264",        {} },
	{ "H.265 / HEVC", {} },
	{ "VP8",          { "-b:v", "1G" } },  //possibly required: https://goughlui.com/2023/12/27/video-codec-round-up-2023-part-4-libvpx-vp8/
	{ "VP9",          { "-b:v", "0" } },   //definitely required
	{ "AV1",          {} },                //requires FFmpeg 4.3+ to not require "-b:v 0", 4.4+ to avoid a lossless bug
	{ "FFV1",         { "-level", "3" } }, //version 3
};

/* Python argparse problems:
 * An argument like "-b:v" will confuse argparse because it expects that to be
 * used by the program instead of being an actual input. Adding a "--"
 * psuedo-argument before each use won't work, because that will stop the
 * nargs="+" from consuming. So as a workaround, prepend every argument with an
 * underscore and simply remove it in Python.
 * https://docs.python.org/3/library/argparse.html#arguments-containing
 * https://docs.python.org/3/howto/argparse.html#specifying-ambiguous-arguments
 */
std::string VideoData::get_codecExtraArgs() const {
	const std::vector<std::string>& argList = VideoData::codecExtraArgs.at(get_videoEncoder());
	if (argList.size() == 0) {
		return "";
	} else if (argList.size() == 1) {
		return "\"_" + argList[0] + "\"";
	} else {
		std::string args = "\"_" + argList[0] + "\"";
		for (int i = 1; i < argList.size(); i++) {
			args += " \"_" + argList[i] + "\"";
		}
		return args;
	}
}

void VideoData::update_videoCrfValues() {
	const VideoData::CrfData& data = codecToCrf.at(get_videoEncoder());
	crf_v = data.starting_value;
	crf_min = data.min_value;
	crf_max = data.max_value;
}

void VideoData::update_videoPresetArray() {
	//TODO: handle these later
}
