#pragma once
#include <string>
#include "image_data.h"
#include "audio_data.h"
#include "video_data.h"

namespace ARVT {

const std::string INPUT_COMMENTS = "../input_comments/";
const std::string INPUT_SPLITS = "../input_splits/";
const std::string OUTPUT_SPEECH = "../output_speech/";
#if defined(WIN32) || defined(_WIN32)
const std::string COMMAND_QUOTE = "\""; //TODO: actually use
#else
const std::string COMMAND_QUOTE = "\'";
#endif

std::string inputFileName_toCommentSplitterPath(const char* name);
std::string inputFileName_toCommentTestImagePath_Text(const char* name);
std::string inputFileName_toCommentTestImagePath_Speech(const char* name);
std::string inputFileName_toCommentTestImagePath_TestImage(const char* name, const char* format);
std::string inputFileName_toCommentToSpeechPath(const char* name, const char* container);
std::string inputFileName_toCommentToSpeechPath_AudioOnly(const char* name);
std::string inputFileName_toCommentToSpeechPath_getFileExplorerName(const char* name, const char* container, bool audioOnly);

inline int system_helper(const char* command, bool no_cmd);

//returns 0 if no errors; does not have to completely fill the buffer
//1: some error; 2: does not exist or is not a file; 3: reading error
int copyFileToCStr(const char* path, char* dest, int buf_size);

int copy_file(const char* path, const char* newPath);
int revealFileExplorer(const char* path); //note: this also checks for existence using <filesystem>

int call_comment_splitter(const char* path, const char* output);
int call_comment_test_image(const char* textPath, const char* output, const ImageData& idata);
int call_comment_to_speech(const char* path, const char* output, const ImageData& idata, const AudioData& adata, const VideoData& vdata);

}
