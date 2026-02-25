#include "arvt_helpers.h"
#include <cstdlib> //system()
#include <cstring> //strncpy
#include <fstream>
#include <filesystem>
#include <algorithm> //std::replace, std::remove
#include <vector>
#include <utility>
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#include <thread> //maybe <spawn.h>
#include "program_data.h" //findIdxOfAutoFileExplorerCmd()
#endif
#include <cstdio> //popen/pclose, fgets

namespace ARVT {

template <int buf_size>
int readPipeIntoString(const char* cmd, std::vector<std::string>& lines) {
	#ifdef _WIN32
	FILE* pipe = _popen(cmd, "rt");
	#else
	FILE* pipe = popen(cmd, "r");
	#endif
	if (pipe == NULL) {
		return 1;
	}

	lines.push_back("");
	char buf[buf_size];
	while (fgets(buf, sizeof(buf), pipe) != NULL) { //reads until newline or EOF
		lines[lines.size()-1] += std::string(buf);
		if (strlen(buf) > 0 && buf[strlen(buf)-1] == '\n') {
			std::string& lastLine = lines[lines.size()-1];
			lastLine.erase(std::remove(lastLine.begin(), lastLine.end(), '\r'), lastLine.end());
			lastLine.erase(std::remove(lastLine.begin(), lastLine.end(), '\n'), lastLine.end());
			lines.push_back("");
		}
	}

	#ifdef _WIN32
	_pclose(pipe);
	#else
	pclose(pipe);
	#endif
	return 0;
}
template int readPipeIntoString<64>(const char* cmd, std::vector<std::string>& lines);
template int readPipeIntoString<2>(const char* cmd, std::vector<std::string>& lines);

void CreateApplicationFoldersIfNeeded() {
	//TODO: filesystem errors

	const auto create_dir = [] (const std::string& dir) -> void {
		const std::string FOLDER = (dir[dir.size()-1] == '/') ? dir.substr(0, dir.size()-1) : dir;
		if (!std::filesystem::exists(FOLDER)) {
			std::filesystem::create_directory(FOLDER);
		} else {
			if (!std::filesystem::is_directory(FOLDER)) {
				std::cerr << ("ERROR: \"" + FOLDER + "\" is not a folder!") << std::endl;
			}
		}
	};

	create_dir(DEFAULT_INPUT_COMMENTS);
	create_dir(DEFAULT_INPUT_SPLITS);
	create_dir(DEFAULT_TEST_IMAGES);
	create_dir(DEFAULT_OUTPUT_SPEECH);
	create_dir(DEFAULT_VIDEO_SETTINGS);
}

void copyEvaluatedFileName_toCommentSplitterPath(ProgramData& pdata) {
	strncpy(pdata.evaluated_input_file_name,     (pdata.get_input_comments_path() + inputFileName_toCommentSplitterName(pdata.the_file_input_name)).c_str(), sizeof(pdata.evaluated_input_file_name)/sizeof(*pdata.evaluated_input_file_name));
}
void copyEvaluatedFileName_toCommentTestImagePath_Text(ProgramData& pdata) {
	strncpy(pdata.evaluated_input_split_1,       (pdata.get_input_splits_path()   + inputFileName_toCommentTestImageName_Text(pdata.the_file_input_name)).c_str(), sizeof(pdata.evaluated_input_split_1)/sizeof(*pdata.evaluated_input_split_1));
}
void copyEvaluatedFileName_toCommentTestImagePath_Speech(ProgramData& pdata) {
	strncpy(pdata.evaluated_input_split_2,       (pdata.get_input_splits_path()   + inputFileName_toCommentTestImageName_Speech(pdata.the_file_input_name)).c_str(), sizeof(pdata.evaluated_input_split_2)/sizeof(*pdata.evaluated_input_split_2));
}
void copyEvaluatedFileName_toCommentTestImagePath_TestImage(ProgramData& pdata, const ImageData& id) {
	strncpy(pdata.evaluated_test_image_path,     (pdata.get_test_images_path()    + inputFileName_toCommentTestImageName_TestImage(pdata.the_file_input_name, id.get_imageFormat().c_str())).c_str(), sizeof(pdata.evaluated_test_image_path)/sizeof(*pdata.evaluated_test_image_path));
}
void copyEvaluatedFileName_toCommentToSpeechPath(ProgramData& pdata, const VideoData& vd) {
	strncpy(pdata.evaluated_output_speech_path,  (pdata.get_output_speech_path()  + inputFileName_toCommentToSpeechName(pdata.the_file_input_name, vd.get_videoContainer().c_str(), vd.audio_only_option_input)).c_str(), sizeof(pdata.evaluated_output_speech_path)/sizeof(*pdata.evaluated_output_speech_path));
}
void copyEvaluatedFileName_toVideoSettingsPath(ProgramData& pdata) {
	strncpy(pdata.evaluated_video_settings_path, (pdata.get_video_settings_path() + inputFileName_toVideoSettingsName(pdata.the_file_input_name)).c_str(), sizeof(pdata.evaluated_video_settings_path)/sizeof(*pdata.evaluated_video_settings_path));
}

std::string inputFileName_toCommentSplitterName(const char* name) {
	return std::string(name) + ".txt";
}
std::string inputFileName_toCommentTestImageName_Text(const char* name) {
	return std::string(name) + "_text.txt";
}
std::string inputFileName_toCommentTestImageName_Speech(const char* name) {
	return std::string(name) + "_speech.txt";
}
std::string inputFileName_toCommentTestImageName_TestImage(const char* name, const char* format) {
	return std::string(name) + "_image" + std::string(format);
}
std::string inputFileName_toCommentToSpeechName(const char* name, const char* container, bool audio_only) {
	if (audio_only) {
		return std::string(name) + "_$.wav";
	} else {
		return std::string(name) + "_$" + std::string(container);
	}
}
std::string inputFileName_toCommentToSpeechName_getFileExplorerName(const char* name, const char* container, bool audio_only) {
	std::string path = inputFileName_toCommentToSpeechName(name, container, audio_only);
	std::replace(path.begin(), path.end(), '$', '1');
	return path;
}
std::string inputFileName_toVideoSettingsName(const char* name) {
	return std::string(name) + ".ini";
}

//helper function using WinAPI to not spawn a new command prompt
int system_helper(const char* command, bool no_cmd) {
	#ifdef _WIN32
	//mostly copied from https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes
	if (no_cmd) {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		memset(&pi, 0, sizeof(pi));

		if( !CreateProcess( NULL,
			(TCHAR*) command,
			NULL,
			NULL,
			FALSE,
			CREATE_NO_WINDOW,  // the entire point of this helper function
			NULL,
			NULL,
			&si,
			&pi ))
		{
			//failed
			return 1;
		}

		WaitForSingleObject( pi.hProcess, INFINITE );
		DWORD exitCode;
		GetExitCodeProcess(pi.hProcess, &exitCode);

		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		return (int)exitCode;
	} else {
		return system(command);
	}
	#else
	//TODO
	return system(command);
	#endif
}

int SpawnDebugConsole() {
	#ifdef _WIN32
	return !AllocConsole();
	#else
	//TODO
	return 1;
	#endif
}

int copyFileToCStr(const char* path, char* dest, int buf_size) {
	//note: this vvv call can throw
	if (!std::filesystem::exists(path)) {
		return 1;
	}
	if (!std::filesystem::is_regular_file(path)) {
		return 1;
	}

	std::ifstream input_file;
	input_file.open(path);
	if (!input_file) {
		//error
		return 1;
	}

	input_file.read(dest, buf_size-1);
	if (input_file.gcount() > 0) {
		//"success" (look, get() for some reason only read a single line)
		dest[input_file.gcount()] = '\0';
		input_file.close();
		return 0;
	} else {
		//error
		//input_file.close();
		return 1;
	}
}

int copy_file(const char* path, const char* newPath) {
	//TODO: this can throw
	if (( std::filesystem::exists(path)    && std::filesystem::is_regular_file(path)) &&
	    (!std::filesystem::exists(newPath) || std::filesystem::is_regular_file(newPath))) {
		std::filesystem::copy_file(path, newPath, std::filesystem::copy_options::overwrite_existing);
		return 0;
	} else {
		return 1;
	}
}

//note: this also checks for existence using <filesystem>
int revealFileExplorer(const char* path, const ProgramData& pdata) {
	//TODO: this can throw
	if (!std::filesystem::is_regular_file(path)) {
		return 1;
	}

	#ifdef _WIN32

	std::string windows_stupid_backslash_requirement = std::string(path);
	std::replace(windows_stupid_backslash_requirement.begin(), windows_stupid_backslash_requirement.end(), '/', '\\');
	std::string command = "explorer /select, \"" + windows_stupid_backslash_requirement + "\"";
	//explorer.exe will always return 1: https://github.com/microsoft/WSL/issues/6565
	system_helper(command.c_str(), true);
	return 0;
	(void)pdata;

	#else

	int cmd_idx = pdata.fileExplorerCmdArray_current;
	if (cmd_idx == 0) {
		cmd_idx = ProgramData::findIdxOfAutoFileExplorerCmd();
		if (cmd_idx < 0) {
			cmd_idx = 1; //xdg-open
		}
	}

	const std::string fileExplorerCmd = std::string(ProgramData::fileExplorerCmdArray_exe[cmd_idx]);
	if (cmd_idx == 0) [[unlikely]] {
		// Shouldn't happen due to above condition, but just in case:
		system_helper((fileExplorerCmd + " \"" + path + "\"").c_str(), true);
		return 1;
	} else if (cmd_idx == 1) {
		// Open on the folder
		std::string temp_path = std::string(path);
		size_t pos = temp_path.find_last_of("/");
		temp_path = temp_path.substr(0, pos);
		return system_helper((fileExplorerCmd + " \"" + temp_path + "\"").c_str(), true);
	} else {
		// On some distros/DEs/file managers, this is blocking, so do it on a new thread just in case
		std::string cmd = fileExplorerCmd + " \"" + path + "\"";
		std::thread t([cmd] { //just [=] works too
			ARVT::system_helper(cmd.c_str(), true);
		});
		t.detach();
		return 0; //assume it worked
	}

	#endif
}

int revealFileExplorer_folderBackup(const char* path, const ProgramData& pdata) {
	if (std::filesystem::exists(path)) {
		return revealFileExplorer(path, pdata);
	} else {
		revealFileExplorer_folderOnly(path);
		return 1;
	}
}

int revealFileExplorer_folderOnly(const char* path) {
	std::string folderpath = std::string(path);
	size_t pos = folderpath.find_last_of('/');
	folderpath.erase(pos);

	if (!std::filesystem::is_directory(folderpath)) {
		return 1;
	}

	//see revealFileExplorer() for comments

	#ifdef _WIN32

	std::string windows_stupid_backslash_requirement = std::string(folderpath);
	std::replace(windows_stupid_backslash_requirement.begin(), windows_stupid_backslash_requirement.end(), '/', '\\');
	std::string command = "explorer \"" + windows_stupid_backslash_requirement + "\""; //no /select
	system_helper(command.c_str(), true);
	return 0;

	#else

	const std::string fileExplorerCmd = std::string(ProgramData::fileExplorerCmdArray_exe[1]); //xdg-open
	return system_helper((fileExplorerCmd + " \"" + folderpath + "\"").c_str(), true);

	#endif
}

int getListOfOldFiles(const char* dir, int hourCount, std::vector<std::string>& deleteFileList) {
	if (!std::filesystem::is_directory(dir)) {
		return 1;
	}

	auto nowTime = std::chrono::file_clock::now();
	for (const auto& file : std::filesystem::directory_iterator(dir)) {
		if (!std::filesystem::is_regular_file(file)) [[unlikely]] {
			continue;
		}

		std::filesystem::file_time_type ftime = std::filesystem::last_write_time(file);
		auto duration = nowTime - ftime;
		if (duration > std::chrono::hours(hourCount)) {
			deleteFileList.push_back(file.path().string());
		}
	}
	return 0;
}

int deleteAllOldFiles(const std::vector<std::string>& fileList) {
	for (const auto& file : fileList) {
		if (std::filesystem::exists(file)) [[likely]] {
			if (!std::filesystem::remove(file)) [[unlikely]] {
				return 1;
			}
		} else {
			//probably error
		}
	}
	return 0;
}



int call_comment_splitter(const ProgramData& pdata) {
	const std::string input_folder = pdata.get_input_comments_path();
	const std::string output_folder = pdata.get_input_splits_path();

	const std::string command = pdata.get_pythonCmd() + " ../scripts/comment_splitter.py" +
		" \"" + input_folder + "\"" +
		" \"" + inputFileName_toCommentSplitterName(pdata.the_file_input_name) + "\"" +
		" \"" + output_folder + "\"" +
		" \"" + inputFileName_toCommentTestImageName_Text(pdata.the_file_input_name) + "\"";

	#ifdef _WIN32
	if (command.size() > 4000) {
		std::cout << "Extremely long command for comment_splitter.py, possibly erroring" << std::endl;
	}
	#endif

	return system_helper(command.c_str(), true);
}

int call_comment_test_image(const ProgramData& pdata, const ImageData& idata) {
	const std::string input_folder = pdata.get_input_splits_path();
	const std::string output_folder = pdata.get_test_images_path();
	const std::string temp_folder = pdata.get_temporary_file_path();

	const std::string command = pdata.get_pythonCmd() + " ../scripts/comment_test_image.py" +
		" \"" + input_folder + "\"" +
		" \"" + inputFileName_toCommentTestImageName_Text(pdata.the_file_input_name) + "\"" +
		" \"" + output_folder + "\"" +
		" \"" + inputFileName_toCommentTestImageName_TestImage(pdata.the_file_input_name, idata.get_imageFormat().c_str()) + "\"" +
		(temp_folder.empty() ? "" : (" -temp \"" + temp_folder + "\"")) +

		" "   + idata.get_image_width_input() +
		" "   + idata.get_image_height_input() +
		" "   + idata.get_image_w_border_input() +
		" "   + idata.get_image_h_border_input() +
		" "   + idata.get_font_size_input() +
		" \"" + idata.get_font_color_input() + "\"" +
		" \"" + idata.get_background_color_input() + "\"" +
		" "   + std::to_string(idata.paragraph_newline_v) +
		" "   + std::to_string(idata.paragraph_tabbed_start_input) +
		" \"" + idata.font_name + "\"" +
		" "   + std::to_string(idata.font_is_family_input) +
		" "   + idata.get_textAlignment() +
		" "   + std::to_string(idata.skip_lone_lf_input) +
		" "   + std::to_string(idata.replace_magick_escape_sequences);

	#ifdef _WIN32
	if (command.size() > 4000) {
		std::cout << "Extremely long command for comment_test_image.py, possibly erroring" << std::endl;
	}
	#endif

	return system_helper(command.c_str(), false);
}

int call_comment_to_speech(const ProgramData& pdata, const ImageData& idata, const AudioData& adata, const VideoData& vdata) {
	const std::string input_folder = pdata.get_input_splits_path();
	const std::string output_folder = pdata.get_output_speech_path();
	const std::string temp_folder = pdata.get_temporary_file_path();

	const std::string command = pdata.get_pythonCmd() + " ../scripts/comment_to_speech.py" +
		" \"" + input_folder + "\"" +
		" \"" + inputFileName_toCommentTestImageName_Text(pdata.the_file_input_name) + "\"" +
		(vdata.use_speech_text ? (" -s \"" + inputFileName_toCommentTestImageName_Speech(pdata.the_file_input_name) + "\"") : "") +
		" \"" + output_folder + "\"" +
		" \"" + inputFileName_toCommentToSpeechName(pdata.the_file_input_name, vdata.get_videoContainer().c_str(), vdata.audio_only_option_input) + "\"" +
		(temp_folder.empty() ? "" : (" -temp \"" + temp_folder + "\"")) +

		" "   + idata.get_image_width_input() +
		" "   + idata.get_image_height_input() +
		" "   + idata.get_image_w_border_input() +
		" "   + idata.get_image_h_border_input() +
		" "   + idata.get_font_size_input() +
		" \"" + idata.get_font_color_input() + "\"" +
		" \"" + idata.get_background_color_input() + "\"" +
		" "   + std::to_string(idata.paragraph_newline_v) +
		" "   + std::to_string(idata.paragraph_tabbed_start_input) +
		" \"" + idata.font_name + "\"" +
		" "   + std::to_string(idata.font_is_family_input) +
		" "   + idata.get_textAlignment() +
		" "   + std::to_string(idata.skip_lone_lf_input) +
		" "   + std::to_string(idata.replace_magick_escape_sequences) +
		" "   + idata.get_imageFormat() +

		" -n \"_" + vdata.get_video_replacement_numbers_input() + "\"" + //-n is required if there's no prepended underscore, because otherwise Python's argparse will treat something like "-3" as an argument and get confused; prefer an underscore regardless in case a future argparse version is pickier
		(vdata.audio_only_option_input ? " -a" : "") +

		" \"" + adata.get_speechEngine() + "\"" +
		" \"" + adata.get_voiceName() + "\"" +
		" \"" + adata.get_audioEncoder()->internalName + "\"" +
		" \"" + adata.get_audioPreset1_currentValue() + "\"" +
		" "   + adata.get_audioBitrate() +
		" \"" + vdata.get_videoEncoder()->internalName + "\"" +
		" \"" + vdata.get_videoPreset1_currentValue() + "\"" +
		" \"" + vdata.get_videoPreset2_currentValue() + "\"" +

		" "   + vdata.get_videoCrf() +
		" "   + vdata.get_fps() +
		" "   + std::to_string(vdata.faststart_flag && vdata.get_faststart_available());

	#ifdef _WIN32
	if (command.size() > 4000) {
		std::cout << "Extremely long command for comment_to_speech.py, possibly erroring" << std::endl;
	}
	#endif

	//TODO: need to exit early if there's an error
	return system_helper(command.c_str(), false);
}

} // namespace ARVT
