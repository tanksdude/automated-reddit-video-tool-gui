#pragma once
#include <string>
#include <vector>
#include <cstdint>

enum class CodecRecommendedLevel : uint8_t {
	No_Opinion,
	Awful,
	Okay,
	Good,
	Best,
};

struct GenericCodecPreset {
	const char* displayTerm;
	std::vector<const char*> displayValues;
	std::vector<std::string> internalValues; //also the search ones, no need to make another list
};

struct AudioCodecData {
	struct BitrateData {
		// Bitrate in kbps
		uint16_t starting_value;
		uint16_t min_value; //"sane" min
		uint16_t max_value; //"sane" max
		uint16_t codec_default_value; //unused
		uint16_t codec_min_value;     //used when clamping inputs
		uint16_t codec_max_value;     //used when clamping inputs
	};

	const char* displayName;
	const char* internalName;
	std::vector<std::string> searchNames;
	const GenericCodecPreset& preset1;
	//std::vector<const char*> supportedContainers = {}; //TODO

	BitrateData bitrateInfo;
	CodecRecommendedLevel recommendation;
	bool isLossless;
	const char* information_text;
};

struct VideoCodecData {
	struct CrfData {
		// A value of -1 is technically allowed, but until something actually uses it this will stay unsigned
		uint8_t starting_value;
		uint8_t min_value; //"sane" min
		uint8_t max_value; //"sane" max
		uint8_t codec_default_value; //unused
		uint8_t codec_min_value;     //used when clamping inputs
		uint8_t codec_max_value;     //used when clamping inputs
	};

	const char* displayName;
	const char* internalName;
	std::vector<std::string> searchNames;
	const GenericCodecPreset& preset1;
	const GenericCodecPreset& preset2;
	//std::vector<const GenericCodecPreset&> preset2_list; //different presets depending on the first's value
	//std::vector<const char*> supportedContainers = {}; //TODO
	CrfData crfInfo;

	CodecRecommendedLevel recommendation;
	bool isLossless; //most codecs (used by this program) have a lossless mode, but this flag is for always lossless
	bool supportsAlpha;
	const char* information_text;
};



extern const GenericCodecPreset PRESET_AUDIO_AAC_encoding;
extern const GenericCodecPreset PRESET_AUDIO_Opus_compression;
extern const GenericCodecPreset PRESET_AUDIO_FLAC_compression;
extern const GenericCodecPreset PRESET_AUDIO_MP3_compression;
//other possibilities: flac lpc_coeff_precision, libopus application, aac profile ("low complexity" default, "main", "scalable sampling rate")

extern const GenericCodecPreset PRESET_VIDEO_H264_preset;
extern const GenericCodecPreset PRESET_VIDEO_VP9_deadline;
extern const GenericCodecPreset PRESET_VIDEO_AV1_usage;
extern const GenericCodecPreset PRESET_VIDEO_VP9_cpu_used;
extern const GenericCodecPreset PRESET_VIDEO_UtVideo_prediction;
extern const GenericCodecPreset PRESET_VIDEO_CineForm_quality;
extern const GenericCodecPreset PRESET_VIDEO_VVC_preset;
extern const GenericCodecPreset PRESET_VIDEO_EVC_preset;

extern const GenericCodecPreset PRESET_empty;



extern const AudioCodecData CODEC_AUDIO_copypcm;
extern const AudioCodecData CODEC_AUDIO_AAC;
extern const AudioCodecData CODEC_AUDIO_Opus;
extern const AudioCodecData CODEC_AUDIO_FLAC;
extern const AudioCodecData CODEC_AUDIO_Vorbis;
extern const AudioCodecData CODEC_AUDIO_MP3;
extern const AudioCodecData CODEC_AUDIO_ALAC;
extern const AudioCodecData CODEC_AUDIO_AC3;
extern const AudioCodecData CODEC_AUDIO_EAC3;
extern const AudioCodecData CODEC_AUDIO_Speex;
extern const AudioCodecData CODEC_AUDIO_TTA;
extern const AudioCodecData CODEC_AUDIO_WMA2;
extern const AudioCodecData CODEC_AUDIO_MP2;

extern const VideoCodecData CODEC_VIDEO_H264;
extern const VideoCodecData CODEC_VIDEO_H265;
extern const VideoCodecData CODEC_VIDEO_VP8;
extern const VideoCodecData CODEC_VIDEO_VP9;
extern const VideoCodecData CODEC_VIDEO_AV1;
extern const VideoCodecData CODEC_VIDEO_FFV1;
extern const VideoCodecData CODEC_VIDEO_UtVideo;
extern const VideoCodecData CODEC_VIDEO_ProRes;
extern const VideoCodecData CODEC_VIDEO_QuickTime;
extern const VideoCodecData CODEC_VIDEO_CineForm;
extern const VideoCodecData CODEC_VIDEO_LosslessH264;
extern const VideoCodecData CODEC_VIDEO_VVC;
extern const VideoCodecData CODEC_VIDEO_EVC;
