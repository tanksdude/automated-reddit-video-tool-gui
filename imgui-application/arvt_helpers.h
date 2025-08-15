#pragma once
#include <string>
#include "image_data.h"
#include "audio_data.h"
#include "video_data.h"

namespace ARVT {

const std::string INPUT_COMMENTS = "../input_comments/";
const std::string INPUT_SPLITS = "../input_splits/";
const std::string TEST_IMAGES = "../test_images/";
//const std::string TEST_IMAGE_DATA = "../test_image_data/"; //TODO
const std::string OUTPUT_SPEECH = "../output_speech/";
#ifdef _WIN32
const std::string COMMAND_QUOTE = "\""; //TODO: actually use
const std::string PYTHON_CMD = "python";
#else
const std::string COMMAND_QUOTE = "\'";
const std::string PYTHON_CMD = "python3";
#endif

// Public methods:

void CreateApplicationFoldersIfNeeded();

void copyEvaluatedFileName_toCommentSplitterPath(const char* name, char* dest, size_t buf_size);
void copyEvaluatedFileName_toCommentTestImagePath_Text(const char* name, char* dest, size_t buf_size);
void copyEvaluatedFileName_toCommentTestImagePath_Speech(const char* name, char* dest, size_t buf_size);
void copyEvaluatedFileName_toCommentTestImagePath_TestImage(const char* name, const ImageData& id, char* dest, size_t buf_size);
void copyEvaluatedFileName_toCommentToSpeechPath(const char* name, const VideoData& vd, char* dest, size_t buf_size);

//returns 0 if no errors; does not have to completely fill the buffer
//1: some error; 2: does not exist or is not a file; 3: reading error
int copyFileToCStr(const char* path, char* dest, int buf_size); //TODO: maybe make separate versions for each textbox

int copy_file(const char* path, const char* newPath);
int revealFileExplorer(const char* path); //note: this also checks for existence using <filesystem> //TODO: should make a separate version for each path (going further, should that mean everything this program can do is a function exposed here?)
int deleteAllOldFiles(const char* dir, int hourCount);

int call_comment_splitter(const char* name);
int call_comment_test_image(const char* name, const ImageData& idata);
int call_comment_to_speech(const char* name, const ImageData& idata, const AudioData& adata, const VideoData& vdata);



// Internal use:

std::string inputFileName_toCommentSplitterPath(const char* name);
std::string inputFileName_toCommentTestImagePath_Text(const char* name);
std::string inputFileName_toCommentTestImagePath_Speech(const char* name);
std::string inputFileName_toCommentTestImagePath_TestImage(const char* name, const char* format);
std::string inputFileName_toCommentToSpeechPath(const char* name, const char* container, bool audio_only);
std::string inputFileName_toCommentToSpeechPath_getFileExplorerName(const char* name, const char* container, bool audio_only);

inline int system_helper(const char* command, bool no_cmd);

} // namespace ARVT
