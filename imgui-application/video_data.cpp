#include "video_data.h"

const char* VideoData::videoEncoderArray[7]          = { "H.264", "H.265 / HEVC", "VP8", "VP9", "AV1", "FFV1", "Ut Video" };
const char* VideoData::videoEncoderArrayExtended[13] = { "H.264", "H.265 / HEVC", "VP8", "VP9", "AV1", "FFV1", "Ut Video", "Apple ProRes", "QuickTime Animation", "CineForm", "Lossless_H.264", "VVC / H.266", "EVC / MPEG-5 Part 1" };
std::vector<const char*> VideoData::videoPresetArray_H264_preset = { "default", "ultrafast", "superfast", "veryfast", "faster", "fast", "medium (default)", "slow", "slower", "veryslow", "placebo (not recommended)" };
std::vector<const char*> VideoData::videoPresetArray_VP9_deadline = { "default", "best", "good (default)", "realtime" };
std::vector<const char*> VideoData::videoPresetArray_AV1_usage    = { "default", "good (default)", "realtime", "allintra" };
std::vector<const char*> VideoData::videoPresetArray_VP9_cpu_used = { "default", "0", "1 (default)", "2", "3", "4", "5", "6", "7", "8" }; //TODO: the range changes based on the deadline
std::vector<const char*> VideoData::videoPresetArray_UtVideo_prediction = { "default", "none", "left (default)", "gradient (NOT SUPPORTED)", "median" }; //https://ffmpeg.org/doxygen/3.0/libutvideoenc_8cpp.html
std::vector<const char*> VideoData::videoPresetArray_empty = {};
std::vector<const char*> VideoData::videoPresetArray_CineForm_quality = { "default", "film3+ (default)", "film3", "film2+", "film2", "film1.5", "film1+", "film1", "high+", "high", "medium+", "medium", "low+", "low" };
std::vector<const char*> VideoData::videoPresetArray_VVC_preset = { "default", "faster", "fast", "medium (default)", "slow", "slower" };
std::vector<const char*> VideoData::videoPresetArray_EVC_preset = { "default",           "fast", "medium (default)", "slow", "placebo" };
const char* VideoData::videoContainerArray[6] = { ".mp4", ".mkv", ".mov", ".webm", ".ogg", ".avi" }; //TODO: .ogg has really poor support for codecs, so find a way to communicate that or disable/warn on certain codecs; could remove codecs based on the container or vice versa
const char* VideoData::fpsArray[9] = { "10", "20", "25", "30", "50", "60", "75", "90", "120" };

const std::unordered_map<std::string, CodecPresetInformation> VideoData::codecToPresetArray1 = {
	{ "H.264",        { "Preset", videoPresetArray_H264_preset } },
	{ "H.265 / HEVC", { "Preset", videoPresetArray_H264_preset } },
	{ "VP8",          { "Deadline", videoPresetArray_VP9_deadline } },
	{ "VP9",          { "Deadline", videoPresetArray_VP9_deadline } },
	{ "AV1",          { "Usage", videoPresetArray_AV1_usage } },
	{ "FFV1",         { "", videoPresetArray_empty } },
	{ "Ut Video",     { "Prediction", videoPresetArray_UtVideo_prediction } },

	{ "Apple ProRes",        { "", videoPresetArray_empty } },
	{ "QuickTime Animation", { "", videoPresetArray_empty } },
	{ "CineForm",            { "Quality", videoPresetArray_CineForm_quality } },
	{ "Lossless_H.264",      { "Preset", videoPresetArray_H264_preset } },
	{ "VVC / H.266",         { "Preset", videoPresetArray_VVC_preset } },
	{ "EVC / MPEG-5 Part 1", { "Preset", videoPresetArray_EVC_preset } },
};

const std::unordered_map<std::string, CodecPresetInformation> VideoData::codecToPresetArray2 = {
	{ "H.264",        { "", videoPresetArray_empty } },
	{ "H.265 / HEVC", { "", videoPresetArray_empty } },
	{ "VP8",          { "-cpu-used", videoPresetArray_VP9_cpu_used } },
	{ "VP9",          { "-cpu-used", videoPresetArray_VP9_cpu_used } }, //0-5 for best & good, 0-8 for realtime
	{ "AV1",          { "-cpu-used", videoPresetArray_VP9_cpu_used } }, //lacking information on the valid range
	{ "FFV1",         { "", videoPresetArray_empty } },
	{ "Ut Video",     { "", videoPresetArray_empty } }, //TODO: -flags +ilme (don't wanna do a checkbox, so maybe { "il", "im", "ilm", "ile", "ime", "ilme" })

	{ "Apple ProRes",        { "", videoPresetArray_empty } },
	{ "QuickTime Animation", { "", videoPresetArray_empty } },
	{ "CineForm",            { "", videoPresetArray_empty } },
	{ "Lossless_H.264",      { "", videoPresetArray_empty } },
	{ "VVC / H.266",         { "", videoPresetArray_empty } },
	{ "EVC / MPEG-5 Part 1", { "", videoPresetArray_empty } },
};

//note: video_data.cpp *needs* to be included before main.cpp to initialize this, otherwise update_videoCrfValues() in the constructor will be called when there's zero elements
//apart from H.264, the recommended starting value and range are mostly guesses
const std::unordered_map<std::string, VideoData::CrfData> VideoData::codecToCrf = {
	{ "H.264",        { 18, 17, 28, 23,  0, 51 } },
	{ "H.265 / HEVC", { 24, 23, 34, 28,  0, 51 } }, //+6 to H.264
	{ "VP8",          { 12,  5, 30, 32,  4, 63 } }, //0 is the true crf minimum but 4 is the default minimum
	{ "VP9",          { 31, 15, 35, 32,  0, 63 } },
	{ "AV1",          { 23, 20, 35, 32,  0, 63 } },
	{ "FFV1",         {  0,  0,  0,  0,  0,  0 } },
	{ "Ut Video",     {  0,  0,  0,  0,  0,  0 } },

	{ "Apple ProRes",        {  0,  0,  0, -1, -1, -1 } }, //not lossless, see CineForm's comment
	{ "QuickTime Animation", {  0,  0,  0,  0,  0,  0 } }, //apparently has a lossy mode
	{ "CineForm",            {  0,  0,  0, -1, -1, -1 } }, //not lossless, however it uses -q or -b:v instead of CRF, but also the filesize doesn't change, so it seems for this specific application quality settings don't matter
	{ "Lossless_H.264",      {  0,  0,  0,  0,  0,  0 } },
	{ "VVC / H.266",         { 27, 26, 37, -1, -1, -1 } }, //+3 to H.265, but doesn't support crf mode
	{ "EVC / MPEG-5 Part 1", { 30, 29, 40, 34, 10, 49 } }, //+6 to H.265, uncertain default crf
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

const std::unordered_map<std::string, VideoCodecMiscInformation> VideoData::codecMiscInformation = {
	{ "H.264",        { .recommendation=CodecRecommendedLevel::Best,  .lossless=false, .supportsAlpha=false, .information_text="The most widespread video codec. Fast encoding times." } },
	{ "H.265 / HEVC", { .recommendation=CodecRecommendedLevel::Okay,  .lossless=false, .supportsAlpha=false, .information_text="H.264's successor. Achieves smaller filesizes, though encoding takes longer. Also compression artifacts may be more noticeable." } },
	{ "VP8",          { .recommendation=CodecRecommendedLevel::Awful, .lossless=false, .supportsAlpha=true,  .information_text="Succeeded by VP9, and has very poor container support. Strongly discouraged from using." } },
	{ "VP9",          { .recommendation=CodecRecommendedLevel::Good,  .lossless=false, .supportsAlpha=true,  .information_text="About the same quality as HEVC with faster encode times. (Though it has very poor support for multithreading, so outside of this application it will probably be slower so you might perfer HEVC.)\nThis application does 1-pass encoding, though the codec was designed with 2-pass in mind." } },
	{ "AV1",          { .recommendation=CodecRecommendedLevel::Okay,  .lossless=false, .supportsAlpha=false, .information_text="VP9's successor and fully open-source. Even better quality, however it is *extremely* slow." } }, // The standard seems to support alpha, but libaom-av1 does not (yet?)
	{ "FFV1",         { .recommendation=CodecRecommendedLevel::Best,  .lossless=true,  .supportsAlpha=true,  .information_text="It's lossless. Might not be supported by your video editor, in which case try Ut Video." } }, //NBSP for Ut Video
	{ "Ut Video",     { .recommendation=CodecRecommendedLevel::Good,  .lossless=true,  .supportsAlpha=true,  .information_text="It's lossless. In general, faster to encode than FFV1 at the cost of much larger filesizes. It is the codec used by OBS's lossless capture." } },

	{ "Apple ProRes",        { .recommendation=CodecRecommendedLevel::Okay,  .lossless=true /* not lossless, see CineForm's comment */, .supportsAlpha=true, .information_text="Very professional, designed as a high-end codec. Not a great choice for this application." } },
	{ "QuickTime Animation", { .recommendation=CodecRecommendedLevel::Okay,  .lossless=true,  .supportsAlpha=true, .information_text="Lossless and supports an alpha channel. Doesn't seem to be commonly used. ProRes may be better." } },
	{ "CineForm",            { .recommendation=CodecRecommendedLevel::Good,  .lossless=true /* not lossless, this is just to get rid of the CRF slider as CineForm does not use CRF */, .supportsAlpha=true,  .information_text="Decent support. Great random access seeking (intra-frame-only compression)." } },
	{ "Lossless_H.264",      { .recommendation=CodecRecommendedLevel::Okay,  .lossless=true,  .supportsAlpha=false, .information_text="H.264 but lossless. Somehow manages to achieve smaller filesizes than FFV1.\nSome video editors do not support lossless H.264." } },
	{ "VVC / H.266",         { .recommendation=CodecRecommendedLevel::Awful, .lossless=true /* not lossless, but doesn't support CRF mode, might in the future */, .supportsAlpha=false, .information_text="H.265's successor, competing with AV1. Very poor support." } },
	{ "EVC / MPEG-5 Part 1", { .recommendation=CodecRecommendedLevel::Awful, .lossless=false, .supportsAlpha=false, .information_text="Seems to be competing with VVC. Very poor support." } },

	// Theora is another codec that could be added to pad out the list, however it doesn't use CRF, so that's a bunch of extra work; though it's been discontinued by Firefox and Chrome, so why bother
};

CodecRecommendedLevel VideoData::get_videoEncoderRecommendation() const {
	return codecMiscInformation.at(get_videoEncoder()).recommendation;
}
bool VideoData::get_videoEncoderIsLossless() const {
	return codecMiscInformation.at(get_videoEncoder()).lossless;
}
bool VideoData::get_videoEncoderSupportsAlpha() const {
	return codecMiscInformation.at(get_videoEncoder()).supportsAlpha;
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
