#pragma once
#include "image_data.h"
#include "audio_data.h"
#include "video_data.h"
#include "program_data.h"

#define MINI_CASE_SENSITIVE
#include <mini/ini.h>

namespace ARVT {

void CreateDefaultIniIfNeeded(const std::string& path);

void Fill_ImageData(ImageData& idata, const mINI::INIStructure& ini_object);
void Fill_AudioData(AudioData& adata, const mINI::INIStructure& ini_object, bool useExtraCodecs);
void Fill_VideoData(VideoData& vdata, const mINI::INIStructure& ini_object, bool useExtraCodecs);
void Fill_ProgramData(ProgramData& pdata, const mINI::INIStructure& ini_object);

void CopySettingsToIni(mINI::INIStructure& ini_object, const ImageData& idata, const AudioData& adata, const VideoData& vdata);

} // namespace ARVT
