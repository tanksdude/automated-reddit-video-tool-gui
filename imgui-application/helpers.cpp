#include "helpers.h"
#include <cstdlib> //system()
#include <cstring> //strncpy
#include <fstream>
#include <filesystem>
#include <algorithm> //replace()
#include <vector>
#include <utility>
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace ARVT {

void copyEvaluatedFileName_toCommentSplitterPath(const char* name, char* dest, size_t buf_size) {
	strncpy(dest, inputFileName_toCommentSplitterPath(name).c_str(), buf_size);
}
void copyEvaluatedFileName_toCommentTestImagePath_Text(const char* name, char* dest, size_t buf_size) {
	strncpy(dest, inputFileName_toCommentTestImagePath_Text(name).c_str(), buf_size);
}
void copyEvaluatedFileName_toCommentTestImagePath_Speech(const char* name, char* dest, size_t buf_size) {
	strncpy(dest, inputFileName_toCommentTestImagePath_Speech(name).c_str(), buf_size);
}
void copyEvaluatedFileName_toCommentTestImagePath_TestImage(const char* name, const ImageData& id, char* dest, size_t buf_size) {
	strncpy(dest, inputFileName_toCommentTestImagePath_TestImage(name, id.get_imageFormat().c_str()).c_str(), buf_size);
}
void copyEvaluatedFileName_toCommentToSpeechPath(const char* name, const VideoData& vd, char* dest, size_t buf_size) {
	strncpy(dest, inputFileName_toCommentToSpeechPath(name, vd.get_videoContainer().c_str(), vd.audio_only_option_input).c_str(), buf_size);
}

std::string inputFileName_toCommentSplitterPath(const char* name) {
	return INPUT_COMMENTS + std::string(name) + ".txt";
}
std::string inputFileName_toCommentTestImagePath_Text(const char* name) {
	return INPUT_SPLITS + std::string(name) + "_text.txt";
}
std::string inputFileName_toCommentTestImagePath_Speech(const char* name) {
	return INPUT_SPLITS + std::string(name) + "_speech.txt";
}
std::string inputFileName_toCommentTestImagePath_TestImage(const char* name, const char* format) {
	return TEST_IMAGES + std::string(name) + "_image" + std::string(format);
}
std::string inputFileName_toCommentToSpeechPath(const char* name, const char* container, bool audio_only) {
	if (audio_only) {
		return OUTPUT_SPEECH + std::string(name) + "_$.wav";
	} else {
		return OUTPUT_SPEECH + std::string(name) + "_$" + std::string(container);
	}
}
std::string inputFileName_toCommentToSpeechPath_getFileExplorerName(const char* name, const char* container, bool audio_only) {
	std::string path = inputFileName_toCommentToSpeechPath(name, container, audio_only);
	std::replace(path.begin(), path.end(), '$', '1');
	return path;
}

//helper function using WinAPI to not spawn a new command prompt
inline int system_helper(const char* command, bool no_cmd) {
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
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		return 0;
	} else {
		return system(command);
	}
	#else
	//TODO
	return system(command);
	#endif
}

int copyFileToCStr(const char* path, char* dest, int buf_size) {
	//note: this vvv call can throw
	if (!std::filesystem::exists(path)) {
		return 2;
	}
	if (!std::filesystem::is_regular_file(path)) {
		return 2;
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
		return 3;
	}
}

int copy_file(const char* path, const char* newPath) {
	//TODO: this can throw
	std::filesystem::copy_file(path, newPath, std::filesystem::copy_options::overwrite_existing);
	return 0;
}

//note: this also checks for existence using <filesystem>
int revealFileExplorer(const char* path) {
	//note: this vvv call can throw
	if (!std::filesystem::exists(path)) {
		return 1;
		//TODO: don't have to check for existence?
	}
	if (!std::filesystem::is_regular_file(path)) {
		return 1;
	}

	#ifdef _WIN32
	std::string windows_stupid_backslash_requirement = std::string(path);
	std::replace(windows_stupid_backslash_requirement.begin(), windows_stupid_backslash_requirement.end(), '/', '\\');
	std::string command = "explorer /select, \"" + windows_stupid_backslash_requirement + "\"";
	//TODO: HACK: prepend powershell to avoid replacing slashes
	return system_helper(command.c_str(), true);
	#else
	//LINUX TODO
	//#error "file explorer stuff"
	return 0;
	#endif
}

int deleteAllOldFiles(const char* dir, int hourCount) {
	//TODO: this needs to have some kind of guarantee that the path is somewhere inside this project's root folder
	//it does work though, but I'm not pushing it until there's safety
	return 0;

	/*
	if (hourCount <= 0) [[unlikely]] {
		return 0;
	}
	//TODO: this should print a list for confirmation
	auto nowTime = std::chrono::file_clock::now();
	for (const auto& file : std::filesystem::directory_iterator(dir)) {
		std::filesystem::file_time_type ftime = std::filesystem::last_write_time(file);
		auto duration = nowTime - ftime;
		if (duration > std::chrono::hours(hourCount)) {
			std::filesystem::remove(file);
		}
	}
	return 0;
	*/
}



int call_comment_splitter(const char* name) {
	const std::string input_path = inputFileName_toCommentSplitterPath(name);
	const std::string output_path = inputFileName_toCommentTestImagePath_Text(name);
	const std::string command = PYTHON_CMD + " ../comment_splitter.py \"" + input_path + "\" \"" + output_path + "\"";
	return system_helper(command.c_str(), true);
}

int call_comment_test_image(const char* name, const ImageData& idata) {
	const std::string text_path = inputFileName_toCommentTestImagePath_Text(name);
	const std::string image_path = inputFileName_toCommentTestImagePath_TestImage(name, idata.get_imageFormat().c_str());
	const std::string command = PYTHON_CMD + " ../comment_test_image.py \"" + std::string(text_path) + "\" \"" + std::string(image_path) + "\" " +
		idata.get_image_width_input() + " " +
		idata.get_image_height_input() + " " +
		idata.get_image_w_border_input() + " " +
		idata.get_image_h_border_input() + " " +
		idata.get_font_size_input() + " " +
		"\"" + idata.get_font_color_input() + "\" " +
		"\"" + idata.get_background_color_input() + "\" " +
		std::to_string(idata.paragraph_newline_v) + " " +
		std::to_string(idata.paragraph_tabbed_start_input) + " ";

	#ifdef _WIN32
	if (command.size() > 4000) {
		std::cout << "Extremely long command for comment_test_image.py, possibly erroring" << std::endl;
	}
	#endif

	return system_helper(command.c_str(), false);
}

int call_comment_to_speech(const char* name, const ImageData& idata, const AudioData& adata, const VideoData& vdata) {
	const std::string text_path = inputFileName_toCommentTestImagePath_Text(name);
	const std::string speech_path = inputFileName_toCommentTestImagePath_Speech(name);
	const std::string video_path = inputFileName_toCommentToSpeechPath(name, vdata.get_videoContainer().c_str(), vdata.audio_only_option_input);
	const std::string command = PYTHON_CMD + " ../comment_to_speech.py \"" + std::string(text_path) + "\" \"" + std::string(video_path) + "\" " +
		(vdata.use_speech_text ? "-s \"" + std::string(speech_path) + "\" " : "") +
		idata.get_image_width_input() + " " +
		idata.get_image_height_input() + " " +
		idata.get_image_w_border_input() + " " +
		idata.get_image_h_border_input() + " " +
		idata.get_font_size_input() + " " +
		"\"" + idata.get_font_color_input() + "\" " +
		"\"" + idata.get_background_color_input() + "\" " +
		std::to_string(idata.paragraph_newline_v) + " " +
		std::to_string(idata.paragraph_tabbed_start_input) + " " +
		idata.get_imageFormat() + " " +

		"-n \"" + vdata.get_video_replacement_numbers_input() + "\" " + //needs the -n flag, otherwise Python's argparse will treat something like "-3" as an argument and get confused (pretty sure it's allowed because "-3" is a negative number)
		(vdata.audio_only_option_input ? "-a " : "") +

		"\"" + adata.get_voiceEngine() + "\" " +
		"\"" + adata.get_voice() + "\" " +
		"\"" + adata.get_audioEncoder() + "\" " +
		adata.get_audioBitrate() + " " +
		"\"" + adata.get_audioPreset() + "\" " +
		"\"" + vdata.get_videoEncoder() + "\" " +
		"\"" + vdata.get_videoPreset1() + "\" " +
		"\"" + vdata.get_videoPreset2() + "\" " +
		std::to_string(vdata.faststart_flag && vdata.get_faststart_available()) + " " +

		vdata.get_fps() + " " +
		vdata.get_videoCrf() + " ";

	#ifdef _WIN32
	if (command.size() > 4000) {
		std::cout << "Extremely long command for comment_to_speech.py, possibly erroring" << std::endl;
	}
	#endif

	//TODO: need to exit early if there's an error
	return system_helper(command.c_str(), false);
}

} // namespace ARVT
