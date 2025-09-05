#include "image_data.h"
#include <cstring> //strcpy

const char* ImageData::textAlignmentArray[5] = { "default", "left", "center", "right", "justify" };
const char* ImageData::imageFormatArray[6] = { ".png", ".jpg", ".avif", ".webp", ".tiff", ".bmp" };

ImageData::ImageData()  {
	strcpy(image_width_input, "960");
	strcpy(image_height_input, "640");
	strcpy(image_w_border_input, "32");
	strcpy(image_h_border_input, "32");

	strcpy(font_size_input, "16");
	strcpy(font_color_input, "white");
	strcpy(background_color_input, "black");

	font_is_family_input = false;
	strcpy(font_name, "Verdana");
}
