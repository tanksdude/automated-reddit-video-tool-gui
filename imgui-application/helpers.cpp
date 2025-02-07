#include "helpers.h"
#include <cstdlib> //system()
#include <fstream>
#include <filesystem>
#include <algorithm> //replace()
#include <vector>
#include <utility>
#include <iostream>

#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#endif

namespace ARVT {

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
	return INPUT_SPLITS + std::string(name) + "_image" + std::string(format);
}
std::string inputFileName_toCommentToSpeechPath(const char* name, const char* container) {
	return OUTPUT_SPEECH + std::string(name) + "_$" + std::string(container);
}
std::string inputFileName_toCommentToSpeechPath_AudioOnly(const char* name) {
	return OUTPUT_SPEECH + std::string(name) + "_$.wav";
}
std::string inputFileName_toCommentToSpeechPath_getFileExplorerName(const char* name, const char* container, bool audioOnly) {
	std::string path = audioOnly ? inputFileName_toCommentToSpeechPath_AudioOnly(name) : inputFileName_toCommentToSpeechPath(name, container);
	std::replace(path.begin(), path.end(), '$', '1');
	return path;
}

//helper function using WinAPI to not spawn a new command prompt
inline int ARVT::system_helper(const char* command, bool no_cmd) {
	#if defined(WIN32) || defined(_WIN32)
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
		dest[input_file.gcount()] = '\n';
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

	#if defined(WIN32) || defined(_WIN32)
	std::string windows_stupid_backslash_requirement = std::string(path);
	std::replace(windows_stupid_backslash_requirement.begin(), windows_stupid_backslash_requirement.end(), '/', '\\');
	std::string command = "explorer /select, \"" + windows_stupid_backslash_requirement + "\"";
	//TODO: HACK: prepend powershell to avoid replacing slashes
	return system_helper(command.c_str(), true);
	#else
	//LINUX TODO
	#error "file explorer stuff"
	#endif
}



int call_comment_splitter(const char* path, const char* output) {
	std::string command = "python ../comment_splitter.py " + std::string(path) + " " + std::string(output);
	return system_helper(command.c_str(), true);
}

int call_comment_test_image(const char* textPath, const char* output, const ImageData& idata) {
	std::string command = "python ../comment_test_image.py " + std::string(textPath) + " " + std::string(output) + " " +
		idata.get_image_width_input() + " " +
		idata.get_image_height_input() + " " +
		idata.get_image_w_border_input() + " " +
		idata.get_image_h_border_input() + " " +
		idata.get_font_size_input() + " " +
		"\"" + idata.get_font_color_input() + "\" " +
		"\"" + idata.get_background_color_input() + "\" " +
		"\"" + idata.get_paragraph_separator() + "\" ";

	#if defined(WIN32) || defined(_WIN32)
	if (command.size() > 4000) {
		std::cout << "Extremely long command for comment_test_image.py, possibly erroring" << std::endl;
	}
	#endif

	return system_helper(command.c_str(), false);
}

int call_comment_to_speech(const char* path, const char* output, const ImageData& idata, const AudioData& adata, const VideoData& vdata) {

	//TODO: paths can have spaces

	std::string command = "python ../comment_to_speech.py " + std::string(path) + " " + std::string(output) + " " +
		idata.get_image_width_input() + " " +
		idata.get_image_height_input() + " " +
		idata.get_image_w_border_input() + " " +
		idata.get_image_h_border_input() + " " +
		idata.get_font_size_input() + " " +
		"\"" + idata.get_font_color_input() + "\" " +
		"\"" + idata.get_background_color_input() + "\" " +
		"\"" + idata.get_paragraph_separator() + "\" " +

		"\"" + vdata.get_video_replacement_numbers_input() + "\" " + //TODO: should this have -n?
		(vdata.audio_only_option_input ? "-a " : "") +

		vdata.get_imageFormat() + " " +
		adata.get_voiceEngine() + " " +
		"\"" + adata.get_voice() + "\" " +
		"\"" + adata.get_audioEncoder() + "\" " +
		"\"" + vdata.get_videoEncoder() + "\" " +
		"\"" + vdata.get_videoPreset() + "\" " +
		std::to_string(vdata.faststart_flag) + " " +

		std::to_string(vdata.fps_v) + " " +
		std::to_string(vdata.crf_v) + " ";

	#if defined(WIN32) || defined(_WIN32)
	if (command.size() > 4000) {
		std::cout << "Extremely long command for comment_to_speech.py, possibly erroring" << std::endl;
	}
	#endif

	//TODO: need to exit early if there's an error
	return system_helper(command.c_str(), false);
}

}
