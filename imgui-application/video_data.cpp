#include "video_data.h"

const char* VideoData::videoEncoderArray[7] = { "H.264", "H.265 / HEVC", "VP8", "VP9", "AV1", "FFV1", "Ut Video" };
std::vector<const char*> VideoData::videoPresetArray_H264_preset = { "default", "ultrafast", "superfast", "veryfast", "faster", "fast", "medium (default)" , "slow", "slower", "veryslow", "placebo (not recommended)" };
std::vector<const char*> VideoData::videoPresetArray_VP9_deadline = { "default", "best", "good (default)", "realtime" };
std::vector<const char*> VideoData::videoPresetArray_AV1_usage    = { "default", "good (default)", "realtime", "allintra" };
std::vector<const char*> VideoData::videoPresetArray_VP9_cpu_used = { "default", "0", "1 (default)", "2", "3", "4", "5", "6", "7", "8" }; //TODO: the range changes based on the deadline
std::vector<const char*> VideoData::videoPresetArray_UtVideo_prediction = { "default", "none", "left (default)", "gradient (NOT SUPPORTED)", "median" }; //https://ffmpeg.org/doxygen/3.0/libutvideoenc_8cpp.html
std::vector<const char*> VideoData::videoPresetArray_empty = {};
const char* VideoData::videoContainerArray[6] = { ".mp4", ".mkv", ".mov", ".webm", ".ogg", ".avi" }; //TODO: .ogg has really poor support for codecs, so either remove it or find a way to communicate that or disable/warn on certain codecs; could remove codecs based on the container or vice versa
const char* VideoData::fpsArray[9] = { "10", "20", "25", "30", "50", "60", "75", "90", "120" };

const std::unordered_map<std::string, VideoData::CodecPresetInformation> VideoData::codecToPresetArray1 = {
	{ "H.264",        { "Preset", videoPresetArray_H264_preset } },
	{ "H.265 / HEVC", { "Preset", videoPresetArray_H264_preset } },
	{ "VP8",          { "Deadline", videoPresetArray_VP9_deadline } },
	{ "VP9",          { "Deadline", videoPresetArray_VP9_deadline } },
	{ "AV1",          { "Usage", videoPresetArray_AV1_usage } },
	{ "FFV1",         { "", videoPresetArray_empty } },
	{ "Ut Video",     { "Prediction", videoPresetArray_UtVideo_prediction } },
};
const std::unordered_map<std::string, VideoData::CodecPresetInformation> VideoData::codecToPresetArray2 = {
	{ "H.264",        { "", videoPresetArray_empty } },
	{ "H.265 / HEVC", { "", videoPresetArray_empty } },
	{ "VP8",          { "-cpu-used", videoPresetArray_VP9_cpu_used } },
	{ "VP9",          { "-cpu-used", videoPresetArray_VP9_cpu_used } }, //0-5 for best & good, 0-8 for realtime
	{ "AV1",          { "-cpu-used", videoPresetArray_VP9_cpu_used } }, //lacking information on the valid range
	{ "FFV1",         { "", videoPresetArray_empty } },
	{ "Ut Video",     { "", videoPresetArray_empty } }, //TODO: -flags +ilme (don't wanna do a checkbox, so maybe { "il", "im", "ilm", "ile", "ime", "ilme" })
};

//note: video_data.cpp *needs* to be included before main.cpp to initialize this, otherwise update_videoCrfValues() in the constructor will be called when there's zero elements
//apart from H.264, the recommended starting value and range are mostly guesses
const std::unordered_map<std::string, VideoData::CrfData> VideoData::codecToCrf = {
	{ "H.264",        { 18, 17, 28, 23,  0, 51 } },
	{ "H.265 / HEVC", { 24, 23, 34, 28,  0, 51 } }, //+6 to H.264
	{ "VP8",          { 12,  5, 30, 32,  4, 63 } }, //0 is the true crf minimum but 4 is the default minimum
	{ "VP9",          { 31, 15, 35, 32,  0, 63 } },
	{ "AV1",          { 23, 20, 35, 32,  0, 63 } },
	{ "FFV1",         { -1, -1, -1, -1, -1, -1 } },
	{ "Ut Video",     { -1, -1, -1, -1, -1, -1 } },
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

const std::unordered_map<std::string, VideoData::VideoCodecMiscInformation> VideoData::codecMiscInformation = {
	{ "H.264",        { .recommendation=VideoData::VideoCodecMiscInformation::RecommendedLevel::Best,  .lossless=false, .supportsAlpha=false, .information_text="The most widespread video codec. Fast encoding times." } },
	{ "H.265 / HEVC", { .recommendation=VideoData::VideoCodecMiscInformation::RecommendedLevel::Okay,  .lossless=false, .supportsAlpha=false, .information_text="H.264\'s successor. Achieves smaller filesizes, though encoding takes longer. Also compression artifacts may be more noticeable." } },
	{ "VP8",          { .recommendation=VideoData::VideoCodecMiscInformation::RecommendedLevel::Awful, .lossless=false, .supportsAlpha=true,  .information_text="Succeeded by VP9, and has very poor container support." } },
	{ "VP9",          { .recommendation=VideoData::VideoCodecMiscInformation::RecommendedLevel::Good,  .lossless=false, .supportsAlpha=true,  .information_text="About the same quality as HEVC with faster encode times." } },
	{ "AV1",          { .recommendation=VideoData::VideoCodecMiscInformation::RecommendedLevel::Okay,  .lossless=false, .supportsAlpha=false /* the standard seems to support it, but libaom-av1 does not (yet) */, .information_text="VP9\'s successor and fully open-source. Even better quality, however it is extremely slow (like 10x-60x H.264)." } },
	{ "FFV1",         { .recommendation=VideoData::VideoCodecMiscInformation::RecommendedLevel::Best,  .lossless=true,  .supportsAlpha=true,  .information_text="It's lossless. Might not be supported by your video editor, in which case use Ut Video if you want lossless video." } }, //NBSP for Ut Video
	{ "Ut Video",     { .recommendation=VideoData::VideoCodecMiscInformation::RecommendedLevel::Best,  .lossless=true,  .supportsAlpha=true,  .information_text="It's lossless. Faster to encode than FFV1 (though not really for this application) at the cost of larger filesizes. It is the codec used by OBS\'s lossless capture" } },
	//other FFmpeg encoders that support alpha (without strange workarounds): QuickTime Animation (qtrle), Apple ProRes 4444 (prores), Cineform (cfhd)
};

std::string VideoData::VideoCodecMiscInformation::get_recommendedStr() const {
	switch (recommendation) {
		default: [[fallthrough]];
		case VideoData::VideoCodecMiscInformation::RecommendedLevel::No_Opinion:
			return "?";
		case VideoData::VideoCodecMiscInformation::RecommendedLevel::Awful:
			return "☆☆☆"; //"□□□";
		case VideoData::VideoCodecMiscInformation::RecommendedLevel::Okay:
			return "★☆☆"; //"■□□";
		case VideoData::VideoCodecMiscInformation::RecommendedLevel::Good:
			return "★★☆"; //"■■□";
		case VideoData::VideoCodecMiscInformation::RecommendedLevel::Best:
			return "★★★"; //"■■■";
	}
}

bool VideoData::get_videoEncoderIsLossless() const {
	return codecMiscInformation.at(get_videoEncoder()).lossless;
}
bool VideoData::get_videoEncoderSupportsAlpha() const {
	return codecMiscInformation.at(get_videoEncoder()).supportsAlpha;
}
std::string VideoData::get_videoEncoderRecommendationStr() const {
	return codecMiscInformation.at(get_videoEncoder()).get_recommendedStr();
}
std::string VideoData::get_videoEncoderInformationText() const {
	return codecMiscInformation.at(get_videoEncoder()).information_text;
}

const char** VideoData::get_videoPresetArray1() const {
	return codecToPresetArray1.at(get_videoEncoder()).presetArray.data();
}
const char** VideoData::get_videoPresetArray2() const {
	return codecToPresetArray2.at(get_videoEncoder()).presetArray.data();
}

int VideoData::get_videoPresetArray1_size() const {
	const std::vector<const char*>& codecArr = codecToPresetArray1.at(get_videoEncoder()).presetArray;
	return codecArr.size();
}
int VideoData::get_videoPresetArray2_size() const {
	const std::vector<const char*>& codecArr = codecToPresetArray2.at(get_videoEncoder()).presetArray;
	return codecArr.size();
}

std::string VideoData::get_videoPreset1() const {
	const std::vector<const char*>& codecArr = codecToPresetArray1.at(get_videoEncoder()).presetArray;
	return codecArr.empty() ? "default" : codecArr[videoPresetArray1_current];
}
std::string VideoData::get_videoPreset2() const {
	const std::vector<const char*>& codecArr = codecToPresetArray2.at(get_videoEncoder()).presetArray;
	return codecArr.empty() ? "default" : codecArr[videoPresetArray2_current];
}

void VideoData::update_videoCrfValues() {
	const VideoData::CrfData& data = codecToCrf.at(get_videoEncoder());
	crf_v = data.starting_value;
	crf_min = data.min_value;
	crf_max = data.max_value;
}

void VideoData::update_videoPresetArray() {
	videoPresetArray1_current = 0;
	videoPresetArray2_current = 0;

	const std::string codec = get_videoEncoder();
	videoCodec_hasPreset1 = !codecToPresetArray1.at(codec).presetArray.empty();
	videoCodec_hasPreset2 = !codecToPresetArray2.at(codec).presetArray.empty();
	videoCodec_preset1Term = codecToPresetArray1.at(codec).term;
	videoCodec_preset2Term = codecToPresetArray2.at(codec).term;
}
