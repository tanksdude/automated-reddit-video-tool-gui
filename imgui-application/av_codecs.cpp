#include "av_codecs.h"

// Defaults were obtained as follows:
// * mostly obtained through ffmpeg -h encoder=<>
// * some limits obtained through various sites
// * codec crf/bitrate ranges sometimes obtained empirically
// * Kdenlive CRF range (generally not used):
//   * H.264:   { 23, 15, 45, ... }
//   * H.265:   { 28, 15, 45, ... }
//   * VP8:     { 15,  5, 45, ... }
//   * VP9:     { 25, 15, 45, ... }
//   * AV1:     { 25, 15, 45, ... } (libaom-av1)
//   * AV1:     { 32,  5, 45, ... } (libsvtav1)

const GenericCodecPreset PRESET_AUDIO_AAC_encoding = {
	.displayTerm = "Encoding Method",
	.displayValues  = { "default", "anmr (not recommended) (NOT SUPPORTED)", "twoloop (default)", "fast" },
	.internalValues = { "default", "anmr",                                   "twoloop",           "fast" }
};

const GenericCodecPreset PRESET_AUDIO_Opus_compression = {
	.displayTerm = "Compression Level",
	.displayValues  = { "default", "0 (fast)", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10 (default)" },
	.internalValues = { "default", "0",        "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" }
};

const GenericCodecPreset PRESET_AUDIO_FLAC_compression = {
	.displayTerm = "Compression Level",
	.displayValues  = { "default", "0 (fast)", "1", "2", "3", "4", "5 (default)", "6", "7", "8", "9", "10", "11", "12" },
	.internalValues = { "default", "0",        "1", "2", "3", "4", "5",           "6", "7", "8", "9", "10", "11", "12" }
};

const GenericCodecPreset PRESET_AUDIO_MP3_compression = {
	.displayTerm = "Compression Level",
	.displayValues  = { "default", "0 (slow)", "1", "2", "3", "4", "5 (default)", "6", "7", "8", "9" }, //default is 5 according to its API
	.internalValues = { "default", "0",        "1", "2", "3", "4", "5",           "6", "7", "8", "9" }
};



const GenericCodecPreset PRESET_VIDEO_H264_preset = {
	.displayTerm = "Preset",
	.displayValues  = { "default", "ultrafast", "superfast", "veryfast", "faster", "fast", "medium (default)", "slow", "slower", "veryslow", "placebo (not recommended)" },
	.internalValues = { "default", "ultrafast", "superfast", "veryfast", "faster", "fast", "medium",           "slow", "slower", "veryslow", "placebo" }
};

const GenericCodecPreset PRESET_VIDEO_VP9_deadline = {
	.displayTerm = "Deadline",
	.displayValues  = { "default", "best", "good (default)", "realtime" },
	.internalValues = { "default", "best", "good",           "realtime" }
};

const GenericCodecPreset PRESET_VIDEO_AV1_usage = {
	.displayTerm = "Usage",
	.displayValues  = { "default", "good (default)", "realtime", "allintra" },
	.internalValues = { "default", "good",           "realtime", "allintra" }
};

// VP8 range: best doesn't seem to use it, 0-5 for good, realtime uses 0-15: //https://www.webmproject.org/docs/encoder-parameters/
// VP9 range: 0-5 for best & good, 0-8 for realtime: //https://trac.ffmpeg.org/wiki/Encode/VP9 (this says the default cpu-used is 0 though, so maybe it changed?)
// AV1 range: 0-8 for good, 7-10 for realtime (though >8 doesn't exist), unknown for allintra but assumed 0-8: //https://trac.ffmpeg.org/wiki/Encode/AV1
const GenericCodecPreset PRESET_VIDEO_VP9_cpu_used = {
	.displayTerm = "-cpu-used",
	.displayValues  = { "default", "0", "1 (default)", "2", "3", "4", "5", "6", "7", "8 (fast)" },
	.internalValues = { "default", "0", "1",           "2", "3", "4", "5", "6", "7", "8" }
};

// TODO: could also add -flags { "+il", "+im", "+ilm", "+ile", "+ime", "+ilme" }
const GenericCodecPreset PRESET_VIDEO_UtVideo_prediction = {
	.displayTerm = "Prediction",
	.displayValues  = { "default", "none", "left (default)", "gradient (NOT SUPPORTED)", "median" },
	.internalValues = { "default", "none", "left",           "gradient",                 "median" }
};

const GenericCodecPreset PRESET_VIDEO_CineForm_quality = {
	.displayTerm = "Quality",
	.displayValues  = { "default", "film3+ (default)", "film3", "film2+", "film2", "film1.5", "film1+", "film1", "high+", "high", "medium+", "medium", "low+", "low" },
	.internalValues = { "default", "film3+",           "film3", "film2+", "film2", "film1.5", "film1+", "film1", "high+", "high", "medium+", "medium", "low+", "low" }
};

const GenericCodecPreset PRESET_VIDEO_VVC_preset = {
	.displayTerm = "Preset",
	.displayValues  = { "default", "faster", "fast", "medium (default)", "slow", "slower" },
	.internalValues = { "default", "faster", "fast", "medium",           "slow", "slower" }
};

const GenericCodecPreset PRESET_VIDEO_EVC_preset = {
	.displayTerm = "Preset",
	.displayValues  = { "default",           "fast", "medium (default)", "slow",           "placebo" },
	.internalValues = { "default",           "fast", "medium",           "slow",           "placebo" }
};



const GenericCodecPreset PRESET_empty = { "", {}, {} };



const AudioCodecData CODEC_AUDIO_copypcm = {
	.displayName = "copy (pcm)",
	.internalName = "copy",
	.searchNames = { "copy", "pcm" },
	.preset1 = PRESET_empty,

	.bitrateInfo = {   0,   0,   0,   0,   0,   0 },
	.recommendation = CodecRecommendedLevel::Okay,
	.isLossless = true,
	.information_text = "Uncompressed raw audio. Not very useful. Should have the fastest decode speed."
};

const AudioCodecData CODEC_AUDIO_AAC = {
	.displayName = "AAC",
	.internalName = "AAC",
	.searchNames = { "AAC", "Advanced Audio Coding" },
	.preset1 = PRESET_AUDIO_AAC_encoding,

	.bitrateInfo = { 128,  30, 192, 128,   0, INT16_MAX }, //codec clamps bitrate when too high/low
	.recommendation = CodecRecommendedLevel::Good,
	.isLossless = false,
	.information_text = "Very widespread. Opus is faster and higher quality though."
};

const AudioCodecData CODEC_AUDIO_Opus = {
	.displayName = "Opus",
	.internalName = "Opus",
	.searchNames = { "Opus" },
	.preset1 = PRESET_AUDIO_Opus_compression,

	.bitrateInfo = {  96,  30, 128,  64,   1, 256 }, //actual minimum is .5k
	.recommendation = CodecRecommendedLevel::Best,
	.isLossless = false,
	.information_text = "Currently the best lossy audio codec. Very unlikely to encounter compatibility issues."
};

const AudioCodecData CODEC_AUDIO_FLAC = {
	.displayName = "FLAC",
	.internalName = "FLAC",
	.searchNames = { "FLAC", "Free Lossless Audio Codec" },
	.preset1 = PRESET_AUDIO_FLAC_compression,

	.bitrateInfo = {   0,   0,   0,   0,   0,   0 },
	.recommendation = CodecRecommendedLevel::Best,
	.isLossless = true,
	.information_text = "It's lossless. Smaller filesizes than raw audio."
};

const AudioCodecData CODEC_AUDIO_Vorbis = {
	.displayName = "Vorbis",
	.internalName = "Vorbis",
	.searchNames = { "Vorbis" },
	.preset1 = PRESET_empty,

	.bitrateInfo = {  90,  45,  90,  40,  16,  90 }, //default bitrate found empirically (sidenote: ffprobe couldn't read the bitrate from a .mkv, that's annoying); also Vorbis itself seems to support basically any bitrate, but libvorbis errors outside that range
	.recommendation = CodecRecommendedLevel::Okay,
	.isLossless = false,
	.information_text = "Succeeded by Opus. Not much reason to use it."
};

const AudioCodecData CODEC_AUDIO_MP3 = {
	.displayName = "MP3",
	.internalName = "MP3",
	.searchNames = { "MP3" },
	.preset1 = PRESET_AUDIO_MP3_compression,

	.bitrateInfo = { 128,  64, 256, 128,   0, INT16_MAX }, //codec clamps bitrate when too high/low; seems like the range is 8k-160k per channel
	.recommendation = CodecRecommendedLevel::Awful,
	.isLossless = false,
	.information_text = "There are better options; at least use its successor AAC."
};

const AudioCodecData CODEC_AUDIO_ALAC = {
	.displayName = "ALAC",
	.internalName = "ALAC",
	.searchNames = { "ALAC", "Apple Lossless" },
	.preset1 = PRESET_empty,

	.bitrateInfo = {   0,   0,   0,   0,   0,   0 },
	.recommendation = CodecRecommendedLevel::Good,
	.isLossless = true,
	.information_text = "It's lossless. FLAC probably has better support."
};

const AudioCodecData CODEC_AUDIO_AC3 = {
	.displayName = "AC-3",
	.internalName = "AC-3",
	.searchNames = { "AC-3", "Dolby AC-3", "Dolby Digital" },
	.preset1 = PRESET_empty,

	.bitrateInfo = { 192,  96, 448,  96,  32, 640 }, //min bitrate found empirically
	.recommendation = CodecRecommendedLevel::Awful,
	.isLossless = false,
	.information_text = "One of the oldest audio codecs. Still used in modern movies, for some reason."
};

const AudioCodecData CODEC_AUDIO_EAC3 = {
	.displayName = "E-AC-3",
	.internalName = "E-AC-3",
	.searchNames = { "E-AC-3", "Dolby Digital Plus", "Enhanced AC-3" },
	.preset1 = PRESET_empty,

	.bitrateInfo = { 128,  32, 384,  96,  12, 4096 }, //32k-6144k is the range according to Wikipedia; bitrate depends on sample rate and channels, probably like AC-3
	.recommendation = CodecRecommendedLevel::No_Opinion,
	.isLossless = false,
	.information_text = "AC-3's successor, and decently modern. Not a good choice for this program's usage, but definitely used elsewhere."
};

const AudioCodecData CODEC_AUDIO_Speex = {
	.displayName = "Speex",
	.internalName = "Speex",
	.searchNames = { "Speex" },
	.preset1 = PRESET_empty,

	.bitrateInfo = {  27,   3,  42,  27,   2,  44 }, //default bitrate found empirically; intended to use a 0-10 quality parameter rather than bitrate
	.recommendation = CodecRecommendedLevel::Okay,
	.isLossless = false,
	.information_text = "Succeeded by Opus. Designed for speech, while Vorbis is more general-purpose."
};

const AudioCodecData CODEC_AUDIO_TTA = {
	.displayName = "TTA True Audio",
	.internalName = "TTA",
	.searchNames = { "TTA", "True Audio", "TTA True Audio" },
	.preset1 = PRESET_empty,

	.bitrateInfo = {   0,   0,   0,   0,   0,   0 },
	.recommendation = CodecRecommendedLevel::Awful,
	.isLossless = true,
	.information_text = "It's lossless, but doesn't seem to be used anywhere..."
};

// The versions are 1, 2, 9/Pro, and Lossless, but 2 is the one exposed by Kdenlive so that's the one that will be offered (and 9/Pro and Lossless don't have encoders)
const AudioCodecData CODEC_AUDIO_WMA2 = {
	.displayName = "Windows Media Audio (2)",
	.internalName = "WMA2",
	.searchNames = { "WMA", "WMA2", "Windows Media Audio", "Windows Media Audio (2)" },
	.preset1 = PRESET_empty,

	.bitrateInfo = { 128,  64, 256, 128,  24, INT16_MAX }, //max bitrate empirically found to be 361267.088k, which is *very* weird, however nothing above ~5644.628k gets any more bits
	.recommendation = CodecRecommendedLevel::Awful,
	.isLossless = false,
	.information_text = "Competed with MP3. Doesn't reach transparency at any bitrate."
};

const AudioCodecData CODEC_AUDIO_MP2 = {
	.displayName = "MP2 / MPEG Audio Layer II",
	.internalName = "MP2",
	.searchNames = { "MP2", "MPEG Audio Layer II" },
	.preset1 = PRESET_empty,

	.bitrateInfo = { 128,  64, 160,  79,  32, 160 }, //bitrate range found empirically
	.recommendation = CodecRecommendedLevel::Awful,
	.isLossless = false,
	.information_text = "Competed with MP3 (wow the names are confusing), and succeeded by AAC. Still used in DVDs."
};



const VideoCodecData CODEC_VIDEO_H264 = {
	.displayName = "H.264",
	.internalName = "H.264",
	.searchNames = { "H.264", "AVC" },
	.preset1 = PRESET_VIDEO_H264_preset,
	.preset2 = PRESET_empty,

	.crfInfo = { 18, 17, 28, 23,  0, 51 },
	.recommendation = CodecRecommendedLevel::Best,
	.isLossless    = false,
	.supportsAlpha = false,
	.information_text = "The most widespread video codec. Fast encoding times."
};

const VideoCodecData CODEC_VIDEO_H265 = {
	.displayName = "H.265 / HEVC",
	.internalName = "H.265",
	.searchNames = { "H.265", "HEVC" },
	.preset1 = PRESET_VIDEO_H264_preset,
	.preset2 = PRESET_empty,

	.crfInfo = { 24, 23, 34, 28,  0, 51 }, //+6 to H.264
	.recommendation = CodecRecommendedLevel::Good,
	.isLossless    = false,
	.supportsAlpha = false,
	.information_text = "H.264's successor. Achieves smaller filesizes, though encoding takes longer. Also compression artifacts may be more noticeable."
};

const VideoCodecData CODEC_VIDEO_VP8 = {
	.displayName = "VP8",
	.internalName = "VP8",
	.searchNames = { "VP8" },
	.preset1 = PRESET_VIDEO_VP9_deadline,
	.preset2 = PRESET_VIDEO_VP9_cpu_used,

	.crfInfo = { 12,  5, 30, 32,  4, 63 }, //0 is the true CRF minimum but 4 is the default minimum
	.recommendation = CodecRecommendedLevel::Awful,
	.isLossless    = false,
	.supportsAlpha = true,
	.information_text = "Succeeded by VP9, and has very poor container support. Strongly discouraged from using."
};

const VideoCodecData CODEC_VIDEO_VP9 = {
	.displayName = "VP9",
	.internalName = "VP9",
	.searchNames = { "VP9" },
	.preset1 = PRESET_VIDEO_VP9_deadline,
	.preset2 = PRESET_VIDEO_VP9_cpu_used,

	.crfInfo = { 31, 15, 35, 32,  0, 63 },
	.recommendation = CodecRecommendedLevel::Good,
	.isLossless    = false,
	.supportsAlpha = true,
	.information_text = "About the same quality as HEVC with faster encode times. "
	                    "(Though it has very poor support for multithreading, so outside of this application it will probably be slower so you might perfer HEVC.)\n\n"
	                    "This application does 1-pass encoding, though the codec was designed with 2-pass in mind."
};

const VideoCodecData CODEC_VIDEO_AV1 = {
	.displayName = "AV1",
	.internalName = "AV1",
	.searchNames = { "AV1" },
	.preset1 = PRESET_VIDEO_AV1_usage,
	.preset2 = PRESET_VIDEO_VP9_cpu_used,

	.crfInfo = { 23, 20, 35, 32,  0, 63 },
	.recommendation = CodecRecommendedLevel::Okay,
	.isLossless    = false,
	.supportsAlpha = false, // The standard seems to support alpha, but libaom-av1 does not (yet?)
	.information_text = "VP9's successor and fully open-source. Even better quality, however it is *extremely* slow."
};

const VideoCodecData CODEC_VIDEO_FFV1 = {
	.displayName = "FFV1",
	.internalName = "FFV1",
	.searchNames = { "FFV1" },
	.preset1 = PRESET_empty,
	.preset2 = PRESET_empty,

	.crfInfo = {  0,  0,  0,  0,  0,  0 },
	.recommendation = CodecRecommendedLevel::Best,
	.isLossless    = true,
	.supportsAlpha = true,
	.information_text = "It's lossless. Might not be supported by your video editor, in which case try Ut Video." //NBSP for Ut Video
};

const VideoCodecData CODEC_VIDEO_UtVideo = {
	.displayName = "Ut Video",
	.internalName = "Ut_Video",
	.searchNames = { "Ut Video", "Ut_Video", "Ut Video Codec Suite" },
	.preset1 = PRESET_VIDEO_UtVideo_prediction,
	.preset2 = PRESET_empty,

	.crfInfo = {  0,  0,  0,  0,  0,  0 },
	.recommendation = CodecRecommendedLevel::Good,
	.isLossless    = true,
	.supportsAlpha = true,
	.information_text = "It's lossless. In general, faster to encode than FFV1 at the cost of much larger filesizes.\n"
	                    "It is the codec used by OBS's lossless capture."
};

const VideoCodecData CODEC_VIDEO_ProRes = {
	.displayName = "Apple ProRes",
	.internalName = "ProRes",
	.searchNames = { "Apple ProRes", "ProRes" },
	.preset1 = PRESET_empty,
	.preset2 = PRESET_empty,

	.crfInfo = {  0,  0,  0, UINT8_MAX, UINT8_MAX, UINT8_MAX },
	.recommendation = CodecRecommendedLevel::Okay,
	.isLossless    = true, // not lossless, but doesn't support CRF mode
	.supportsAlpha = true,
	.information_text = "Very professional, designed as a high-end codec. Not a great choice for this application."
};

const VideoCodecData CODEC_VIDEO_QuickTime = {
	.displayName = "QuickTime Animation",
	.internalName = "QuickTime",
	.searchNames = { "QuickTime", "QuickTime Animation", "QuickTime RLE" },
	.preset1 = PRESET_empty,
	.preset2 = PRESET_empty,

	.crfInfo = {  0,  0,  0,  0,  0,  0 }, //apparently has a lossy mode
	.recommendation = CodecRecommendedLevel::Okay,
	.isLossless    = true,
	.supportsAlpha = true,
	.information_text = "Lossless and supports an alpha channel. Doesn't seem to be commonly used. ProRes may be better."
};

const VideoCodecData CODEC_VIDEO_CineForm = {
	.displayName = "CineForm",
	.internalName = "CineForm",
	.searchNames = { "CineForm" },
	.preset1 = PRESET_VIDEO_CineForm_quality,
	.preset2 = PRESET_empty,

	.crfInfo = {  0,  0,  0, UINT8_MAX, UINT8_MAX, UINT8_MAX }, //intended to use -q instead of CRF
	.recommendation = CodecRecommendedLevel::Good,
	.isLossless    = true, // not lossless, but doesn't support CRF mode (though it is designed as constant quality / variable bitrate)
	.supportsAlpha = true,
	.information_text = "Decent support. Great random access seeking (intra-frame-only compression)."
};

const VideoCodecData CODEC_VIDEO_LosslessH264 = {
	.displayName = "H.264 but lossless",
	.internalName = "H.264_Lossless",
	.searchNames = { "H.264 but lossless", "H.264_Lossless", "H.264 Lossless", "Lossless_H.264", "Lossless H.264" },
	.preset1 = PRESET_VIDEO_H264_preset,
	.preset2 = PRESET_empty,

	.crfInfo = {  0,  0,  0,  0,  0,  0 },
	.recommendation = CodecRecommendedLevel::Okay,
	.isLossless    = true,
	.supportsAlpha = false,
	.information_text = "H.264 but lossless. Manages to achieve smaller filesizes than FFV1.\n\n"
	                    "Some video editors do not support lossless H.264."
};

const VideoCodecData CODEC_VIDEO_VVC = {
	.displayName = "VVC / H.266",
	.internalName = "VVC", //not sure
	.searchNames = { "VVC", "H.266" },
	.preset1 = PRESET_VIDEO_VVC_preset,
	.preset2 = PRESET_empty,

	.crfInfo = { 27, 26, 37, UINT8_MAX, UINT8_MAX, UINT8_MAX }, //+3 to H.265
	.recommendation = CodecRecommendedLevel::Awful,
	.isLossless    = true, // not lossless, but doesn't support CRF mode, might in the future
	.supportsAlpha = false,
	.information_text = "H.265's successor, competing with AV1. Very poor support."
};

const VideoCodecData CODEC_VIDEO_EVC = {
	.displayName = "EVC / MPEG-5 Part 1",
	.internalName = "EVC", //not sure
	.searchNames = { "EVC", "MPEG-5 Part 1" },
	.preset1 = PRESET_VIDEO_EVC_preset,
	.preset2 = PRESET_empty,

	.crfInfo = { 30, 29, 40, 34, 10, 49 }, //+6 to H.265, uncertain default CRF
	.recommendation = CodecRecommendedLevel::Awful,
	.isLossless    = false,
	.supportsAlpha = false,
	.information_text = "Seems to be competing with VVC. Very poor support."
};
