#include "program_data.h"
#include <cstring> //strcpy

const std::array<const char*, 6> ProgramData::fileDeleteAgeList = { "0 seconds", "1 hour", "24 hours", "2 weeks", "1 month", "6 months" };
const std::array<int, 6> ProgramData::fileDeleteAgeList_values = { 0, 1, 24, 14*24, 30*24, 180*24 };

ProgramData::ProgramData() {
	application_scale_to_monitor = true;
	background_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	window_color = ImVec4(0.06f, 0.06f, 0.06f, 0.94f); // ImGui::GetStyle().Colors[ImGuiCol_WindowBg]
	initial_windowWidth = 1600;
	initial_windowHeight = 900;

	strcpy(the_file_input_name, "lorem_ipsum");
	strcpy(input_comment_data, "");

	strcpy(input_split_1_data, "");
	strcpy(input_split_2_data, "");

	// Other char[] don't need to be set because they are not directly
	// accessed; they are always written to (using the_file_input_name)
	// before being used or sent to ImGui.

	#ifdef _WIN32
	strcpy(application_font_path, "C:\\Windows\\Fonts\\segoeui.ttf");
	application_font_size = 24.0f;
	#else
	//just assume Ubuntu
	strcpy(application_font_path, "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc");
	application_font_size = 24.0f;
	#endif
}
