#include "program_data.h"
#include <cstring> //strcpy

#ifdef _WIN32
const std::array<const char*, 3> ProgramData::pythonCmdArray = { "python", "python3", "py -3" };
//explorer.exe is enough
#else
const std::array<const char*, 3> ProgramData::pythonCmdArray = { "python", "python3", "py" }; //"py" is only used by people who explicitly made a symlink/alias
//"auto" queries the system, xdg-open opens on the folder (feels like a hack), then the rest are the default file managers: GNOME, KDE Plasma, Xfce, Cinnamon, MATE
const std::array<const char*, 7> ProgramData::fileExplorerCmdArray           = { "auto", "xdg-open (folder only)", "Nautilus (GNOME Files)", "Dolphin",          "Thunar", "Nemo", "Caja" };
const std::array<const char*, 7> ProgramData::fileExplorerCmdArray_iniValues = { "auto", "xdg-open",               "Nautilus",               "Dolphin",          "Thunar", "Nemo", "Caja" };
const std::array<const char*, 7> ProgramData::fileExplorerCmdArray_exe       = { "echo", "xdg-open",               "nautilus",               "dolphin --select", "thunar", "nemo", "caja --select" };
//others: LXDE/LXQt's PCManFM(-Qt) behaves like xdg-open

#include "arvt_helpers.h"
#include <algorithm> //std::find
int ProgramData::findIdxOfAutoFileExplorerCmd() {
	// 1. Run process
	std::vector<std::string> lines;
	int result = ARVT::readPipeIntoString("xdg-mime query default inode/directory", lines);
	if (result || lines.empty()) {
		return -1;
	}

	// 2. Extract the file manager's name
	// The output always seems to be "A.B.C.D", where:
	// * C is the file manager
	// * D is "desktop"
	// * A.B sometimes exists (GNOME and KDE Plasma only: "org.gnome" or "org.kde")
	std::string fileManager = lines[0];
	size_t end = fileManager.find_last_of(".");
	if (end != std::string::npos) [[likely]] {
		fileManager.erase(end);
		size_t start = fileManager.find_last_of(".");
		if (start != std::string::npos) {
			fileManager = fileManager.substr(start+1);
		}
	}

	// 3. Standardize the string
	// Nautilus is capitalized, everything else isn't, but robustness says to handle the possibility
	if (fileManager == "Nautilus" || fileManager == "nautilus" || fileManager == "NAUTILUS") {
		fileManager = "Nautilus";
	} else if (fileManager == "dolphin" || fileManager == "Dolphin" || fileManager == "DOLPHIN") {
		fileManager = "Dolphin";
	} else if (fileManager == "nemo" || fileManager == "Nemo" || fileManager == "NEMO") {
		fileManager = "Nemo";
	} else if (fileManager == "thunar" || fileManager == "Thunar" || fileManager == "THUNAR") {
		fileManager = "Thunar";
	} else if (fileManager == "caja-folder-handler") { // Caja doesn't follow the others
		fileManager = "Caja";
	} else {
		// unsupported/unknown
		fileManager = "";
	}

	// 4. Translate the string to the index
	//copied from ini_helper.cpp
	size_t index = std::distance(ProgramData::fileExplorerCmdArray_iniValues.begin(), std::find(ProgramData::fileExplorerCmdArray_iniValues.begin(), ProgramData::fileExplorerCmdArray_iniValues.end(), fileManager));
	if (index != ProgramData::fileExplorerCmdArray_iniValues.size()) {
		return index;
	} else {
		return -1;
	}
}
#endif

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

	strcpy(application_font_path, "../res/NotoSans-Regular.otf");
	application_font_size = 24.0f;
}
