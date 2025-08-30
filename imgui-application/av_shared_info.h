#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct CodecPresetInformation {
	std::string term;
	std::vector<const char*>& presetArray;
};

enum class CodecRecommendedLevel : uint8_t {
	No_Opinion,
	Awful,
	Okay,
	Good,
	Best,
};

struct AudioCodecMiscInformation {
	CodecRecommendedLevel recommendation;
	bool lossless;
	std::string information_text;
};

struct VideoCodecMiscInformation {
	CodecRecommendedLevel recommendation;
	bool lossless; //most codecs (used by this program) have a lossless mode, but this flag is for always lossless
	bool supportsAlpha;
	std::string information_text;
};
