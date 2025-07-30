#pragma once
#include <string>

struct ProgramData {
	char the_file_input_name[1024];
	char evaluated_input_file_name[1024];
	char input_comment_data[16 * 1024];

	char evaluated_input_split_1[1024];
	char evaluated_input_split_2[1024];
	char input_split_1_data[16 * 1024];
	char input_split_2_data[16 * 1024];

	char evaluated_test_image_path[1024];
	char evaluated_output_speech_path[1024];

	char application_font_path[1024];
	char application_font_size[32];
	float evaluated_font_size;

	bool input_comment_word_wrap = true; //TODO
	bool useExtraCodecs = false; //used for VideoData/AudioData
	int default_tab_idx = 0; //only used on program startup; *not* clamped to valid range

	static const char* imageDeleteAgeList[6];
	static const int imageDeleteAgeList_values[6];
	int imageDeleteAgeList_current = 0;

	ProgramData();
};
