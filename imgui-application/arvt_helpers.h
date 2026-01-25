#pragma once
#include <string>
#include <vector>
#include "image_data.h"
#include "audio_data.h"
#include "video_data.h"
#include "program_data.h"

namespace ARVT {

const std::string INPUT_COMMENTS = "../arvt_input_comments/";
const std::string INPUT_SPLITS = "../arvt_input_splits/";
const std::string TEST_IMAGES = "../arvt_test_images/";
const std::string OUTPUT_SPEECH = "../arvt_output_speech/";
const std::string VIDEO_SETTINGS = "../arvt_video_settings/";
#ifdef _WIN32
const std::string COMMAND_QUOTE = "\""; //TODO: actually use
#else
const std::string COMMAND_QUOTE = "\'";
#endif

// Public methods:

template <int buf_size = 64>
int readPipeIntoString(const char* cmd, std::vector<std::string>& lines); 
// Templates can be declared in headers if you define *every* instantiation in the source file

void CreateApplicationFoldersIfNeeded();

void copyEvaluatedFileName_toCommentSplitterPath(const char* name, char* dest, size_t buf_size);
void copyEvaluatedFileName_toCommentTestImagePath_Text(const char* name, char* dest, size_t buf_size);
void copyEvaluatedFileName_toCommentTestImagePath_Speech(const char* name, char* dest, size_t buf_size);
void copyEvaluatedFileName_toCommentTestImagePath_TestImage(const char* name, const ImageData& id, char* dest, size_t buf_size);
void copyEvaluatedFileName_toCommentToSpeechPath(const char* name, const VideoData& vd, char* dest, size_t buf_size);
void copyEvaluatedFileName_toVideoSettingsPath(const char* name, char* dest, size_t buf_size);

//returns 0 if no errors; does not have to completely fill the buffer
//1: some error; 2: does not exist or is not a file; 3: reading error
int copyFileToCStr(const char* path, char* dest, int buf_size);

int copy_file(const char* path, const char* newPath);
int revealFileExplorer(const char* path, const ProgramData& pdata); //note: this also checks for existence using <filesystem>
int revealFileExplorer_folderBackup(const char* path, const ProgramData& pdata); //opens on the folder if the file wasn't found

int getListOfOldFiles(const char* dir, int hourCount, std::vector<std::string>& deleteFileList);
int deleteAllOldFiles(const std::vector<std::string>& fileList); //no undo!

int call_comment_splitter(const char* name, const ProgramData& pdata);
int call_comment_test_image(const char* name, const ProgramData& pdata, const ImageData& idata);
int call_comment_to_speech(const char* name, const ProgramData& pdata, const ImageData& idata, const AudioData& adata, const VideoData& vdata);



// Internal use:

std::string inputFileName_toCommentSplitterPath(const char* name);
std::string inputFileName_toCommentTestImagePath_Text(const char* name);
std::string inputFileName_toCommentTestImagePath_Speech(const char* name);
std::string inputFileName_toCommentTestImagePath_TestImage(const char* name, const char* format);
std::string inputFileName_toCommentToSpeechPath(const char* name, const char* container, bool audio_only);
std::string inputFileName_toCommentToSpeechPath_getFileExplorerName(const char* name, const char* container, bool audio_only);
std::string inputFileName_toVideoSettingsPath(const char* name);

int system_helper(const char* command, bool no_cmd);
int SpawnDebugConsole();

} // namespace ARVT
