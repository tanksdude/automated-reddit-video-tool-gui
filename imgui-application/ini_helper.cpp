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
	";ApplicationFont = C:\\Windows\\Fonts\\NotoSans-Regular.ttf\n"
	";ApplicationFont = /usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc\n"
	";ApplicationFont = /usr/share/fonts/google-noto/NotoSans-Regular.ttf\n"
	";ApplicationFont = /usr/share/fonts/noto/NotoSans-Regular.ttf\n"
	";ApplicationFontSize = 24\n"
	"\n"
	"ApplicationWindowWidth = 1600\n"
	"ApplicationWindowHeight = 1000\n"
	"ApplicationScaleWithMonitorScale = true\n"
	"\n"
	"; Accepts hex codes and RGB floats:\n"
	";ApplicationBackgroundColor = (0.45, 0.55, 0.60)\n"
	";ApplicationWindowColor = (0.06, 0.06, 0.06, 0.94)\n"
	"\n"
	"UseExtraCodecs = false\n"
	"InitialOpenTab = 0\n"
	"\n"
	"; Windows:\n"
	";CmdPython = python\n"
	"; Linux:\n"
	";CmdPython = python3\n"
	";CmdFileManager = Nautilus\n"
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
	"TextAlignment = default\n"
	"SkipLoneLF = true\n"
	"ReplaceEscapeSequences = true\n"
	"\n"
	"ImageFormat = .png\n"
	"\n"

	"[AUDIO]\n"
	"\n"
	";SpeechLanguage = en\n"
	"SpeechEngine =\n"
	"VoiceName =\n"
	"\n"
	"AudioEncoder = AAC\n"
	"AudioPreset = default\n"
	"\n"
	";AudioBitrateKbps = 128\n"
	"\n"

	"[VIDEO]\n"
	"\n"
	"VideoEncoder = H.264\n"
	"VideoPreset1 = default\n"
	"VideoPreset2 = default\n"
	"\n"
	";VideoCRF = 23\n"
	"VideoFPS = 60\n"
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
void copyUserStringToCharBuffer(char* dest, size_t dest_size, const char* src, size_t src_length) {
	if (dest_size >= src_length+1) {
		memcpy(dest, src, src_length+1);
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
				idata.font_is_family_input = true;
			} else if (get == "false" || get == "False" || get == "FALSE" || get == "0") {
				idata.font_is_family_input = false;
			} else {
				std::cerr << ("Unknown value for [IMAGE].FontIsFamily: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("IMAGE").has("TextAlignment")) {
		std::string get = ini_object.get("IMAGE").get("TextAlignment");
		if (!get.empty()) {
			size_t index = std::distance(idata.textAlignmentArray.begin(), std::find(idata.textAlignmentArray.begin(), idata.textAlignmentArray.end(), get));
			if (index != idata.textAlignmentArray.size()) {
				idata.textAlignmentArray_current = index;
			} else {
				std::cerr << ("Unknown value for [IMAGE].TextAlignment: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("IMAGE").has("SkipLoneLF")) {
		std::string get = ini_object.get("IMAGE").get("SkipLoneLF");
		if (!get.empty()) {
			if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
				idata.skip_lone_lf_input = true;
			} else if (get == "false" || get == "False" || get == "FALSE" || get == "0") {
				idata.skip_lone_lf_input = false;
			} else {
				std::cerr << ("Unknown value for [IMAGE].SkipLoneLF: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("IMAGE").has("ReplaceEscapeSequences")) {
		std::string get = ini_object.get("IMAGE").get("ReplaceEscapeSequences");
		if (!get.empty()) {
			if (get == "true" || get == "True" || get == "TRUE" || get == "1") {
				idata.replace_magick_escape_sequences = true;
			} else if (get == "false" || get == "False" || get == "FALSE" || get == "0") {
				idata.replace_magick_escape_sequences = false;
			} else {
				std::cerr << ("Unknown value for [IMAGE].ReplaceEscapeSequences: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("IMAGE").has("ImageFormat")) {
		std::string get = ini_object.get("IMAGE").get("ImageFormat");
		if (!get.empty()) {
			size_t index = std::distance(idata.imageFormatArray.begin(), std::find(idata.imageFormatArray.begin(), idata.imageFormatArray.end(), get));
			if (index != idata.imageFormatArray.size()) {
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

	if (ini_object.get("AUDIO").has("SpeechLanguage")) {
		std::string get = ini_object.get("AUDIO").get("SpeechLanguage");
		//setting it to empty is valid
		copyUserStringToCharBuffer(adata.speech_language_input, sizeof(adata.speech_language_input)/sizeof(*adata.speech_language_input), get.c_str(), get.size());
	}

	if (ini_object.get("AUDIO").has("SpeechEngine")) {
		std::string get = ini_object.get("AUDIO").get("SpeechEngine");
		if (!get.empty()) {
			size_t index = std::distance(adata.speechEngineArray.begin(), std::find(adata.speechEngineArray.begin(), adata.speechEngineArray.end(), get));
			if (index != adata.speechEngineArray.size()) {
				adata.speechEngineArray_current = index;
				if (adata.update_voiceArray()) {
					//failed
				}
			} else {
				std::cerr << ("Unknown value for [AUDIO].SpeechEngine: \"" + get + "\"") << std::endl;
			}
		}
	}

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

	if (ini_object.get("AUDIO").has("AudioEncoder")) {
		std::string get = ini_object.get("AUDIO").get("AudioEncoder");
		if (!get.empty()) {
			bool found = false;
			for (int i = 0; i < AudioData::get_audioEncoderArraySize(useExtraCodecs); i++) {
				const AudioCodecData* ac = AudioData::audioEncoderArrayExtended[i];
				auto it = std::find(ac->searchNames.begin(), ac->searchNames.end(), get);
				if (it != ac->searchNames.end()) {
					adata.set_encoder_idx(i);
					found = true;
					break;
				}
			}
			if (!found) {
				std::cerr << ("Unknown value for [AUDIO].AudioEncoder: \"" + get + "\"") << std::endl;
			}
		}

		if (ini_object.get("AUDIO").has("AudioPreset") && !adata.get_audioEncoder()->preset1.internalValues.empty()) {
			std::string get = ini_object.get("AUDIO").get("AudioPreset");
			if (!get.empty() && get != "default") {
				const GenericCodecPreset& ac_p1 = adata.get_audioEncoder()->preset1;
				size_t index = std::distance(ac_p1.internalValues.begin(), std::find(ac_p1.internalValues.begin(), ac_p1.internalValues.end(), get));
				if (index != ac_p1.internalValues.size()) {
					adata.set_encoder_preset1_idx(index);
				} else {
					std::cerr << ("Unknown value for [AUDIO].AudioPreset: \"" + get + "\"") << std::endl;
				}
			}
		}
	}

	// Setting the bitrate without a codec doesn't really make sense, but oh well
	if (ini_object.get("AUDIO").has("AudioBitrateKbps") && !adata.get_audioEncoder()->isLossless) {
		std::string get = ini_object.get("AUDIO").get("AudioBitrateKbps");
		if (!get.empty()) {
			try {
				uint16_t val = std::stoull(get);
				adata.set_audioBitrate(val);
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [AUDIO].AudioBitrateKbps: \"" + get + "\"") << std::endl;
			}
		}
	}
}

void Fill_VideoData(VideoData& vdata, const mINI::INIStructure& ini_object, bool useExtraCodecs) {
	if (!ini_object.has("VIDEO")) {
		return;
	}

	if (ini_object.get("VIDEO").has("VideoEncoder")) {
		std::string get = ini_object.get("VIDEO").get("VideoEncoder");
		if (!get.empty()) {
			bool found = false;
			for (int i = 0; i < VideoData::get_videoEncoderArraySize(useExtraCodecs); i++) {
				const VideoCodecData* vc = VideoData::videoEncoderArrayExtended[i];
				auto it = std::find(vc->searchNames.begin(), vc->searchNames.end(), get);
				if (it != vc->searchNames.end()) {
					vdata.set_encoder_idx(i);
					found = true;
					break;
				}
			}
			if (!found) {
				std::cerr << ("Unknown value for [VIDEO].VideoEncoder: \"" + get + "\"") << std::endl;
			}
		}

		if (ini_object.get("VIDEO").has("VideoPreset1") && !vdata.get_videoEncoder()->preset1.internalValues.empty()) {
			std::string get = ini_object.get("VIDEO").get("VideoPreset1");
			if (!get.empty() && get != "default") {
				const GenericCodecPreset& vc_p1 = vdata.get_videoEncoder()->preset1;
				size_t index = std::distance(vc_p1.internalValues.begin(), std::find(vc_p1.internalValues.begin(), vc_p1.internalValues.end(), get));
				if (index != vc_p1.internalValues.size()) {
					vdata.set_encoder_preset1_idx(index);
				} else {
					std::cerr << ("Unknown value for [VIDEO].VideoPreset1: \"" + get + "\"") << std::endl;
				}
			}
		}

		if (ini_object.get("VIDEO").has("VideoPreset2") && !vdata.get_videoEncoder()->preset2.internalValues.empty()) {
			std::string get = ini_object.get("VIDEO").get("VideoPreset2");
			if (!get.empty() && get != "default") {
				const GenericCodecPreset& vc_p2 = vdata.get_videoEncoder()->preset2;
				size_t index = std::distance(vc_p2.internalValues.begin(), std::find(vc_p2.internalValues.begin(), vc_p2.internalValues.end(), get));
				if (index != vc_p2.internalValues.size()) {
					vdata.set_encoder_preset2_idx(index);
				} else {
					std::cerr << ("Unknown value for [VIDEO].VideoPreset2: \"" + get + "\"") << std::endl;
				}
			}
		}
	}

	// Setting the CRF without a codec doesn't really make sense, but oh well
	if (ini_object.get("VIDEO").has("VideoCRF") && !vdata.get_videoEncoder()->isLossless) {
		std::string get = ini_object.get("VIDEO").get("VideoCRF");
		if (!get.empty()) {
			try {
				uint8_t val = std::stoull(get);
				vdata.set_videoCrf(val);
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [VIDEO].VideoCRF: \"" + get + "\"") << std::endl;
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
			size_t index = std::distance(vdata.fpsArray.begin(), std::find(vdata.fpsArray.begin(), vdata.fpsArray.end(), get));
			if (index != vdata.fpsArray.size()) {
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
			size_t index = std::distance(vdata.videoContainerArray.begin(), std::find(vdata.videoContainerArray.begin(), vdata.videoContainerArray.end(), get));
			if (index != vdata.videoContainerArray.size()) {
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
				pdata.startup_tab_idx = val;
			}
			catch (const std::exception&) {
				std::cerr << ("Unable to parse [APPLICATION].InitialOpenTab: \"" + get + "\"") << std::endl;
			}
		}
	}

	if (ini_object.get("APPLICATION").has("CmdPython")) {
		std::string get = ini_object.get("APPLICATION").get("CmdPython");
		if (!get.empty()) {
			size_t index = std::distance(pdata.pythonCmdArray.begin(), std::find(pdata.pythonCmdArray.begin(), pdata.pythonCmdArray.end(), get));
			if (index != pdata.pythonCmdArray.size()) {
				pdata.pythonCmdArray_current = index;
			} else {
				std::cerr << ("Unknown value for [APPLICATION].CmdPython: \"" + get + "\"") << std::endl;
			}
		}
	}

	#ifdef _WIN32
	//explorer.exe is enough
	#else
	if (ini_object.get("APPLICATION").has("CmdFileManager")) {
		std::string get = ini_object.get("APPLICATION").get("CmdFileManager");
		if (!get.empty()) {
			size_t index = std::distance(pdata.fileExplorerCmdArray_iniValues.begin(), std::find(pdata.fileExplorerCmdArray_iniValues.begin(), pdata.fileExplorerCmdArray_iniValues.end(), get));
			if (index != pdata.fileExplorerCmdArray_iniValues.size()) {
				pdata.fileExplorerCmdArray_current = index;
			} else {
				std::cerr << ("Unknown value for [APPLICATION].CmdFileManager: \"" + get + "\"") << std::endl;
			}
		}
	}
	#endif
}

void CopySettingsToIni(mINI::INIStructure& ini_object, const ImageData& idata, const AudioData& adata, const VideoData& vdata) {
	ini_object["IMAGE"]["ImageWidth"]   = idata.get_image_width_input();
	ini_object["IMAGE"]["ImageHeight"]  = idata.get_image_height_input();
	ini_object["IMAGE"]["ImageBorderW"] = idata.get_image_w_border_input();
	ini_object["IMAGE"]["ImageBorderH"] = idata.get_image_h_border_input();

	ini_object["IMAGE"]["FontSize"]        = idata.get_font_size_input();
	ini_object["IMAGE"]["FontColor"]       = idata.get_font_color_input();
	ini_object["IMAGE"]["BackgroundColor"] = idata.get_background_color_input();

	ini_object["IMAGE"]["ParagraphNewlineCount"] = std::to_string(idata.paragraph_newline_v);
	ini_object["IMAGE"]["ParagraphTabbedStart"]  = std::to_string(idata.paragraph_tabbed_start_input);

	ini_object["IMAGE"]["FontName"]               = idata.font_name;
	ini_object["IMAGE"]["FontIsFamily"]           = std::to_string(idata.font_is_family_input);
	ini_object["IMAGE"]["TextAlignment"]          = idata.get_textAlignment();
	ini_object["IMAGE"]["SkipLoneLF"]             = std::to_string(idata.skip_lone_lf_input);
	ini_object["IMAGE"]["ReplaceEscapeSequences"] = std::to_string(idata.replace_magick_escape_sequences);

	ini_object["IMAGE"]["ImageFormat"] = idata.get_imageFormat();

	ini_object["AUDIO"]["SpeechLanguage"] = std::string(adata.speech_language_input);
	ini_object["AUDIO"]["SpeechEngine"]   = adata.get_speechEngine();
	ini_object["AUDIO"]["VoiceName"]      = adata.get_voiceName();

	ini_object["AUDIO"]["AudioEncoder"] = adata.get_audioEncoder()->internalName;
	ini_object["AUDIO"]["AudioPreset"]  = adata.get_audioPreset1_currentValue();

	ini_object["AUDIO"]["AudioBitrateKbps"] = std::to_string(adata.audio_bitrate_v); //no need to check if lossless

	ini_object["VIDEO"]["VideoEncoder"] = vdata.get_videoEncoder()->internalName;
	ini_object["VIDEO"]["VideoPreset1"] = vdata.get_videoPreset1_currentValue();
	ini_object["VIDEO"]["VideoPreset2"] = vdata.get_videoPreset2_currentValue();

	ini_object["VIDEO"]["VideoCRF"] = vdata.get_videoCrf(); //no need to check if lossless
	ini_object["VIDEO"]["VideoFPS"] = vdata.get_fps();

	ini_object["VIDEO"]["VideoFaststartIfPossible"] = std::to_string(vdata.faststart_flag);
	ini_object["VIDEO"]["VideoContainer"]           = vdata.get_videoContainer();
	ini_object["VIDEO"]["AudioOnly"]                = std::to_string(vdata.audio_only_option_input);
}

} // namespace ARVT
