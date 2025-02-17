#include "video_data.h"

//note: video_data.cpp *needs* to be included before main.cpp to initialize this, otherwise update_videoCrfValues() in the constructor will be called when there's zero elements
const std::unordered_map<std::string, VideoData::CrfData> VideoData::codecToCrf = {
	{ "H.264",        { 18, 17, 28, 23, 0, 51 } }, //23 default, 18 "lossless", 17-28 "sane"
	{ "H.265 / HEVC", { 18, -1, -1, 28, 0, 51 } }, //28 default
	{ "VP8 (TODO)",   { -1, -1, -1, -1, 4, 63 } }, //10 default?
	{ "VP9 (TODO)",   { 31, 15, 35, -1, 0, 63 } }, //15-35 "sane", 31 "recommended"; requires "-b:v 0" to trigger crf mode; also there's lossless mode, check it out; also there's something like presets
	{ "AV1 (TODO)",   { 23, -1, -1, 35, 0, 63 } }, //35 default, 23 "lossless"; requires FFmpeg 4.3+ to not require "-b:v 0", 4.4+ to avoid a lossless bug
};

void VideoData::update_videoCrfValues() {
	const VideoData::CrfData& data = codecToCrf.at(get_videoEncoder());
	crf_v = data.starting_value;
	crf_min = data.min_value;
	crf_max = data.max_value;
}
