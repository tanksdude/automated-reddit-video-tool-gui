#pragma once
#include <string>
#include <array>
#include <imgui.h>

struct ProgramData {
#ifdef _WIN32
	static const std::array<const char*, 3> pythonCmdArray;
	int pythonCmdArray_current = 0;
	bool spawn_debug_console = false;
#else
	static const std::array<const char*, 3> pythonCmdArray;
	int pythonCmdArray_current = 1;
	static const std::array<const char*, 7> fileExplorerCmdArray;
	static const std::array<const char*, 7> fileExplorerCmdArray_iniValues;
	static const std::array<const char*, 7> fileExplorerCmdArray_exe;
	int fileExplorerCmdArray_current = 0;
	static int findIdxOfAutoFileExplorerCmd(); // Returns -1 if unknown/unsupported
#endif
	inline std::string get_pythonCmd() const { return std::string(pythonCmdArray[pythonCmdArray_current]); }

	char the_file_input_name[1024];
	char evaluated_input_file_name[1024];
	char input_comment_data[16 * 1024];

	char evaluated_input_split_1[1024];
	char evaluated_input_split_2[1024];
	char input_split_1_data[16 * 1024];
	char input_split_2_data[16 * 1024];

	char evaluated_test_image_path[1024];
	char evaluated_output_speech_path[1024];
	char evaluated_video_settings_path[1024];

	char input_comments_path[1024];
	char input_splits_path[1024];
	char test_images_path[1024];
	char output_speech_path[1024];
	char video_settings_path[1024];
	char temporary_file_path[1024];
	void ResetFilePaths(); // (not including the font)

	std::string get_input_comments_path() const;
	std::string get_input_splits_path() const;
	std::string get_test_images_path() const;
	std::string get_output_speech_path() const;
	std::string get_video_settings_path() const;
	std::string get_temporary_file_path() const;
	static bool valid_filepath_for_scripts(const char* path); // Returns true if empty or last character is '/'
	static void clean_filepath(char* path); // Used by the INI file when loading paths

	char application_font_path[1024];
	float application_font_size;
	bool application_scale_to_monitor;

	bool useCustomPaths = false;
	//bool input_comment_word_wrap = true; //TODO: word wrap doesn't affect TextMultiline for some reason
	bool useExtraCodecs = false; //used for VideoData/AudioData
	int startup_tab_idx = 0; //*not* clamped to valid range

	int initial_windowWidth;
	int initial_windowHeight;
	ImVec4 background_color;
	ImVec4 window_color;

	static const std::array<const char*, 6> fileDeleteAgeList;
	static const std::array<int, 6> fileDeleteAgeList_values;
	int fileDeleteAgeList_current = 0;

	ProgramData();
};
