#include "video_data.h"
#include <algorithm> //std::clamp

const std::array<const VideoCodecData*, 7>  VideoData::videoEncoderArray         = { &CODEC_VIDEO_H264, &CODEC_VIDEO_H265, &CODEC_VIDEO_VP8, &CODEC_VIDEO_VP9, &CODEC_VIDEO_AV1, &CODEC_VIDEO_FFV1, &CODEC_VIDEO_UtVideo };
const std::array<const VideoCodecData*, 13> VideoData::videoEncoderArrayExtended = { &CODEC_VIDEO_H264, &CODEC_VIDEO_H265, &CODEC_VIDEO_VP8, &CODEC_VIDEO_VP9, &CODEC_VIDEO_AV1, &CODEC_VIDEO_FFV1, &CODEC_VIDEO_UtVideo, &CODEC_VIDEO_ProRes, &CODEC_VIDEO_QuickTime, &CODEC_VIDEO_CineForm, &CODEC_VIDEO_LosslessH264, &CODEC_VIDEO_VVC, &CODEC_VIDEO_EVC };
const std::array<const char*, 6>  VideoData::videoContainerArray = { ".mp4", ".mkv", ".mov", ".webm", ".ogg", ".avi" }; //TODO: .ogg has really poor support for codecs, so find a way to communicate that or disable/warn on certain codecs; could remove codecs based on the container or vice versa
const std::array<const char*, 10> VideoData::fpsArray = { "10", "20", "25", "30", "50", "60", "75", "90", "100", "120" };

void VideoData::update_videoEncoderValues() {
	// CRF:
	const VideoCodecData::CrfData& data = get_videoEncoder()->crfInfo;
	video_crf_v = data.starting_value;
	video_crf_min = data.min_value;
	video_crf_max = data.max_value;
}

void VideoData::set_videoCrf(int8_t val) {
	const VideoCodecData::CrfData& data = get_videoEncoder()->crfInfo;
	video_crf_v = std::clamp(val, data.codec_min_value, data.codec_max_value);
}
