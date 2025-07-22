#pragma once
#include <string>
#include <cstring> //strcpy

struct ProgramData {
	char the_file_input_name[1024];
	char evaluated_input_file_name[1024];

	char input_comment_data[16 * 1024];
	bool input_comment_word_wrap = true; //TODO

	char evaluated_input_split_1[1024];
	char evaluated_input_split_2[1024];
	char input_split_1_data[16 * 1024];
	char input_split_2_data[16 * 1024];

	char evaluated_test_image_path[1024];
	char evaluated_output_speech_path[1024];

	char application_font_path[1024];
	char application_font_size[32];
	float evaluated_font_size;

	bool useExtraCodecs = false; //used for VideoData/AudioData

	const char* imageDeleteAgeList[6] = { "0 seconds", "1 hour", "24 hours", "2 weeks", "1 month", "6 months" };
	const int imageDeleteAgeList_values[6] = { 0, 1, 24, 14*24, 30*24, 180*24 };
	int imageDeleteAgeList_current = 0;

	ProgramData() {
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
};
