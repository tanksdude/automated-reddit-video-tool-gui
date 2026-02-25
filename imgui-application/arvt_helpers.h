#pragma once
#include <string>
#include <vector>
#include "image_data.h"
#include "audio_data.h"
#include "video_data.h"
#include "program_data.h"

namespace ARVT {

const std::string DEFAULT_INPUT_COMMENTS = "../arvt_input_comments/";
const std::string DEFAULT_INPUT_SPLITS   = "../arvt_input_splits/";
const std::string DEFAULT_TEST_IMAGES    = "../arvt_test_images/";
const std::string DEFAULT_OUTPUT_SPEECH  = "../arvt_output_speech/";
const std::string DEFAULT_VIDEO_SETTINGS = "../arvt_video_settings/";

// Public methods:

template <int buf_size = 64>
int readPipeIntoString(const char* cmd, std::vector<std::string>& lines); 
// Templates can be declared in headers if you define *every* instantiation in the source file

void CreateApplicationFoldersIfNeeded();

void copyEvaluatedFileName_toCommentSplitterPath(ProgramData& pdata);
void copyEvaluatedFileName_toCommentTestImagePath_Text(ProgramData& pdata);
void copyEvaluatedFileName_toCommentTestImagePath_Speech(ProgramData& pdata);
void copyEvaluatedFileName_toCommentTestImagePath_TestImage(ProgramData& pdata, const ImageData& id);
void copyEvaluatedFileName_toCommentToSpeechPath(ProgramData& pdata, const VideoData& vd);
void copyEvaluatedFileName_toVideoSettingsPath(ProgramData& pdata);

int copyFileToCStr(const char* path, char* dest, int buf_size);
int copy_file(const char* path, const char* newPath);

int revealFileExplorer(const char* path, const ProgramData& pdata); //note: this also checks for existence using <filesystem>
int revealFileExplorer_folderBackup(const char* path, const ProgramData& pdata); //opens on the folder if the file wasn't found
int revealFileExplorer_folderOnly(const char* path); //requires the folder name to end with '/'

int getListOfOldFiles(const char* dir, int hourCount, std::vector<std::string>& deleteFileList);
int deleteAllOldFiles(const std::vector<std::string>& fileList); //no undo!

int call_comment_splitter(const ProgramData& pdata);
int call_comment_test_image(const ProgramData& pdata, const ImageData& idata);
int call_comment_to_speech(const ProgramData& pdata, const ImageData& idata, const AudioData& adata, const VideoData& vdata);



// Internal use:

std::string inputFileName_toCommentSplitterName(const char* name);
std::string inputFileName_toCommentTestImageName_Text(const char* name);
std::string inputFileName_toCommentTestImageName_Speech(const char* name);
std::string inputFileName_toCommentTestImageName_TestImage(const char* name, const char* format);
std::string inputFileName_toCommentToSpeechName(const char* name, const char* container, bool audio_only);
std::string inputFileName_toCommentToSpeechName_getFileExplorerName(const char* name, const char* container, bool audio_only);
std::string inputFileName_toVideoSettingsName(const char* name);

int system_helper(const char* command, bool no_cmd);
int SpawnDebugConsole();

} // namespace ARVT
