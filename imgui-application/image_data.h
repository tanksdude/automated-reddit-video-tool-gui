#pragma once
#include <string>
#include <cstdint>
#include <array>

struct ImageData {
	char image_width_input[32];
	char image_height_input[32];
	char image_w_border_input[32];
	char image_h_border_input[32];

	char font_size_input[32];
	char font_color_input[32];
	char background_color_input[32];

	uint8_t paragraph_newline_v = 2;
	uint8_t paragraph_newline_min = 0;
	uint8_t paragraph_newline_max = 4;
	bool paragraph_tabbed_start_input = false;
	bool skip_lone_lf_input = true;
	bool replace_magick_escape_sequences = true;

	bool font_is_family_input;
	char font_name[256]; //there are some fonts with really long names

	static const std::array<const char*, 5> textAlignmentArray;
	int textAlignmentArray_current = 0;

	static const std::array<const char*, 6> imageFormatArray;
	int imageFormatArray_current = 0;
	//-quality exists, don't bother, it's kinda confusing

	inline std::string get_image_width_input() const { return std::string(image_width_input); }
	inline std::string get_image_height_input() const { return std::string(image_height_input); }
	inline std::string get_image_w_border_input() const { return std::string(image_w_border_input); }
	inline std::string get_image_h_border_input() const { return std::string(image_h_border_input); }

	inline std::string get_font_size_input() const { return std::string(font_size_input); }
	inline std::string get_font_color_input() const { return std::string(font_color_input); }
	inline std::string get_background_color_input() const { return std::string(background_color_input); }
	inline std::string get_textAlignment() const { return std::string(textAlignmentArray[textAlignmentArray_current]); }
	inline std::string get_imageFormat() const { return std::string(imageFormatArray[imageFormatArray_current]); }

	ImageData();
};
