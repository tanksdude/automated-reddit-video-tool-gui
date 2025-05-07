#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct AudioData {
	struct BitrateData {
		// Bitrate in kbps
		std::int16_t starting_value;
		std::int16_t min_value; //"sane" min
		std::int16_t max_value; //"sane" max
		std::int16_t codec_default_value; //unused
		std::int16_t codec_min_value;     //unused
		std::int16_t codec_max_value;     //unused
	};

	struct CodecPresetInformation {
		//yes this is the exact same struct as VideoData, it's whatever
		std::string term;
		std::vector<const char*>& presetArray;
	};

	struct AudioCodecMiscInformation {
		//yes this is the exact same enum as VideoData, it's whatever
		enum class RecommendedLevel : uint8_t {
			No_Opinion,
			Awful,
			Okay,
			Good,
			Best,
		};
		RecommendedLevel recommendation;
		bool lossless;
		std::string information_text;
		std::string get_recommendedStr() const;
	};

	/* Passing voice engine information to the Python script:
	 * The Python script will read the string character for character. It will
	 * translate each voice engine name to the correct program.
	 */
#ifdef _WIN32
	static const char* voiceEngineArray[4];
	static const char* voiceEngineArray_exeForUpdatingVoiceList[4]; //note: internal use only
#else
	static const char* voiceEngineArray[5];
	static const char* voiceEngineArray_exeForUpdatingVoiceList[5]; //note: internal use only
#endif
	int voiceEngineArray_current = 0;

	char** voiceArray = nullptr;
	int voiceArray_current;
	int voiceArray_length;

	static void getVoiceListFromExe_Balabolka(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList);
	static void getVoiceListFromExe_Espeak(std::vector<std::string>& file_lines, std::vector<std::string>& voiceList);

	/* Passing codec information to the Python script:
	 * The Python script will ignore everything after the first space. This
	 * means "copy (pcm)" will be interpreted the same as "copy".
	 */
	static const char* audioEncoderArray[7];
	/* Passing "preset" information to the Python script:
	 * The Python script will ignore everything after the first space. This
	 * means "5 (default)" will be interpreted the same as "5".
	 */
	static std::vector<const char*> audioPresetArray_AAC;
	static std::vector<const char*> audioPresetArray_Opus;
	static std::vector<const char*> audioPresetArray_FLAC;
	static std::vector<const char*> audioPresetArray_MP3;
	static std::vector<const char*> audioPresetArray_empty; //placeholder for the hashmap lookups
	//other possibilities: flac lpc_coeff_precision, libopus application, aac profile ("low complexity" default, "main", "scalable sampling rate")

	static const std::unordered_map<std::string, CodecPresetInformation> codecToPresetArray;
	static const std::unordered_map<std::string, BitrateData> codecToBitrate;
	static const std::unordered_map<std::string, AudioCodecMiscInformation> codecMiscInformation;

	int audioEncoderArray_current = 0;
	bool get_audioEncoderIsLossless() const;
	std::string get_audioEncoderRecommendationStr() const;
	std::string get_audioEncoderInformationText() const;

	bool audioCodec_hasPreset;
	int audioPresetArray_current;
	std::string audioCodec_presetTerm;
	const char** get_audioPresetArray() const;
	int get_audioPresetArray_size() const;

	std::uint16_t audio_bitrate_v = 192;
	std::uint16_t audio_bitrate_min = 60;
	std::uint16_t audio_bitrate_max = 300;
	std::uint16_t audio_bitrate_step = 4; //TODO: unused because ImGui sliders don't support stepping

	inline std::string get_voiceEngine() const { return std::string(voiceEngineArray[voiceEngineArray_current]); }
	inline std::string get_voice() const {
		if (voiceArray_current < 0) { return ""; }
		return std::string(voiceArray[voiceArray_current]);
	}
	inline std::string get_audioEncoder() const { return std::string(audioEncoderArray[audioEncoderArray_current]); }
	inline std::string get_audioBitrate() const { return std::to_string(audio_bitrate_v) + "k"; }
	std::string get_audioPreset() const;

	void update_voiceArray(); // updates voiceArray and its related data
	void update_audioBitrateValues();
	void update_audioPresetArray();
	
	AudioData() {
		update_voiceArray();
		update_audioPresetArray();
	}

	~AudioData() {
		if (voiceArray != nullptr) [[likely]] {
			for (int i = 0; i < voiceArray_length; i++) {
				delete voiceArray[i];
			}
			delete[] voiceArray;
		}
	}
};
