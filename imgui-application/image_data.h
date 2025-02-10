#pragma once
#include <string>
#include <cstring> //strcpy

struct ImageData {
	char image_width_input[32];
	char image_height_input[32];
	char image_w_border_input[32];
	char image_h_border_input[32];

	char font_size_input[32];
	char font_color_input[32];
	char background_color_input[32];
	char paragraph_separator_input[32];

	const char* imageFormatArray[6] = { ".png", ".jpg", ".avif", ".webp", ".tiff", ".bmp" };
	int imageFormatArray_current = 0;

	inline std::string get_image_width_input() const { return std::string(image_width_input); }
	inline std::string get_image_height_input() const { return std::string(image_height_input); }
	inline std::string get_image_w_border_input() const { return std::string(image_w_border_input); }
	inline std::string get_image_h_border_input() const { return std::string(image_h_border_input); }

	inline std::string get_font_size_input() const { return std::string(font_size_input); }
	inline std::string get_font_color_input() const { return std::string(font_color_input); }
	inline std::string get_background_color_input() const { return std::string(background_color_input); }
	std::string get_paragraph_separator() const; //escapes quotes and backslashes (TODO: Linux probably handles things differently, might have to make this a "newline count" instead of anything)
	inline std::string get_imageFormat() const { return std::string(imageFormatArray[imageFormatArray_current]); }

	ImageData() {
		strcpy(image_width_input, "960");
		strcpy(image_height_input, "640");
		strcpy(image_w_border_input, "32");
		strcpy(image_h_border_input, "32");

		strcpy(font_size_input, "16");
		strcpy(font_color_input, "white");
		strcpy(background_color_input, "black");
		strcpy(paragraph_separator_input, "\\n\\n");
	}
};
