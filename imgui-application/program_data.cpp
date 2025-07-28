#include "program_data.h"
#include <cstring> //strcpy

const char* ProgramData::imageDeleteAgeList[6] = { "0 seconds", "1 hour", "24 hours", "2 weeks", "1 month", "6 months" };
const int ProgramData::imageDeleteAgeList_values[6] = { 0, 1, 24, 14*24, 30*24, 180*24 };

ProgramData::ProgramData() {
	strcpy(the_file_input_name, "lorem_ipsum");
	strcpy(input_comment_data, "");

	strcpy(input_split_1_data, "");
	strcpy(input_split_2_data, "");

	// Other char[] don't need to be set because they are not directly
	// accessed; they are always written to (using the_file_input_name)
	// before being used or sent to ImGui.

	#ifdef _WIN32
	strcpy(application_font_path, "C:\\Windows\\Fonts\\segoeui.ttf");
	strcpy(application_font_size, "24.0");
	evaluated_font_size = 24.0f;
	#else
	strcpy(application_font_path, "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf");
	strcpy(application_font_size, "20.0");
	evaluated_font_size = 20.0f;
	#endif
}
