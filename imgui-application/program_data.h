#pragma once
#include <string>
#include "imgui/imgui.h"

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
	float application_font_size;
	bool application_scale_to_monitor;

	bool input_comment_word_wrap = true; //TODO
	bool useExtraCodecs = false; //used for VideoData/AudioData
	int default_tab_idx = 0; //only used on program startup; *not* clamped to valid range

	int initial_windowWidth;
	int initial_windowHeight;
	ImVec4 background_color;
	ImVec4 window_color;

	static const char* imageDeleteAgeList[6];
	static const int imageDeleteAgeList_values[6];
	int imageDeleteAgeList_current = 0;

	ProgramData();
};
