#pragma once
#include "image_data.h"
#include "audio_data.h"
#include "video_data.h"
#include "program_data.h"
#include <cstring> //strncpy_s
#include <iostream>
#include <fstream>
#define MINI_CASE_SENSITIVE
#include <mini/ini.h>

namespace ARVT {

void CreateDefaultIni() {
	static std::string ini_string =

	"[APPLICATION]\n"
	"\n"
	"ApplicationFont =\n"
	"ApplicationFontSize =\n"
	"\n"
	"UseExtraCodecs = false\n"
	"\n"

	"[IMAGE]\n"
	"\n"
	"ImageWidth = 960\n"
	"ImageHeight = 640\n"
	"ImageBorderW = 32\n"
	"ImageBorderH = 32\n"
	"\n"
	"FontSize = 16\n"
	"FontColor = white\n"
	"BackgroundColor = black\n"
	"\n"
	"ParagraphNewlineCount = 2\n"
	"ParagraphTabbedStart = false\n"
	"\n"
	"ImageFormat = .png\n"
	"\n"

	"[AUDIO]\n"
	"\n"
	"VoiceEngine =\n"
	"VoiceName =\n"
	"\n"
	"AudioEncoder = copy\n"
	"AudioBitrateKbps = 128\n"
	"AudioPreset = default\n"
	"\n"

	"[VIDEO]\n"
	"\n"
	"VideoEncoder = H.264\n"
	"VideoPreset1 = default\n"
	"VideoPreset2 = default\n"
	"\n"
	"VideoFPS = 60\n"
	"VideoCRF = 23\n"
	"\n"
	"VideoFaststartIfPossible = 0\n"
	"VideoContainer = .mp4\n"
	"AudioOnly = false\n"
	;

	std::ofstream ini_file;
	ini_file.open("../arvt.ini");
	if (ini_file.is_open()) {
		ini_file << ini_string;
		ini_file.close();
	} else {
		//TODO: could not create
	}
}

void Fill_ImageData(ImageData& idata, const mINI::INIStructure& ini_object) {
	if (!ini_object.has("IMAGE")) {
		return;
	}

	if (ini_object.get("IMAGE").has("ImageWidth")) {
		std::string get = ini_object.get("IMAGE").get("ImageWidth");
		strncpy_s(idata.image_width_input, sizeof(idata.image_width_input)/sizeof(*idata.image_width_input), get.c_str(), get.size());
	}
	if (ini_object.get("IMAGE").has("ImageHeight")) {
		std::string get = ini_object.get("IMAGE").get("ImageHeight");
		strncpy_s(idata.image_height_input, sizeof(idata.image_height_input)/sizeof(*idata.image_height_input), get.c_str(), get.size());
	}

	if (ini_object.get("IMAGE").has("ImageBorderW")) {
		std::string get = ini_object.get("IMAGE").get("ImageBorderW");
		strncpy_s(idata.image_w_border_input, sizeof(idata.image_w_border_input)/sizeof(*idata.image_w_border_input), get.c_str(), get.size());
	}
	if (ini_object.get("IMAGE").has("ImageBorderH")) {
		std::string get = ini_object.get("IMAGE").get("ImageBorderH");
		strncpy_s(idata.image_h_border_input, sizeof(idata.image_h_border_input)/sizeof(*idata.image_h_border_input), get.c_str(), get.size());
	}

	if (ini_object.get("IMAGE").has("FontSize")) {
		std::string get = ini_object.get("IMAGE").get("FontSize");
		strncpy_s(idata.font_size_input, sizeof(idata.font_size_input)/sizeof(*idata.font_size_input), get.c_str(), get.size());
	}

	if (ini_object.get("IMAGE").has("FontColor")) {
		std::string get = ini_object.get("IMAGE").get("FontColor");
		strncpy_s(idata.font_color_input, sizeof(idata.font_color_input)/sizeof(*idata.font_color_input), get.c_str(), get.size());
	}

	if (ini_object.get("IMAGE").has("BackgroundColor")) {
		std::string get = ini_object.get("IMAGE").get("BackgroundColor");
		strncpy_s(idata.background_color_input, sizeof(idata.background_color_input)/sizeof(*idata.background_color_input), get.c_str(), get.size());
	}

	if (ini_object.get("IMAGE").has("ParagraphNewlineCount")) {
		std::string get = ini_object.get("IMAGE").get("ParagraphNewlineCount");
		try {
			uint8_t val = std::stoi(get);
			idata.paragraph_newline_v = val;
		}
		catch (const std::exception&) {
			std::cerr << ("Unable to parse [IMAGE].ParagraphNewlineCount: \"" + get + "\"") << std::endl;
		}
	}

	if (ini_object.get("IMAGE").has("ParagraphTabbedStart")) {
		std::string get = ini_object.get("IMAGE").get("ParagraphTabbedStart");
		if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
			idata.paragraph_tabbed_start_input = true;
		}
	}

	if (ini_object.get("IMAGE").has("ImageFormat")) {
		std::string get = ini_object.get("IMAGE").get("ImageFormat");
		size_t index = std::distance(idata.imageFormatArray, std::find(idata.imageFormatArray, idata.imageFormatArray + sizeof(idata.imageFormatArray) / sizeof(*idata.imageFormatArray), get));
		if (index != sizeof(idata.imageFormatArray) / sizeof(*idata.imageFormatArray)) {
			idata.imageFormatArray_current = index;
		}
	}
}

void Fill_AudioData(AudioData& adata, const mINI::INIStructure& ini_object) {
	if (!ini_object.has("AUDIO")) {
		return;
	}

	//TODO
	if (ini_object.get("AUDIO").has("VoiceEngine")) {
		std::string get = ini_object.get("AUDIO").get("VoiceEngine");
		size_t index = std::distance(adata.voiceEngineArray, std::find(adata.voiceEngineArray, adata.voiceEngineArray + sizeof(adata.voiceEngineArray) / sizeof(*adata.voiceEngineArray), get));
		if (index != sizeof(adata.voiceEngineArray) / sizeof(*adata.voiceEngineArray)) {
			adata.voiceEngineArray_current = index;
		}
	}

	//TODO
	if (ini_object.get("AUDIO").has("VoiceName")) {
		std::string get = ini_object.get("AUDIO").get("VoiceName");
		size_t index = std::distance(adata.voiceArray, std::find(adata.voiceArray, adata.voiceArray + adata.voiceArray_length, get));
		if (index != adata.voiceArray_length) {
			adata.voiceArray_current = index;
		}
	}

	//TODO
	if (ini_object.get("AUDIO").has("AudioEncoder")) {
		std::string get = ini_object.get("AUDIO").get("AudioEncoder");
		size_t index = std::distance(adata.audioEncoderArray, std::find(adata.audioEncoderArray, adata.audioEncoderArray + sizeof(adata.audioEncoderArray) / sizeof(*adata.audioEncoderArray), get));
		if (index != sizeof(adata.audioEncoderArray) / sizeof(*adata.audioEncoderArray)) {
			adata.audioEncoderArray_current = index;
		}
	}

	//TODO
	if (ini_object.get("AUDIO").has("AudioBitrateKbps")) {
		std::string get = ini_object.get("AUDIO").get("AudioBitrateKbps");
		try {
			int16_t val = std::stoi(get);
			adata.audio_bitrate_v = val;
		}
		catch (const std::exception&) {
			std::cerr << ("Unable to parse [AUDIO].AudioBitrateKbps: \"" + get + "\"") << std::endl;
		}
	}

	//TODO
	if (ini_object.get("AUDIO").has("AudioPreset")) {
		std::string get = ini_object.get("AUDIO").get("AudioPreset");
		size_t index = std::distance(adata.get_audioPresetArray(), std::find(adata.get_audioPresetArray(), adata.get_audioPresetArray() + adata.get_audioPresetArray_size(), get));
		if (index != adata.get_audioPresetArray_size()) {
			adata.audioPresetArray_current = index;
		}
	}
}

void Fill_VideoData(VideoData& vdata, const mINI::INIStructure& ini_object) {
	if (!ini_object.has("VIDEO")) {
		return;
	}

	//TODO
	if (ini_object.get("VIDEO").has("VideoEncoder")) {
		std::string get = ini_object.get("VIDEO").get("VideoEncoder");
		size_t index = std::distance(vdata.videoEncoderArray, std::find(vdata.videoEncoderArray, vdata.videoEncoderArray + sizeof(vdata.videoEncoderArray) / sizeof(*vdata.videoEncoderArray), get));
		if (index != sizeof(vdata.videoEncoderArray) / sizeof(*vdata.videoEncoderArray)) {
			vdata.videoEncoderArray_current = index;
		}
	}

	//TODO
	if (ini_object.get("VIDEO").has("VideoPreset1")) {
		std::string get = ini_object.get("VIDEO").get("VideoPreset1");
		size_t index = std::distance(vdata.get_videoPresetArray1(), std::find(vdata.get_videoPresetArray1(), vdata.get_videoPresetArray1() + vdata.get_videoPresetArray1_size(), get));
		if (index != vdata.get_videoPresetArray1_size()) {
			vdata.videoPresetArray1_current = index;
		}
	}

	//TODO
	if (ini_object.get("VIDEO").has("VideoPreset2")) {
		std::string get = ini_object.get("VIDEO").get("VideoPreset2");
		size_t index = std::distance(vdata.get_videoPresetArray2(), std::find(vdata.get_videoPresetArray2(), vdata.get_videoPresetArray2() + vdata.get_videoPresetArray2_size(), get));
		if (index != vdata.get_videoPresetArray2_size()) {
			vdata.videoPresetArray2_current = index;
		}
	}

	if (ini_object.get("VIDEO").has("VideoFPS")) {
		std::string get = ini_object.get("VIDEO").get("VideoFPS");
		//TODO: this is the most complicated
		
		if (false) {
			vdata.fractionalFps = true;
			//TODO
		} else {
			size_t index = std::distance(vdata.fpsArray, std::find(vdata.fpsArray, vdata.fpsArray + sizeof(vdata.fpsArray) / sizeof(*vdata.fpsArray), get));
			if (index != sizeof(vdata.fpsArray) / sizeof(*vdata.fpsArray)) {
				vdata.fpsArray_current = index;
			} else {
				//TODO
			}
		}
	}

	//TODO
	if (ini_object.get("VIDEO").has("VideoCRF")) {
		std::string get = ini_object.get("VIDEO").get("VideoCRF");
		try {
			int8_t val = std::stoi(get);
			vdata.crf_v = val;
		}
		catch (const std::exception&) {
			std::cerr << ("Unable to parse [VIDEO].VideoCRF: \"" + get + "\"") << std::endl;
		}
	}

	if (ini_object.get("VIDEO").has("VideoFaststartIfPossible")) {
		std::string get = ini_object.get("VIDEO").get("VideoFaststartIfPossible");
		if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
			vdata.faststart_flag = true;
		}
	}

	if (ini_object.get("VIDEO").has("VideoContainer")) {
		std::string get = ini_object.get("VIDEO").get("VideoContainer");
		size_t index = std::distance(vdata.videoContainerArray, std::find(vdata.videoContainerArray, vdata.videoContainerArray + sizeof(vdata.videoContainerArray) / sizeof(*vdata.videoContainerArray), get));
		if (index != sizeof(vdata.videoContainerArray) / sizeof(*vdata.videoContainerArray)) {
			vdata.videoContainerArray_current = index;
		}
	}

	if (ini_object.get("VIDEO").has("AudioOnly")) {
		std::string get = ini_object.get("VIDEO").get("AudioOnly");
		if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
			vdata.audio_only_option_input = true;
		}
	}

}

void Fill_ProgramData(ProgramData& pdata, const mINI::INIStructure& ini_object) {
	if (!ini_object.has("APPLICATION")) {
		return;
	}

	if (ini_object.get("APPLICATION").has("ApplicationFont")) {
		std::string get = ini_object.get("APPLICATION").get("ApplicationFont");
		if (!get.empty()) {
			strncpy_s(pdata.application_font_path, sizeof(pdata.application_font_path)/sizeof(*pdata.application_font_path), get.c_str(), get.size());
		}
	}

	if (ini_object.get("APPLICATION").has("ApplicationFontSize")) {
		std::string get = ini_object.get("APPLICATION").get("ApplicationFontSize");
		if (!get.empty()) {
			try {
				float val = std::stof(get);
				strncpy_s(pdata.application_font_size, sizeof(pdata.application_font_size)/sizeof(*pdata.application_font_size), get.c_str(), get.size());
				pdata.evaluated_font_size = val;
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [APPLICATION].ApplicationFontSize: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("APPLICATION").has("UseExtraCodecs")) {
		std::string get = ini_object.get("APPLICATION").get("UseExtraCodecs");
		if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
			pdata.useExtraCodecs = true;
		}
	}
}

} // namespace ARVT
