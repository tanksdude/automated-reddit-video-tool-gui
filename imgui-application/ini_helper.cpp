#include "ini_helper.h"
#include <cstring> //strcpy, memcpy
#include <filesystem>
#include <fstream>
#include <iostream>

namespace ARVT {

void CreateDefaultIniIfNeeded(const std::string& path) {
	if (std::filesystem::exists(path)) {
		return;
	}

	static const std::string ini_string =

	"[APPLICATION]\n"
	"\n"
	"ApplicationFont =\n"
	"ApplicationFontSize =\n"
	"ApplicationWindowWidth = 1600\n"
	"ApplicationWindowHeight = 900\n"
	"ApplicationScaleWithMonitorScale = true\n"
	"\n"
	"; Accepts hex codes and RGB floats:\n"
	"ApplicationBackgroundColor = (0.45, 0.55, 0.60)\n"
	";ApplicationWindowColor = (0.06, 0.06, 0.06, 0.94)\n"
	"\n"
	"UseExtraCodecs = false\n"
	"InitialOpenTab = 0\n"
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
	"FontName = Verdana\n"
	"FontIsFamily = false\n"
	"\n"
	"ImageFormat = .png\n"
	"\n"

	"[AUDIO]\n"
	"\n"
	"VoiceEngine =\n"
	"VoiceName =\n"
	"\n"
	"AudioEncoder = AAC\n"
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
	"VideoFaststartIfPossible = false\n"
	"VideoContainer = .mp4\n"
	"AudioOnly = false\n"
	;

	std::ofstream ini_file;
	ini_file.open(path);
	if (ini_file.is_open()) {
		ini_file << ini_string;
		ini_file.close();
	} else {
		//TODO: could not create
	}
}

// I wanted to use strncpy_s for this function as that's the "correct" (and
// "safe") way to copy the user input to the char buffers. However, strncpy_s
// is a C function that does not exist in C++. (I didn't know that was a
// thing...) Anyway, a careful memcpy accomplishes the same thing.
// Note: "length" here is the number of characters before the null terminator.
static void copyUserStringToCharBuffer(char* dest, size_t dest_size, const char* src, size_t src_length) {
	if (dest_size - 1 <= src_length) {
		strcpy(dest, src);
	} else {
		memcpy(dest, src, dest_size - 1);
		dest[dest_size - 1] = '\0';
	}
}

void Fill_ImageData(ImageData& idata, const mINI::INIStructure& ini_object) {
	if (!ini_object.has("IMAGE")) {
		return;
	}

	if (ini_object.get("IMAGE").has("ImageWidth")) {
		std::string get = ini_object.get("IMAGE").get("ImageWidth");
		if (!get.empty()) {
			copyUserStringToCharBuffer(idata.image_width_input, sizeof(idata.image_width_input)/sizeof(*idata.image_width_input), get.c_str(), get.size());
		}
	}
	if (ini_object.get("IMAGE").has("ImageHeight")) {
		std::string get = ini_object.get("IMAGE").get("ImageHeight");
		if (!get.empty()) {
			copyUserStringToCharBuffer(idata.image_height_input, sizeof(idata.image_height_input)/sizeof(*idata.image_height_input), get.c_str(), get.size());
		}
	}

	if (ini_object.get("IMAGE").has("ImageBorderW")) {
		std::string get = ini_object.get("IMAGE").get("ImageBorderW");
		if (!get.empty()) {
			copyUserStringToCharBuffer(idata.image_w_border_input, sizeof(idata.image_w_border_input)/sizeof(*idata.image_w_border_input), get.c_str(), get.size());
		}
	}
	if (ini_object.get("IMAGE").has("ImageBorderH")) {
		std::string get = ini_object.get("IMAGE").get("ImageBorderH");
		if (!get.empty()) {
			copyUserStringToCharBuffer(idata.image_h_border_input, sizeof(idata.image_h_border_input)/sizeof(*idata.image_h_border_input), get.c_str(), get.size());
		}
	}

	if (ini_object.get("IMAGE").has("FontSize")) {
		std::string get = ini_object.get("IMAGE").get("FontSize");
		if (!get.empty()) {
			copyUserStringToCharBuffer(idata.font_size_input, sizeof(idata.font_size_input)/sizeof(*idata.font_size_input), get.c_str(), get.size());
		}
	}

	if (ini_object.get("IMAGE").has("FontColor")) {
		std::string get = ini_object.get("IMAGE").get("FontColor");
		if (!get.empty()) {
			copyUserStringToCharBuffer(idata.font_color_input, sizeof(idata.font_color_input)/sizeof(*idata.font_color_input), get.c_str(), get.size());
		}
	}

	if (ini_object.get("IMAGE").has("BackgroundColor")) {
		std::string get = ini_object.get("IMAGE").get("BackgroundColor");
		if (!get.empty()) {
			copyUserStringToCharBuffer(idata.background_color_input, sizeof(idata.background_color_input)/sizeof(*idata.background_color_input), get.c_str(), get.size());
		}
	}

	if (ini_object.get("IMAGE").has("ParagraphNewlineCount")) {
		std::string get = ini_object.get("IMAGE").get("ParagraphNewlineCount");
		if (!get.empty()) {
			try {
				uint8_t val = std::stoi(get);
				idata.paragraph_newline_v = val;
				//don't bother clamping
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [IMAGE].ParagraphNewlineCount: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("IMAGE").has("ParagraphTabbedStart")) {
		std::string get = ini_object.get("IMAGE").get("ParagraphTabbedStart");
		if (!get.empty()) {
			if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
				idata.paragraph_tabbed_start_input = true;
			} else if (get == "false" || get == "False" || get == "FALSE" || get == "0") {
				idata.paragraph_tabbed_start_input = false;
			} else {
				std::cerr << ("Unknown value for [IMAGE].ParagraphTabbedStart: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("IMAGE").has("FontName")) {
		std::string get = ini_object.get("IMAGE").get("FontName");
		if (!get.empty()) {
			copyUserStringToCharBuffer(idata.font_name, sizeof(idata.font_name)/sizeof(*idata.font_name), get.c_str(), get.size());
		}
	}

	if (ini_object.get("IMAGE").has("FontIsFamily")) {
		std::string get = ini_object.get("IMAGE").get("FontIsFamily");
		if (!get.empty()) {
			if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
				idata.font_is_family = true;
			} else if (get == "false" || get == "False" || get == "FALSE" || get == "0") {
				idata.font_is_family = false;
			} else {
				std::cerr << ("Unknown value for [IMAGE].FontIsFamily: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("IMAGE").has("ImageFormat")) {
		std::string get = ini_object.get("IMAGE").get("ImageFormat");
		if (!get.empty()) {
			size_t index = std::distance(idata.imageFormatArray, std::find(idata.imageFormatArray, idata.imageFormatArray + sizeof(idata.imageFormatArray) / sizeof(*idata.imageFormatArray), get));
			if (index != sizeof(idata.imageFormatArray) / sizeof(*idata.imageFormatArray)) {
				idata.imageFormatArray_current = index;
			} else {
				std::cerr << ("Unknown value for [IMAGE].ImageFormat: \"" + get + "\"") << std::endl;
			}
		}
	}
}

void Fill_AudioData(AudioData& adata, const mINI::INIStructure& ini_object, bool useExtraCodecs) {
	if (!ini_object.has("AUDIO")) {
		return;
	}

	//TODO
	if (ini_object.get("AUDIO").has("VoiceEngine")) {
		std::string get = ini_object.get("AUDIO").get("VoiceEngine");
		if (!get.empty()) {
			size_t index = std::distance(adata.voiceEngineArray, std::find(adata.voiceEngineArray, adata.voiceEngineArray + sizeof(adata.voiceEngineArray) / sizeof(*adata.voiceEngineArray), get));
			if (index != sizeof(adata.voiceEngineArray) / sizeof(*adata.voiceEngineArray)) {
				adata.voiceEngineArray_current = index;
			} else {
				std::cerr << ("Unknown value for [AUDIO].VoiceEngine: \"" + get + "\"") << std::endl;
			}
		}
	}

	//TODO
	if (ini_object.get("AUDIO").has("VoiceName")) {
		std::string get = ini_object.get("AUDIO").get("VoiceName");
		if (!get.empty()) {
			size_t index = std::distance(adata.voiceArray, std::find(adata.voiceArray, adata.voiceArray + adata.voiceArray_length, get));
			if (index != adata.voiceArray_length) {
				adata.voiceArray_current = index;
			} else {
				std::cerr << ("Unknown value for [AUDIO].VoiceName: \"" + get + "\"") << std::endl;
			}
		}
	}

	//TODO
	if (ini_object.get("AUDIO").has("AudioEncoder")) {
		std::string get = ini_object.get("AUDIO").get("AudioEncoder");
		if (!get.empty()) {
			if (useExtraCodecs) {
				size_t index = std::distance(adata.audioEncoderArrayExtended, std::find(adata.audioEncoderArrayExtended, adata.audioEncoderArrayExtended + sizeof(adata.audioEncoderArrayExtended) / sizeof(*adata.audioEncoderArrayExtended), get));
				if (index != sizeof(adata.audioEncoderArrayExtended) / sizeof(*adata.audioEncoderArrayExtended)) {
					adata.audioEncoderArray_current = index;
				} else {
					std::cerr << ("Unknown value for [AUDIO].AudioEncoder: \"" + get + "\"") << std::endl;
				}
			} else {
				size_t index = std::distance(adata.audioEncoderArray, std::find(adata.audioEncoderArray, adata.audioEncoderArray + sizeof(adata.audioEncoderArray) / sizeof(*adata.audioEncoderArray), get));
				if (index != sizeof(adata.audioEncoderArray) / sizeof(*adata.audioEncoderArray)) {
					adata.audioEncoderArray_current = index;
				} else {
					std::cerr << ("Unknown value for [AUDIO].AudioEncoder: \"" + get + "\"") << std::endl;
				}
			}
		}
	}

	//TODO
	if (ini_object.get("AUDIO").has("AudioBitrateKbps") && !adata.get_audioEncoderIsLossless()) {
		std::string get = ini_object.get("AUDIO").get("AudioBitrateKbps");
		if (!get.empty()) {
			try {
				int16_t val = std::stoi(get);
				adata.audio_bitrate_v = val;
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [AUDIO].AudioBitrateKbps: \"" + get + "\"") << std::endl;
			}
		}
	}

	//TODO
	if (ini_object.get("AUDIO").has("AudioPreset") && adata.audioCodec_hasPreset) {
		std::string get = ini_object.get("AUDIO").get("AudioPreset");
		if (!get.empty() && get != "default") {
			size_t index = std::distance(adata.get_audioPresetArray(), std::find(adata.get_audioPresetArray(), adata.get_audioPresetArray() + adata.get_audioPresetArray_size(), get));
			if (index != adata.get_audioPresetArray_size()) {
				adata.audioPresetArray_current = index;
			} else {
				std::cerr << ("Unknown value for [AUDIO].AudioPreset: \"" + get + "\"") << std::endl;
			}
		}
	}
}

void Fill_VideoData(VideoData& vdata, const mINI::INIStructure& ini_object, bool useExtraCodecs) {
	if (!ini_object.has("VIDEO")) {
		return;
	}

	//TODO
	if (ini_object.get("VIDEO").has("VideoEncoder")) {
		std::string get = ini_object.get("VIDEO").get("VideoEncoder");
		if (!get.empty()) {
			if (useExtraCodecs) {
				size_t index = std::distance(vdata.videoEncoderArrayExtended, std::find(vdata.videoEncoderArrayExtended, vdata.videoEncoderArrayExtended + sizeof(vdata.videoEncoderArrayExtended) / sizeof(*vdata.videoEncoderArrayExtended), get));
				if (index != sizeof(vdata.videoEncoderArrayExtended) / sizeof(*vdata.videoEncoderArrayExtended)) {
					vdata.videoEncoderArray_current = index;
				} else {
					std::cerr << ("Unknown value for [VIDEO].VideoEncoder: \"" + get + "\"") << std::endl;
				}
			} else {
				size_t index = std::distance(vdata.videoEncoderArray, std::find(vdata.videoEncoderArray, vdata.videoEncoderArray + sizeof(vdata.videoEncoderArray) / sizeof(*vdata.videoEncoderArray), get));
				if (index != sizeof(vdata.videoEncoderArray) / sizeof(*vdata.videoEncoderArray)) {
					vdata.videoEncoderArray_current = index;
				} else {
					std::cerr << ("Unknown value for [VIDEO].VideoEncoder: \"" + get + "\"") << std::endl;
				}
			}
		}
	}

	//TODO
	if (ini_object.get("VIDEO").has("VideoPreset1") && vdata.videoCodec_hasPreset1) {
		std::string get = ini_object.get("VIDEO").get("VideoPreset1");
		if (!get.empty() && get != "default") {
			size_t index = std::distance(vdata.get_videoPresetArray1(), std::find(vdata.get_videoPresetArray1(), vdata.get_videoPresetArray1() + vdata.get_videoPresetArray1_size(), get));
			if (index != vdata.get_videoPresetArray1_size()) {
				vdata.videoPresetArray1_current = index;
			} else {
				std::cerr << ("Unknown value for [VIDEO].VideoPreset1: \"" + get + "\"") << std::endl;
			}
		}
	}

	//TODO
	if (ini_object.get("VIDEO").has("VideoPreset2") && vdata.videoCodec_hasPreset2) {
		std::string get = ini_object.get("VIDEO").get("VideoPreset2");
		if (!get.empty() && get != "default") {
			size_t index = std::distance(vdata.get_videoPresetArray2(), std::find(vdata.get_videoPresetArray2(), vdata.get_videoPresetArray2() + vdata.get_videoPresetArray2_size(), get));
			if (index != vdata.get_videoPresetArray2_size()) {
				vdata.videoPresetArray2_current = index;
			} else {
				std::cerr << ("Unknown value for [VIDEO].VideoPreset2: \"" + get + "\"") << std::endl;
			}
		}
	}

	// This is the most complicated case: fractional if there's a '/', but also
	// fractional if the integer value is not a "common" value. Not bothering
	// to convert "29.97" to "2997 / 100". Variable framerates are not
	// supported by this program.
	if (ini_object.get("VIDEO").has("VideoFPS")) {
		std::string get = ini_object.get("VIDEO").get("VideoFPS");
		size_t pos = get.find("/");

		if (pos != std::string::npos) {
			try {
				long long val_num = std::stoll(get.substr(0, pos));
				long long val_den = std::stoll(get.substr(pos+1));
				std::string numerator   = std::to_string(val_num);
				std::string denominator = std::to_string(val_den);
				copyUserStringToCharBuffer(vdata.fps_numerator_input,   sizeof(vdata.fps_numerator_input)/sizeof(*vdata.fps_numerator_input),     numerator.c_str(),   numerator.size());
				copyUserStringToCharBuffer(vdata.fps_denominator_input, sizeof(vdata.fps_denominator_input)/sizeof(*vdata.fps_denominator_input), denominator.c_str(), denominator.size());
				vdata.fractionalFps = true;
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [VIDEO].VideoFPS: \"" + get + "\"") << std::endl;
			}
		} else if (!get.empty()) {
			size_t index = std::distance(vdata.fpsArray, std::find(vdata.fpsArray, vdata.fpsArray + sizeof(vdata.fpsArray) / sizeof(*vdata.fpsArray), get));
			if (index != sizeof(vdata.fpsArray) / sizeof(*vdata.fpsArray)) {
				vdata.fpsArray_current = index;
			} else {
				try {
					long long val = std::stoll(get);
					std::string integer = std::to_string(val);
					copyUserStringToCharBuffer(vdata.fps_numerator_input, sizeof(vdata.fps_numerator_input)/sizeof(*vdata.fps_numerator_input), integer.c_str(), integer.size());
					strcpy(vdata.fps_denominator_input, "1");
					vdata.fractionalFps = true;
				}
				catch (const std::exception&) {
					std::cerr << ("Unable to parse [VIDEO].VideoFPS: \"" + get + "\"") << std::endl;
				}
			}
		}
	}

	//TODO
	if (ini_object.get("VIDEO").has("VideoCRF") && !vdata.get_videoEncoderIsLossless()) {
		std::string get = ini_object.get("VIDEO").get("VideoCRF");
		if (!get.empty()) {
			try {
				int8_t val = std::stoi(get);
				vdata.crf_v = val;
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [VIDEO].VideoCRF: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("VIDEO").has("VideoFaststartIfPossible")) {
		std::string get = ini_object.get("VIDEO").get("VideoFaststartIfPossible");
		if (!get.empty()) {
			if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
				vdata.faststart_flag = true;
			} else if (get == "false" || get == "False" || get == "FALSE" || get == "0") {
				vdata.faststart_flag = false;
			} else {
				std::cerr << ("Unknown value for [VIDEO].VideoFaststartIfPossible: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("VIDEO").has("VideoContainer")) {
		std::string get = ini_object.get("VIDEO").get("VideoContainer");
		if (!get.empty()) {
			size_t index = std::distance(vdata.videoContainerArray, std::find(vdata.videoContainerArray, vdata.videoContainerArray + sizeof(vdata.videoContainerArray) / sizeof(*vdata.videoContainerArray), get));
			if (index != sizeof(vdata.videoContainerArray) / sizeof(*vdata.videoContainerArray)) {
				vdata.videoContainerArray_current = index;
			} else {
				std::cerr << ("Unknown value for [VIDEO].VideoContainer: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("VIDEO").has("AudioOnly")) {
		std::string get = ini_object.get("VIDEO").get("AudioOnly");
		if (!get.empty()) {
			if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
				vdata.audio_only_option_input = true;
			} else if (get == "false" || get == "False" || get == "FALSE" || get == "0") {
				vdata.audio_only_option_input = false;
			} else {
				std::cerr << ("Unknown value for [VIDEO].AudioOnly: \"" + get + "\"") << std::endl;
			}
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
			copyUserStringToCharBuffer(pdata.application_font_path, sizeof(pdata.application_font_path)/sizeof(*pdata.application_font_path), get.c_str(), get.size());
		}
	}

	if (ini_object.get("APPLICATION").has("ApplicationFontSize")) {
		std::string get = ini_object.get("APPLICATION").get("ApplicationFontSize");
		if (!get.empty()) {
			try {
				float val = std::stof(get);
				pdata.application_font_size = val;
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [APPLICATION].ApplicationFontSize: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("APPLICATION").has("ApplicationWindowWidth")) {
		std::string get = ini_object.get("APPLICATION").get("ApplicationWindowWidth");
		if (!get.empty()) {
			try {
				int val = std::stoi(get);
				pdata.initial_windowWidth = val;
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [APPLICATION].ApplicationWindowWidth: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("APPLICATION").has("ApplicationWindowHeight")) {
		std::string get = ini_object.get("APPLICATION").get("ApplicationWindowHeight");
		if (!get.empty()) {
			try {
				int val = std::stoi(get);
				pdata.initial_windowHeight = val;
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [APPLICATION].ApplicationWindowHeight: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("APPLICATION").has("ApplicationScaleWithMonitorScale")) {
		std::string get = ini_object.get("APPLICATION").get("ApplicationScaleWithMonitorScale");
		if (!get.empty()) {
			if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
				pdata.application_scale_to_monitor = true;
			} else if (get == "false" || get == "False" || get == "FALSE" || get == "0") {
				pdata.application_scale_to_monitor = false;
			} else {
				std::cerr << ("Unknown value for [APPLICATION].ApplicationScaleWithMonitorScale: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("APPLICATION").has("UseExtraCodecs")) {
		std::string get = ini_object.get("APPLICATION").get("UseExtraCodecs");
		if (!get.empty()) {
			if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
				pdata.useExtraCodecs = true;
			} else if (get == "false" || get == "False" || get == "FALSE" || get == "0") {
				pdata.useExtraCodecs = false;
			} else {
				std::cerr << ("Unknown value for [APPLICATION].UseExtraCodecs: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("APPLICATION").has("InitialOpenTab")) {
		std::string get = ini_object.get("APPLICATION").get("InitialOpenTab");
		if (!get.empty()) {
			try {
				int val = std::stoi(get);
				pdata.default_tab_idx = val;
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [APPLICATION].InitialOpenTab: \"" + get + "\"") << std::endl;
			}
		}
	}

	// This is actually the most complicated case, even beating video FPS. If
	// the first character is '#', it's a hex code, and fail on anything not 6
	// digits. If the first character is '(', it's a vec3. Other cases not
	// supported... though there aren't very many.
	//TODO: this is in a "good enough" state, but should finish it eventually
	//TODO: # is an official comment character, so something is gonna have to be done to handle that situation...
	if (ini_object.get("APPLICATION").has("ApplicationBackgroundColor")) {
		std::string get = ini_object.get("APPLICATION").get("ApplicationBackgroundColor");
		if (!get.empty()) {
			if (get[0] == '#') {
				if (get.size() == 7) {
					try {
						int r = std::stoi(get.substr(1, 2), nullptr, 16);
						int g = std::stoi(get.substr(3, 2), nullptr, 16);
						int b = std::stoi(get.substr(5, 2), nullptr, 16);
						if (r < 0 || g < 0 || b < 0) {
							std::cerr << ("Unable to parse [APPLICATION].ApplicationBackgroundColor: \"" + get + "\"") << std::endl;
						} else {
							pdata.background_color = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
						}
					}
					catch (const std::exception&) {
						std::cerr << ("Unable to parse [APPLICATION].ApplicationBackgroundColor: \"" + get + "\"") << std::endl;
					}
				} else {
					std::cerr << ("Unable to parse [APPLICATION].ApplicationBackgroundColor: \"" + get + "\"") << std::endl;
				}
			} else if (get[0] == '(') {
				if ((get[get.size()-1] == ')') && (std::count(get.begin(), get.end(), ',') == 2)) {
					size_t pos1 = get.find(',');
					size_t pos2 = get.find(',', pos1 + 1);
					try {
						float r = std::stof(get.substr(1,        pos1             - 1));
						float g = std::stof(get.substr(pos1 + 1, pos2             - (pos1 + 1)));
						float b = std::stof(get.substr(pos2 + 1, (get.size() - 1) - (pos2 + 1)));
						if ((r < 0 || g < 0 || b < 0) || (r > 1 || g > 1 || b > 1)) {
							std::cerr << ("Unable to parse [APPLICATION].ApplicationBackgroundColor: \"" + get + "\"") << std::endl;
						} else {
							pdata.background_color = ImVec4(r, g, b, 1.0f);
						}
					}
					catch (const std::exception&) {
						std::cerr << ("Unable to parse [APPLICATION].ApplicationBackgroundColor: \"" + get + "\"") << std::endl;
					}
				} else {
					std::cerr << ("Unable to parse [APPLICATION].ApplicationBackgroundColor: \"" + get + "\"") << std::endl;
				}
			} else {
				std::cerr << ("Unable to parse [APPLICATION].ApplicationBackgroundColor: \"" + get + "\"") << std::endl;
			}
		}
	}

	//this has alpha
	if (ini_object.get("APPLICATION").has("ApplicationWindowColor")) {
		std::string get = ini_object.get("APPLICATION").get("ApplicationWindowColor");
		if (!get.empty()) {
			if (get[0] == '#') {
				if (get.size() == 7 || get.size() == 9) {
					try {
						int r = std::stoi(get.substr(1, 2), nullptr, 16);
						int g = std::stoi(get.substr(3, 2), nullptr, 16);
						int b = std::stoi(get.substr(5, 2), nullptr, 16);
						int a = get.size() == 9 ? std::stoi(get.substr(7, 2), nullptr, 16) : 0xFF;
						if (r < 0 || g < 0 || b < 0 || a < 0) {
							std::cerr << ("Unable to parse [APPLICATION].ApplicationWindowColor: \"" + get + "\"") << std::endl;
						} else {
							pdata.window_color = ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
						}
					}
					catch (const std::exception&) {
						std::cerr << ("Unable to parse [APPLICATION].ApplicationWindowColor: \"" + get + "\"") << std::endl;
					}
				} else {
					std::cerr << ("Unable to parse [APPLICATION].ApplicationWindowColor: \"" + get + "\"") << std::endl;
				}
			} else if (get[0] == '(') {
				if ((get[get.size()-1] == ')') && (std::count(get.begin(), get.end(), ',') == 3)) {
					size_t pos1 = get.find(',');
					size_t pos2 = get.find(',', pos1 + 1);
					size_t pos3 = get.find(',', pos2 + 1);
					try {
						float r = std::stof(get.substr(1,        pos1             - 1));
						float g = std::stof(get.substr(pos1 + 1, pos2             - (pos1 + 1)));
						float b = std::stof(get.substr(pos2 + 1, pos3             - (pos2 + 1)));
						float a = std::stof(get.substr(pos3 + 1, (get.size() - 1) - (pos3 + 1)));
						if ((r < 0 || g < 0 || b < 0 || a < 0) || (r > 1 || g > 1 || b > 1 || a > 1)) {
							std::cerr << ("Unable to parse [APPLICATION].ApplicationWindowColor: \"" + get + "\"") << std::endl;
						} else {
							pdata.window_color = ImVec4(r, g, b, a);
						}
					}
					catch (const std::exception&) {
						std::cerr << ("Unable to parse [APPLICATION].ApplicationWindowColor: \"" + get + "\"") << std::endl;
					}
				} else {
					std::cerr << ("Unable to parse [APPLICATION].ApplicationWindowColor: \"" + get + "\"") << std::endl;
				}
			} else {
				std::cerr << ("Unable to parse [APPLICATION].ApplicationWindowColor: \"" + get + "\"") << std::endl;
			}
		}
	}
}

} // namespace ARVT
