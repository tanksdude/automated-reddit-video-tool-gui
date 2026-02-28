#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stdio.h> //fprintf for glfwSetErrorCallback
#include <algorithm> //std::max_element
#include <cstring> //strcmp
#include <filesystem> //only for refreshApplicationFontName(), though plenty of other files also use <filesystem>
#include <iostream> //not actually needed, but it's useful for debugging

#include <GLFW/glfw3.h>
#define MINI_CASE_SENSITIVE
#include <mini/ini.h>
#include "libs/IconFontCppHeaders/IconsFontAwesome6.h"

#include "imgui_helpers.h"
#include "arvt_helpers.h"
#include "ini_helper.h"
#include "arvt_logger.h"

#include "image_data.h"
#include "audio_data.h"
#include "video_data.h"
#include "program_data.h"

ImageData idata;
AudioData adata;
VideoData vdata;
ProgramData pdata;

ARVT_Logger global_log(true);
ARVT_Logger deleteFileLogger(false);
std::vector<std::string> deleteFileList;

bool needToChangeFonts = false;
ImFont* newFontToSwitchTo = nullptr;

inline bool mergeIconFontToCurrentFont() {
	if (std::filesystem::exists("../res/fa6-solid-900.ttf")) {
		ImFontConfig config;
		config.MergeMode = true;
		ImGui::GetIO().Fonts->AddFontFromFileTTF("../res/fa6-solid-900.ttf", 0.0f, &config);
		return true;
	} else {
		return false;
	}
}

void refreshApplicationFontName(bool loadingStartupFont = false) {
	//if the input is empty, load ImGui's default font
	if (pdata.application_font_path[0] == '\0') {
		//see ImGui::ShowFontSelector()
		ImGuiIO& io = ImGui::GetIO();
		for (ImFont* font : io.Fonts->Fonts) {
			if (strcmp(font->GetDebugName(), "ProggyClean.ttf") == 0) {
				io.FontDefault = font;
				break;
			}
		}
		global_log.AddLog("[info]", "Font", "Successfully loaded ProggyClean.ttf");
		return;
	}

	//TODO: should probably use ImFontFlags_NoLoadError instead, but this is good enough
	if (!std::filesystem::exists(pdata.application_font_path)) {
		global_log.AddLog("[error]", "Font", "Font does not exist");
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	ImFont* newFont = io.Fonts->AddFontFromFileTTF(pdata.application_font_path);
	if (newFont == nullptr) {
		//TODO: ImFontFlags_NoLoadError
		return;
	}
	io.FontDefault = newFont;
	if (!mergeIconFontToCurrentFont()) {
		global_log.AddLog("[error]", "Font", "Could not add icon font");
	}

	//ideally this would be used but it doesn't change the font for some reason:
	//ImGui::PushFont(newFont, newSize);

	if (!loadingStartupFont) {
		global_log.AddLog("[info]", "Font", ("Successfully added font " + std::string(pdata.application_font_path)).c_str());
	}

	//TODO: should re-scale for the monitor's scale factor again
}

void refreshApplicationFontSize() {
	//TODO: change this when ImGui fixes it
	ImGuiStyle& style = ImGui::GetStyle();
	style.FontSizeBase = pdata.application_font_size;
	style._NextFrameFontSizeBase = style.FontSizeBase; // FIXME: Temporary hack until we finish remaining work.

	//TODO: should re-scale for the monitor's scale factor again
}

auto integerOnlyPositiveFunc = [] (ImGuiInputTextCallbackData* data) {
	if (data->EventChar >= '0' && data->EventChar <= '9') { return 0; }
	return 1;
};
auto numberOnlyPositiveFunc = [] (ImGuiInputTextCallbackData* data) {
	if ((data->EventChar >= '0' && data->EventChar <= '9') || data->EventChar == '.') { return 0; }
	return 1;
};
auto filenameCleaningFunc = [] (ImGuiInputTextCallbackData* data) {
	//main goal: don't allow quotes
	//additionally, don't allow NTFS-invalid characters, list from https://en.wikipedia.org/wiki/NTFS
	if (data->EventChar == '/'  ||
	    data->EventChar == '\\' ||
	    data->EventChar == ':'  ||
	    data->EventChar == '*'  ||
	    data->EventChar == '\"' || //single quotes can be allowed as long as system() calls use double quotes
	    data->EventChar == '?'  ||
	    data->EventChar == '<'  ||
	    data->EventChar == '>'  ||
	    data->EventChar == '|')
		{ return 1; }
	return 0;
};
auto filenameCleaningFunc_inputFile = [] (ImGuiInputTextCallbackData* data) {
	//main goal: don't allow '$' because that's used for the number in the Python script
	if (data->EventChar == '$') { return 1; }
	return filenameCleaningFunc(data);
};
auto filepathCleaningFunc = [] (ImGuiInputTextCallbackData* data) {
	//identical to filenameCleaningFunc but allows forward slashes (and colons)
	if (data->EventChar == '\\' ||
	    data->EventChar == '*'  ||
	    data->EventChar == '\"' ||
	    data->EventChar == '?'  ||
	    data->EventChar == '<'  ||
	    data->EventChar == '>'  ||
	    data->EventChar == '|')
		{ return 1; }
	return 0;
};
auto video_replacement_scrubbingFunc = [] (ImGuiInputTextCallbackData* data) {
	if (data->EventChar >= '0' && data->EventChar <= '9') { return 0; }
	if (data->EventChar == ',' || data->EventChar == '-' || data->EventChar == ' ') { return 0; }
	return 1;
};

// Note: this only holds the data that can get changed by other functions,
// does not actually hold all state data needed for this program
struct GlobalStateStruct {
	// Preview data, for coloring text red on errors:
	bool input_comment_data_is_bad = false;
	bool input_split_1_data_is_bad = false;
	bool input_split_2_data_is_bad = false;

	// The test image:
	int createdTestImage_width = 0;
	int createdTestImage_height = 0;
	GLuint createdTestImage_texture = 0;
};

// Called when the lock button is clicked
void clear_input_data(bool lockNewState, GlobalStateStruct& global_state) {
	if (lockNewState) {
		//now locked
		strcpy(pdata.input_comment_data, "");
		strcpy(pdata.input_split_1_data, "");
		strcpy(pdata.input_split_2_data, "");
		global_state.input_comment_data_is_bad = false;
		global_state.input_split_1_data_is_bad = false;
		global_state.input_split_2_data_is_bad = false;
		global_state.createdTestImage_texture = global_state.createdTestImage_width = global_state.createdTestImage_height = 0;
	} else {
		//now unlocked
		//nothing
	}
}

inline void lock_filename_tooltip(bool filenameIsLocked) {
	if (!filenameIsLocked) {
		if (ImGui::BeginItemTooltip()) {
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("Lock the file name before clicking this!");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
}

#include <thread>
#include <atomic>
std::atomic_bool thread_func_speech_working = false;

/* Note: this function MUST take pointers rather than references because
 * detached threads' destructors destroy all their objects, which for some
 * reason includes const references.
 * If this isn't done, then AudioData's destructor frees its voiceArray, and
 * calling this function again will call the destructor again, resulting in a
 * double free.
 * Maybe it would be easier to write a basic thread manager and have the
 * threads exit on program exit...
 */
void thread_func_speech(const ProgramData* pdata, const ImageData* idata, const AudioData* adata, const VideoData* vdata) {
	int result = ARVT::call_comment_to_speech(*pdata, *idata, *adata, *vdata);
	thread_func_speech_working.store(false);
	//return result;

	// Logging here is a race condition... but it doesn't really matter
	if (result) {
		global_log.AddLog("[error]", "Video", "Encountered an error when making the videos");
	} else {
		global_log.AddLog("[info]", "Video", ("Successfully created videos " + std::string((*pdata).evaluated_output_speech_path)).c_str());
	}
}

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


int main(int, char**) {
	ARVT::CreateDefaultIniIfNeeded("../arvt.ini");
	mINI::INIFile ini_file("../arvt.ini");
	mINI::INIStructure ini_object;
	ini_file.read(ini_object);

	ARVT::Fill_ProgramData(pdata, ini_object);
	ARVT::Fill_ImageData(idata, ini_object);
	ARVT::Fill_AudioData(adata, ini_object, pdata.useExtraCodecs);
	ARVT::Fill_VideoData(vdata, ini_object, pdata.useExtraCodecs);

	#ifdef _WIN32
	if (pdata.spawn_debug_console) {
		if (ARVT::SpawnDebugConsole()) {
			std::cerr << "ERROR: could not spawn console" << std::endl;
		}
	}
	#else
	//TODO
	#endif

	// If the user didn't set a speech engine in the INI, then fill its
	// available voices. (If the user did, then the values are already filled.)
	if (ini_object.get("AUDIO").get("SpeechEngine").empty()) {
		if (adata.update_voiceArray()) {
			// Don't log because if the user doesn't have the default TTS program then it's annoying to get an error on every startup
			// Also can't log because ImGui hasn't been initialized yet (because ImGui::GetTime() gets called)
		}
	}

	ARVT::CreateApplicationFoldersIfNeeded();

	/* start ImGui code */

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	glfwWindowHint(GLFW_SCALE_TO_MONITOR, pdata.application_scale_to_monitor ? GLFW_TRUE : GLFW_FALSE);
    // Create window with graphics context
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow(pdata.initial_windowWidth, pdata.initial_windowHeight, "Automated Reddit Video Tool GUI v1.0.0", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    io.Fonts->AddFontDefault();
	mergeIconFontToCurrentFont();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");

	/* end ImGui code */

	// Load an icon for fun
	{
		const char* favicon_path = "../favicon.png";
		GLFWimage images[1];
		auto ret = stbi_load(favicon_path, &images[0].width, &images[0].height, NULL, 4);
		if (ret == NULL) {
			std::cerr << "Could not load \"" << favicon_path << "\": " << stbi_failure_reason() << std::endl;
			global_log.AddLog("[error]", "Icon", stbi_failure_reason());
		} else {
			images[0].pixels = ret;
			glfwSetWindowIcon(window, 1, images);
			stbi_image_free(images[0].pixels);
		}
	}

	GlobalStateStruct global_state;

	bool show_demo_window = false;
	bool set_startup_tab = true;
	bool filenameIsLocked = false;
	bool ret;

	GLuint lock_icon_texture = 0, unlock_icon_texture = 0;
	ret = ImGuiHelpers::LoadTextureFromFile("../res/locked_1f512.png", &lock_icon_texture, NULL, NULL);
	IM_ASSERT(ret);
	ret = ImGuiHelpers::LoadTextureFromFile("../res/unlocked_1f513.png", &unlock_icon_texture, NULL, NULL);
	IM_ASSERT(ret);

	GLuint file_folder_texture = 0;
	ret = ImGuiHelpers::LoadTextureFromFile("../res/file-folder_1f4c1.png", &file_folder_texture, NULL, NULL);

	GLuint recommended_awful, recommended_okay, recommended_good, recommended_best, recommended_noopinion;
	ret = ImGuiHelpers::LoadTextureFromFile("../res/cross-mark_274c.png", &recommended_awful, NULL, NULL);
	ret = ImGuiHelpers::LoadTextureFromFile("../res/warning_26a0-fe0f.png", &recommended_okay, NULL, NULL); //TODO: it really shouldn't be a warning sign...
	ret = ImGuiHelpers::LoadTextureFromFile("../res/heavy-check-mark_2714.png", &recommended_good, NULL, NULL);
	ret = ImGuiHelpers::LoadTextureFromFile("../res/gem-stone_1f48e.png", &recommended_best, NULL, NULL);
	ret = ImGuiHelpers::LoadTextureFromFile("../res/white-question-mark-ornament_2754.png", &recommended_noopinion, NULL, NULL);
	//wow, this is a bad emoji: ❎

	std::unordered_map<CodecRecommendedLevel, GLuint> recommendationStr_toTexId = {
		{ CodecRecommendedLevel::Awful, recommended_awful },
		{ CodecRecommendedLevel::Okay, recommended_okay },
		{ CodecRecommendedLevel::Good, recommended_good },
		{ CodecRecommendedLevel::Best, recommended_best },
		{ CodecRecommendedLevel::No_Opinion, recommended_noopinion },
	};

	refreshApplicationFontSize();
	refreshApplicationFontName(true);
	style.Colors[ImGuiCol_WindowBg] = pdata.window_color;

	global_log.AddLog("[info]", "Startup");

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		// Font handling
		if (needToChangeFonts) [[unlikely]] {
			if (newFontToSwitchTo == nullptr) {
				refreshApplicationFontName();
			} else {
				io.FontDefault = newFontToSwitchTo;
				global_log.AddLog("[info]", "Font", ("Successfully switched to font " + std::string(newFontToSwitchTo->GetDebugName())).c_str());
				newFontToSwitchTo = nullptr;
			}
			needToChangeFonts = false;
		}

		/* start ImGui code */

        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		/* end ImGui code */

		// Main window:
		{
			const bool THREAD_IS_WORKING = thread_func_speech_working.load();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			int window_width, window_height;
			glfwGetWindowSize(window, &window_width, &window_height);
			ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
			ImGui::SetWindowPos(ImVec2(0, 0));
			ImGui::SetWindowSize("Main Window", ImVec2(window_width, window_height));
			const ImVec2 ImageButtonSize = { ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() };

			if (ImGui::BeginTabBar("MainTabBar", 0)) {
				ImGuiTabItemFlags tab_flags[3] = { 0, 0, 0 };
				if (set_startup_tab) [[unlikely]] {
					//instead of clamping, set to zero on a bad value, because that's more obvious (maybe Help or About would be better?):
					const int idx = (pdata.startup_tab_idx >= IM_ARRAYSIZE(tab_flags) || pdata.startup_tab_idx < 0) ? 0 : pdata.startup_tab_idx;
					tab_flags[idx] |= ImGuiTabItemFlags_SetSelected;
					set_startup_tab = false;
				}
				const ImGuiTableFlags table_flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_PadOuterX;

				if (ImGui::BeginTabItem("Execute", nullptr, tab_flags[0])) {
					if (ImGui::BeginTable("Execute##table1", 4, table_flags)) {
						ImGui::TableSetupColumn("Input Comment");
						ImGui::TableSetupColumn("Splits");
						ImGui::TableSetupColumn("Image Parameters");
						ImGui::TableSetupColumn("Test Image");
						ImGui::TableHeadersRow();
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						if (THREAD_IS_WORKING) { ImGui::BeginDisabled(); }

						if (filenameIsLocked) { ImGui::BeginDisabled(); }
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 1.5f*style.ItemInnerSpacing.x);
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + style.FramePadding.y); // Because an InputText doesn't exist yet, Text will not be y-aligned with it
						ImGui::Text("File Name:");
						ImGui::SameLine();

						ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.FramePadding.y); // "Correct" the cursor pos back to where it "should" be
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.7f);
						ImGui::InputText("##Main Input Comment", pdata.the_file_input_name, IM_ARRAYSIZE(pdata.the_file_input_name), ImGuiInputTextFlags_CallbackCharFilter, filenameCleaningFunc_inputFile);
						ImGui::PopItemWidth();
						ImGui::SameLine();

						ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.FramePadding.y); // Again
						if (ImGui::ImageButton("##Folder Icon", file_folder_texture, ImageButtonSize)) {
							int result = ARVT::revealFileExplorer_folderOnly(pdata.get_input_comments_path().c_str());
							if (result) {
								global_log.AddLog("[warn]", "File Explorer", "Some error");
							}
						}
						if (ImGui::BeginItemTooltip()) {
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							ImGui::TextUnformatted(filenameIsLocked ? "Don't modify it!" : "Open the folder");
							ImGui::PopTextWrapPos();
							ImGui::EndTooltip();
						}
						if (filenameIsLocked) { ImGui::EndDisabled(); }
						ImGui::SameLine();

						ImGui::SetCursorPosY(ImGui::GetCursorPosY() - style.FramePadding.y); // Again
						if (ImGui::ImageButton("##Lock Icon", filenameIsLocked ? lock_icon_texture : unlock_icon_texture, ImageButtonSize)) {
							clear_input_data(filenameIsLocked, global_state);
							filenameIsLocked = !filenameIsLocked;
						}

						// #if _DEBUG
						// ImGui::Checkbox("Demo Window", &show_demo_window);
						// #endif

						ARVT::copyEvaluatedFileName_toCommentSplitterPath(pdata);
						ImGui::InputText("##Input Comment Path", pdata.evaluated_input_file_name, IM_ARRAYSIZE(pdata.evaluated_input_file_name), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);
						ImGui::SameLine();

						if (ImGui::Button("Preview File##Input Comment")) { //TODO: add?: https://github.com/mlabbe/nativefiledialog
							int result = ARVT::copyFileToCStr(pdata.evaluated_input_file_name, pdata.input_comment_data, IM_ARRAYSIZE(pdata.input_comment_data));
							if (result) {
								strcpy(pdata.input_comment_data, "error reading");
								global_state.input_comment_data_is_bad = true;
								global_log.AddLog("[warn]", "File Preview", "Could not preview the file");
							} else {
								global_state.input_comment_data_is_bad = false;
							}
						}
						if (ImGui::BeginItemTooltip()) {
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							ImGui::TextUnformatted("Display the first 16K characters.");
							ImGui::PopTextWrapPos();
							ImGui::EndTooltip();
						}

						/*
						ImGui::Checkbox("Word Wrap##Input Comment", &pdata.input_comment_word_wrap);
						//word wrap doesn't affect TextMultiline for some reason
						*/

						/*
						if (pdata.input_comment_word_wrap) {
							ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);
						}
						*/

						if (global_state.input_comment_data_is_bad) { ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.0f, 0.0f, 1.0f }); }
						ImGui::InputTextMultiline("##input comment", pdata.input_comment_data, IM_ARRAYSIZE(pdata.input_comment_data), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);
						if (global_state.input_comment_data_is_bad) { ImGui::PopStyleColor(); }

						/*
						if (pdata.input_comment_word_wrap) {
							ImGui::PopTextWrapPos();
						}
						*/

						if (adata.voiceArray_current < 0) {
							const char* text1 = "You haven't set a voice yet!";
							const char* text2 = "Go to the Configure tab.";
							const float text1_size = ImGui::CalcTextSize(text1).x; //adding 2*style.FramePadding.x looks wrong
							const float text2_size = ImGui::CalcTextSize(text2).x;
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - text1_size)/2);
							ImGui::TextColored(ImVec4(1, 0, 0, 1), text1);
							ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - text2_size)/2);
							ImGui::TextColored(ImVec4(1, 0, 0, 1), text2);
						}
						const float no_voice_height = (adata.voiceArray_current < 0) ? 2 * ImGui::GetTextLineHeight() + style.ItemSpacing.y : 0.0f;

						ImGui::TableNextColumn();

						if (!filenameIsLocked) { ImGui::BeginDisabled(); }

						ImGui::SeparatorText("Image Text");

						ARVT::copyEvaluatedFileName_toCommentTestImagePath_Text(pdata);
						ARVT::copyEvaluatedFileName_toCommentTestImagePath_Speech(pdata);

						ImGui::InputText("##Input Split 1 Path", pdata.evaluated_input_split_1, IM_ARRAYSIZE(pdata.evaluated_input_split_1), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);
						ImGui::SameLine();
						if (ImGui::Button("Preview File##Input Split 1")) {
							int result = ARVT::copyFileToCStr(pdata.evaluated_input_split_1, pdata.input_split_1_data, IM_ARRAYSIZE(pdata.input_split_1_data));
							if (result) {
								strcpy(pdata.input_split_1_data, "error reading");
								global_state.input_split_1_data_is_bad = true;
								global_log.AddLog("[warn]", "File Preview", "Could not preview the file");
							} else {
								global_state.input_split_1_data_is_bad = false;
							}
						}

						if (global_state.input_split_1_data_is_bad) { ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.0f, 0.0f, 1.0f }); }
						ImGui::InputTextMultiline("##Input Split 1 Data", pdata.input_split_1_data, IM_ARRAYSIZE(pdata.input_split_1_data), ImVec2(-FLT_MIN, 0), ImGuiInputTextFlags_ReadOnly);
						if (global_state.input_split_1_data_is_bad) { ImGui::PopStyleColor(); }
						if (ImGui::Button("Reveal in File Explorer##Input Split 1")) {
							int result = ARVT::revealFileExplorer(pdata.evaluated_input_split_1, pdata);
							if (result) {
								global_log.AddLog("[warn]", "File Explorer", "Some error");
							}
						}
						ImGui::SameLine();
						if (ImGui::Button(ICON_FA_ARROW_DOWN " Make Copy " ICON_FA_ARROW_DOWN)) {
							int result = ARVT::copy_file(pdata.evaluated_input_split_1, pdata.evaluated_input_split_2);
							if (result) {
								strcpy(pdata.input_split_2_data, "error copying");
								global_state.input_split_2_data_is_bad = true;
								global_log.AddLog("[error]", "Copy", "Encountered an error when copying a file");
							} else {
								global_log.AddLog("[info]", "Copy", ("Successfully copied to " + std::string(pdata.evaluated_input_split_2)).c_str());
								int result = ARVT::copyFileToCStr(pdata.evaluated_input_split_2, pdata.input_split_2_data, IM_ARRAYSIZE(pdata.input_split_2_data));
								if (result) {
									strcpy(pdata.input_split_2_data, "error reading");
									global_state.input_split_2_data_is_bad = true;
									global_log.AddLog("[warn]", "File Preview", "Could not preview the file");
								} else {
									global_state.input_split_2_data_is_bad = false;
								}
							}
						}

						ImGui::SeparatorText("Speech Text (optional)");

						ImGui::Checkbox("Use Speech Text", &vdata.use_speech_text); //don't clear when the lock is clicked, could be locked/unlocked many times

						if (!vdata.use_speech_text) {
							ImGui::BeginDisabled();
							//pushes to disabled stack
						}

						ImGui::InputText("##Input Split 2 Path", pdata.evaluated_input_split_2, IM_ARRAYSIZE(pdata.evaluated_input_split_2), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);
						ImGui::SameLine();
						if (ImGui::Button("Preview File##Input Split 2")) {
							int result = ARVT::copyFileToCStr(pdata.evaluated_input_split_2, pdata.input_split_2_data, IM_ARRAYSIZE(pdata.input_split_2_data));
							if (result) {
								strcpy(pdata.input_split_2_data, "error reading");
								global_state.input_split_2_data_is_bad = true;
								global_log.AddLog("[warn]", "File Preview", "Could not preview the file");
							} else {
								global_state.input_split_2_data_is_bad = false;
							}
						}

						if (global_state.input_split_2_data_is_bad) { ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.0f, 0.0f, 1.0f }); }
						ImGui::InputTextMultiline("##Input Split 2 Data", pdata.input_split_2_data, IM_ARRAYSIZE(pdata.input_split_2_data), ImVec2(-FLT_MIN, 0), ImGuiInputTextFlags_ReadOnly);
						if (global_state.input_split_2_data_is_bad) { ImGui::PopStyleColor(); }
						if (ImGui::Button("Reveal in File Explorer##Input Split 2")) {
							int result = ARVT::revealFileExplorer(pdata.evaluated_input_split_2, pdata);
							if (result) {
								global_log.AddLog("[warn]", "File Explorer", "Some error");
							}
						}

						if (!vdata.use_speech_text) {
							ImGui::EndDisabled();
							//pops from disabled stack
						}

						if (!filenameIsLocked) { ImGui::EndDisabled(); }

						ImGui::TableNextColumn();

						if (!filenameIsLocked) { ImGui::BeginDisabled(); }

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::SeparatorText("Image");
						ImGui::InputText("Image Width",     idata.image_width_input,    IM_ARRAYSIZE(idata.image_width_input),    ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);
						ImGui::InputText("Image Height",    idata.image_height_input,   IM_ARRAYSIZE(idata.image_height_input),   ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);
						ImGui::InputText("Width Border",    idata.image_w_border_input, IM_ARRAYSIZE(idata.image_w_border_input), ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);
						ImGui::InputText("Height Border",   idata.image_h_border_input, IM_ARRAYSIZE(idata.image_h_border_input), ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);

						ImGui::SeparatorText("Font");
						ImGui::InputText("Font Size",             idata.font_size_input,           IM_ARRAYSIZE(idata.font_size_input),        ImGuiInputTextFlags_CallbackCharFilter, numberOnlyPositiveFunc);
						ImGui::InputText("Font Color",            idata.font_color_input,          IM_ARRAYSIZE(idata.font_color_input),       ImGuiInputTextFlags_CallbackCharFilter, filenameCleaningFunc);
						ImGui::InputText("Background Color",      idata.background_color_input,    IM_ARRAYSIZE(idata.background_color_input), ImGuiInputTextFlags_CallbackCharFilter, filenameCleaningFunc);
						ImGui::SliderScalar("Newline Count", ImGuiDataType_U8, &idata.paragraph_newline_v, &idata.paragraph_newline_min, &idata.paragraph_newline_max);
						ImGui::Checkbox("Paragraph Tabbed Start", &idata.paragraph_tabbed_start_input);

						const bool opened_additional_options_test_image = ImGui::TreeNodeEx("Additional Options##Test Image", ImGuiTreeNodeFlags_FramePadding);
						if (opened_additional_options_test_image) {
							ImGui::Unindent(style.IndentSpacing); //TODO: why isn't ImGui::GetTreeNodeToLabelSpacing() correct? //TODO: it seems to align it with the tree node's parent, which is weird (requires DPI=1)

							ImGui::InputText("Font Name",             idata.font_name,                 IM_ARRAYSIZE(idata.font_name),              ImGuiInputTextFlags_CallbackCharFilter, filenameCleaningFunc);
							ImGui::Indent();
							ImGui::Checkbox("Font is a family",       &idata.font_is_family_input);
							ImGui::SameLine();
							ImGuiHelpers::HelpMarker("Every individual font has its own name, but they're often grouped under a family.\n"
							                         "Bold/Italic/Bold+Italic/SemiLight/whatever versions of the font are part of the font's family.\n");
							ImGui::Unindent();

							ImGui::Combo("Text Alignment", &idata.textAlignmentArray_current, idata.textAlignmentArray.data(), idata.textAlignmentArray.size());
							ImGui::Checkbox("Skip line with lone '\\n'", &idata.skip_lone_lf_input);
							ImGui::SameLine();
							ImGuiHelpers::HelpMarker("If a line only has \"\\n\" on it, this will skip it, allowing an extra line with nothing on it.");

							ImGui::Indent(style.IndentSpacing);
							ImGui::TreePop();
						}
						const int test_image_font_item_count = 7 + (opened_additional_options_test_image ? 4 : 0);
						ImGui::PopItemWidth();

						ImGui::SeparatorText("Export");
						ImGui::Checkbox("Replace ImageMagick escape sequences", &idata.replace_magick_escape_sequences);
						if (ImGui::BeginItemTooltip()) {
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							ImGui::TextUnformatted("Replaces '&', '<', '>' with '&amp;', '&lt;', '&gt;'");
							ImGui::PopTextWrapPos();
							ImGui::EndTooltip();
						}

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::Combo("Image Format", &idata.imageFormatArray_current, idata.imageFormatArray.data(), idata.imageFormatArray.size());
						ImGui::PopItemWidth();
						ARVT::copyEvaluatedFileName_toCommentTestImagePath_TestImage(pdata, idata);
						ImGui::InputText("##Test Image Path", pdata.evaluated_test_image_path, IM_ARRAYSIZE(pdata.evaluated_test_image_path), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);

						if (ImGui::Button("Create " ICON_FA_ARROW_RIGHT, ImVec2(-FLT_MIN, 0.0f))) {
							int result = ARVT::call_comment_test_image(pdata, idata);
							if (result) {
								global_log.AddLog("[error]", "Image", "Encountered an error when making the test image");
								global_state.createdTestImage_texture = global_state.createdTestImage_width = global_state.createdTestImage_height = 0;
							} else {
								global_log.AddLog("[info]", "Image", ("Successfully created test image " + std::string(pdata.evaluated_test_image_path)).c_str());
								ret = ImGuiHelpers::LoadTextureFromFile(pdata.evaluated_test_image_path, &global_state.createdTestImage_texture, &global_state.createdTestImage_width, &global_state.createdTestImage_height);
								if (!ret) {
									global_log.AddLog("[error]", "Image", "Could not load test image");
									global_state.createdTestImage_texture = global_state.createdTestImage_width = global_state.createdTestImage_height = 0;
								}
							}
						}
						lock_filename_tooltip(filenameIsLocked);

						ImGui::SeparatorText("Video Settings (optional)");
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::InputText("Video Replacement", vdata.video_replacement_numbers_input, IM_ARRAYSIZE(vdata.video_replacement_numbers_input), ImGuiInputTextFlags_CallbackCharFilter, video_replacement_scrubbingFunc);
						ImGui::SameLine();
						ImGuiHelpers::HelpMarker("ex. \"1,2,3\" or \"4,6-8,15,20-30\"\n"
						                         "\"-3\" is start to 3; \"30-\" is 30 to end");
						ImGui::PopItemWidth();
						ImGui::Checkbox("Audio Only", &vdata.audio_only_option_input);

						ARVT::copyEvaluatedFileName_toCommentToSpeechPath(pdata, vdata);
						ImGui::InputText("##Output Videos Path", pdata.evaluated_output_speech_path, IM_ARRAYSIZE(pdata.evaluated_output_speech_path), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);

						if (!filenameIsLocked) { ImGui::EndDisabled(); }

						ImGui::TableNextColumn();

						/* NOTE: ImGui currently doesn't have a way to calculate
						 * the available height *before* the content has been
						 * rendered. One idea is to skip to the next row then
						 * jump back, but that's not supported. So it seems the
						 * only way to get the available height for scaling the
						 * image is to manually calculate the height of the
						 * columns and compare that against the true remaining
						 * space in the window.
						 */
						//sizing: https://github.com/ocornut/imgui/issues/3714

						float columnHeights[3];
						columnHeights[0] = 2 * ImGui::GetFrameHeightWithSpacing() + ImGuiHelpers::getMultilineInputHeight(ImGui::GetTextLineHeight() * 16) + no_voice_height;
						columnHeights[1] = 7 * ImGui::GetFrameHeightWithSpacing() + 2 * ImGuiHelpers::getMultilineInputHeight(0);
						columnHeights[2] = (5 + test_image_font_item_count + 5 + 4) * ImGui::GetFrameHeightWithSpacing();

						const float largestColumn = *std::max_element(columnHeights, columnHeights + IM_ARRAYSIZE(columnHeights));
						const float contentAvailableY = ImGui::GetContentRegionAvail().y + 2*style.ItemSpacing.y;
						const float availableHeight = std::max(0.0f, std::min(largestColumn, contentAvailableY));
						const float availableWidth = std::max(0.0f, ImGui::GetContentRegionAvail().x);

						if (THREAD_IS_WORKING) { ImGui::EndDisabled(); }

						if ((availableWidth / global_state.createdTestImage_width) * global_state.createdTestImage_height > availableHeight) {
							if (availableHeight > global_state.createdTestImage_height) {
								ImGui::Image((ImTextureID)(intptr_t)global_state.createdTestImage_texture, ImVec2(global_state.createdTestImage_width, global_state.createdTestImage_height));
							} else {
								ImGui::Image((ImTextureID)(intptr_t)global_state.createdTestImage_texture, ImVec2((availableHeight / global_state.createdTestImage_height) * global_state.createdTestImage_width, availableHeight));
							}
						} else {
							if (availableWidth > global_state.createdTestImage_width) {
								ImGui::Image((ImTextureID)(intptr_t)global_state.createdTestImage_texture, ImVec2(global_state.createdTestImage_width, global_state.createdTestImage_height));
							} else {
								ImGui::Image((ImTextureID)(intptr_t)global_state.createdTestImage_texture, ImVec2(availableWidth, (availableWidth / global_state.createdTestImage_width) * global_state.createdTestImage_height));
							}
						}

						if (THREAD_IS_WORKING) { ImGui::BeginDisabled(); }

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						if (!filenameIsLocked) { ImGui::BeginDisabled(); }

						if (ImGui::Button("Split!", ImVec2(-FLT_MIN, 0.0f))) {
							int result = ARVT::call_comment_splitter(pdata);
							if (result) {
								strcpy(pdata.input_split_1_data, "error");
								global_state.input_split_1_data_is_bad = true;
								global_log.AddLog("[error]", "Splitter", "Encountered an error when splitting the input file");
							} else {
								global_log.AddLog("[info]", "Splitter", ("Successfully split " + std::string(pdata.evaluated_input_file_name)).c_str());
								int result = ARVT::copyFileToCStr((pdata.get_input_splits_path() + ARVT::inputFileName_toCommentTestImageName_Text(pdata.the_file_input_name)).c_str(), pdata.input_split_1_data, IM_ARRAYSIZE(pdata.input_split_1_data));
								if (result) {
									strcpy(pdata.input_split_1_data, "error reading");
									global_state.input_split_1_data_is_bad = true;
									global_log.AddLog("[warn]", "File Preview", "Could not preview the file");
								} else {
									global_state.input_split_1_data_is_bad = false;
								}
							}
						}
						lock_filename_tooltip(filenameIsLocked);

						if (!filenameIsLocked) { ImGui::EndDisabled(); }

						ImGui::TableNextColumn();
						ImGui::TableNextColumn();

						if (!filenameIsLocked) { ImGui::BeginDisabled(); }

						const float write_settings_width = ImGui::GetContentRegionAvail().x * .75f;
						if (ImGui::BeginPopup("Settings")) {
							ARVT::copyEvaluatedFileName_toVideoSettingsPath(pdata);
							ImGui::PushItemWidth(write_settings_width);
							ImGui::InputText("##Settings File Path", pdata.evaluated_video_settings_path, IM_ARRAYSIZE(pdata.evaluated_video_settings_path), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);
							ImGui::PopItemWidth();

							if (ImGui::Button("Write Current Settings", ImVec2(write_settings_width, 0.0f))) {
								mINI::INIFile video_settings_file(pdata.evaluated_video_settings_path);
								mINI::INIStructure video_settings_object;

								ARVT::CopySettingsToIni(video_settings_object, idata, adata, vdata);
								bool result = video_settings_file.generate(video_settings_object, true);
								//don't bother with a "confirm" box before actually writing the file if it already exists

								if (!result) {
									global_log.AddLog("[error]", "Settings", "Encountered an error when writing settings");
								} else {
									global_log.AddLog("[info]", "Settings", ("Successfully wrote " + std::string(pdata.evaluated_video_settings_path)).c_str());
								}
							}

							if (ImGui::Button("Import Settings", ImVec2(write_settings_width, 0.0f))) {
								mINI::INIFile video_settings_file(pdata.evaluated_video_settings_path);
								mINI::INIStructure video_settings_object;
								bool result = video_settings_file.read(video_settings_object);
								//TODO: shouldn't this reset unset settings to default?

								if (!result) {
									global_log.AddLog("[error]", "Settings", "Encountered an error when reading settings");
								} else {
									ARVT::Fill_ImageData(idata, video_settings_object);
									ARVT::Fill_AudioData(adata, video_settings_object, pdata.useExtraCodecs);
									ARVT::Fill_VideoData(vdata, video_settings_object, pdata.useExtraCodecs);
									//if the user changed pdata.useExtraCodecs, that's their fault
									global_log.AddLog("[info]", "Settings", ("Successfully loaded " + std::string(pdata.evaluated_video_settings_path)).c_str());
								}
							}

							if (ImGui::Button("Reveal in File Explorer##Video Settings", ImVec2(write_settings_width, 0.0f))) {
								int result = ARVT::revealFileExplorer(pdata.evaluated_video_settings_path, pdata);
								if (result) {
									global_log.AddLog("[warn]", "File Explorer", "Some error");
								}
							}

							ImGui::EndPopup();
						}

						const float button_width = ImGui::GetContentRegionAvail().x / 2; //ImGui::GetContentRegionAvail() changes after changing the cursor position
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x / 4);
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetFrameHeightWithSpacing() / 2);
						if (ImGui::Button("Settings", { button_width, ImGui::GetFrameHeightWithSpacing() })) {
							ImGui::OpenPopup("Settings");
						}
						lock_filename_tooltip(filenameIsLocked);

						if (!filenameIsLocked) { ImGui::EndDisabled(); }

						ImGui::TableNextColumn();

						if (THREAD_IS_WORKING) { ImGui::EndDisabled(); }

						if (!filenameIsLocked) { ImGui::BeginDisabled(); }

						if (THREAD_IS_WORKING) {
							ImGui::ProgressBar(-0.75f * (float)ImGui::GetTime(), ImVec2(-FLT_MIN, 0.0f), "Running...");
						} else {
							if (adata.voiceArray_current < 0) { ImGui::BeginDisabled(); }
							if (ImGui::Button("GO!", ImVec2(-FLT_MIN, 0.0f))) {
								// Note: std::async(std::launch::async, ...) will not work because
								// std::future will get destructed here and that seems to call .join()
								// which is blocking, defeating the whole point of separate threads.
								// Using std::async(std::launch::deferred, ...) will theoretically
								// solve that issue, however it didn't seem to actually start, or it
								// crashed. Therefore, the solution is to not use std::async and
								// instead just use std::thread (not std::jthread because the threads
								// here are created on the stack and thus immediately destructed, and
								// std::jthread destruction calls .join()), followed by .detach() to
								// avoid the std::terminate on destruction.

								thread_func_speech_working.store(true);
								std::thread t(thread_func_speech, &pdata, &idata, &adata, &vdata);
								t.detach();

								// Technically checking thread_func_speech_working before this store
								// is race-y... but I don't think ImGui can handle multiple clicks at
								// different areas on the same frame, so no problem (unless
								// touchscreens are different?)
							}
							lock_filename_tooltip(filenameIsLocked);
							if (adata.voiceArray_current < 0) { ImGui::EndDisabled(); }
						}

						if (ImGui::Button("Reveal in File Explorer##Final Video", ImVec2(-FLT_MIN, 0.0f))) {
							//yes it's *kinda* a hack to open on just the first video, but it's better than iterating through every file in the folder and checking what's available
							int result = ARVT::revealFileExplorer_folderBackup((pdata.get_output_speech_path() + ARVT::inputFileName_toCommentToSpeechName_getFileExplorerName(pdata.the_file_input_name, vdata.videoContainerArray[vdata.videoContainerArray_current], vdata.audio_only_option_input)).c_str(), pdata);
							if (result) {
								global_log.AddLog("[warn]", "File Explorer", "Some error");
							}
						}

						if (!filenameIsLocked) { ImGui::EndDisabled(); }

						ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 1.0f);
						ImGui::EndTable();
						ImGui::PopStyleVar();
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Configure", nullptr, tab_flags[1])) {

					if (THREAD_IS_WORKING) { ImGui::BeginDisabled(); }

					if (ImGui::BeginTable("Configure##table1", 4, table_flags)) {
						ImGui::TableSetupColumn("Audio");
						ImGui::TableSetupColumn("Video");
						ImGui::TableSetupColumn("Paths");
						ImGui::TableSetupColumn("Other");
						ImGui::TableHeadersRow();
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						if (ImGui::Combo("Speech Engine", &adata.speechEngineArray_current, adata.speechEngineArray.data(), adata.speechEngineArray.size())) {
							//there was a change
							if (adata.update_voiceArray()) {
								global_log.AddLog("[error]", "Speech Engine", "Could not update voice list");
								//TODO: more specific message (like the engine not existing)
							}
						}

						ImGui::Indent();
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.2f);
						ImGui::InputText("Voice Language", adata.speech_language_input, IM_ARRAYSIZE(adata.speech_language_input), ImGuiInputTextFlags_CallbackCharFilter, filenameCleaningFunc);
						ImGui::SameLine();
						ImGuiHelpers::HelpMarker("Not all speech engines support filtering by language.\nCan leave this blank.");
						ImGui::PopItemWidth();

						#if 0
						const bool opened_advanced_options_speech_engine = ImGui::TreeNodeEx("Advanced Speech Options##Speech Engine", ImGuiTreeNodeFlags_FramePadding);
						if (opened_advanced_options_speech_engine) {
							//TODO: checkmark before each one to enable it
							ImGui::Unindent(style.IndentSpacing);
							//balcon:
							ImGui::SliderScalar("-s Speech Rate", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //-10 to 10
							ImGui::SliderScalar("-p Speech Pitch", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //-10 to 10
							ImGui::SliderScalar("-v Volume", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //0 to 100
							ImGui::SliderScalar("-e Pause between sentences (ms)", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max);
							//ImGui::SliderScalar("-a Pause between paragraphs (ms)", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max);
							ImGui::SliderScalar("-sb Pause at the beginning (ms)", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max);
							ImGui::SliderScalar("-se Pause at the end (ms)", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max);
							ImGui::SliderScalar("-fr Frequency", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //8 to 48
							//others: -bt audio bit depth (8 or 16), ignore text options

							//espeak-ng:
							ImGui::SliderScalar("-a Amplitude", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //0 to 200 default 100
							ImGui::SliderScalar("-g Pause between words (10ms)", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max);
							ImGui::SliderScalar("-p Pitch adjustment", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //0 to 99 default 50
							ImGui::SliderScalar("-P Pitch range adjustment", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //0 to 99 default 50
							ImGui::SliderScalar("-s WPM", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //default 175
							//others: -z no final sentence pause at end of text

							//wsay:
							ImGui::SliderScalar("--P, --pitch <value>", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //0 to 20 default 10
							ImGui::SliderScalar("-s, --speed <value>", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //0 to 100 default 50
							ImGui::SliderScalar("-V, --volume <value>", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //0 to 100
							ImGui::SliderScalar("--fxradio <value>", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //1 to 6
							ImGui::SliderScalar("--fxradio_nonoise", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max);
							ImGui::SliderScalar("--paragraph_pause <value>", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max); //0 to big
							//others: --nospeechxml

							ImGui::Indent(style.IndentSpacing);
							ImGui::TreePop();
						}
						#endif

						ImGui::Unindent();

						const bool voiceArray_unset = (adata.voiceArray_current < 0);
						if (voiceArray_unset) {
							ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
							ImGui::PushStyleColor(ImGuiCol_Border, { 1.0f, 0.0f, 0.0f, 1.0f });
						}
						ImGui::Combo("Voice", &adata.voiceArray_current, adata.voiceArray, adata.voiceArray_length);
						if (voiceArray_unset) {
							ImGui::PopStyleColor();
							ImGui::PopStyleVar();
						}

						ImGui::SameLine();
						if (ImGui::Button("Refresh")) {
							if (adata.update_voiceArray()) {
								global_log.AddLog("[error]", "Speech Engine", "Could not update voice list");
							}
						}

						if (ImGui::BeginCombo("Audio Encoder", adata.get_audioEncoder()->displayName)) {
							for (int n = 0; n < AudioData::get_audioEncoderArraySize(pdata.useExtraCodecs); n++) {
								if (ImGui::Selectable(AudioData::audioEncoderArrayExtended[n]->displayName, adata.audioEncoder_idx == n)) {
									adata.set_encoder_idx(n);
								}
							}
							ImGui::EndCombo();
						}
						const AudioCodecData* ac = adata.get_audioEncoder();

						ImGui::Indent();

						ImGui::Text("Recommendation:");
						ImGui::SameLine();
						ImGui::Image(recommendationStr_toTexId[ac->recommendation], ImageButtonSize);

						ImGui::Text("Information:");
						ImGui::SameLine();
						ImGuiHelpers::HelpMarker(ac->information_text);

						if (!ac->preset1.displayValues.empty()) {
							const GenericCodecPreset& ac_p1 = ac->preset1;
							if (ImGui::BeginCombo(ac_p1.displayTerm, ac_p1.displayValues[adata.audioEncoder_preset1_idx])) {
								for (int n = 0; n < ac_p1.displayValues.size(); n++) {
									if (ImGui::Selectable(ac_p1.displayValues[n], adata.audioEncoder_preset1_idx == n)) {
										adata.set_encoder_preset1_idx(n);
									}
								}
								ImGui::EndCombo();
							}
						}

						if (!ac->isLossless) {
							//ImGui doesn't support steps for sliders, oh well
							ImGui::SliderScalar("Bitrate (kbps)", ImGuiDataType_U16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max);
						}

						ImGui::Unindent();

						ImGui::TableNextColumn();

						ImGui::Checkbox("Custom FPS", &vdata.fractionalFps);
						ImGui::Indent();
						if (vdata.fractionalFps) {
							ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
							ImGui::InputText("##FPS Numerator",      vdata.fps_numerator_input,   IM_ARRAYSIZE(vdata.fps_numerator_input),   ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);
							ImGui::SameLine();
							ImGui::Text(" / ");
							ImGui::SameLine();
							ImGui::InputText("FPS##FPS Denominator", vdata.fps_denominator_input, IM_ARRAYSIZE(vdata.fps_denominator_input), ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);
							ImGui::PopItemWidth();
						} else {
							ImGui::Combo("FPS##Integer", &vdata.fpsArray_current, vdata.fpsArray.data(), vdata.fpsArray.size(), vdata.fpsArray.size());
						}
						ImGui::Unindent();

						if (ImGui::BeginCombo("Video Encoder", vdata.get_videoEncoder()->displayName)) {
							for (int n = 0; n < VideoData::get_videoEncoderArraySize(pdata.useExtraCodecs); n++) {
								if (ImGui::Selectable(VideoData::videoEncoderArrayExtended[n]->displayName, vdata.videoEncoder_idx == n)) {
									vdata.set_encoder_idx(n);
								}
							}
							ImGui::EndCombo();
						}
						const VideoCodecData* vc = vdata.get_videoEncoder();

						ImGui::Indent();

						ImGui::Text("Recommendation:");
						ImGui::SameLine();
						ImGui::Image(recommendationStr_toTexId[vc->recommendation], ImageButtonSize);

						ImGui::Text("Information:");
						ImGui::SameLine();
						ImGuiHelpers::HelpMarker(vc->information_text);
						ImGui::SameLine();
						ImGui::Text(vc->supportsAlpha ? "  Alpha channel: Yes" : "  Alpha channel: No");

						if (!vc->preset1.displayValues.empty()) {
							const GenericCodecPreset& vc_p1 = vc->preset1;
							if (ImGui::BeginCombo(vc_p1.displayTerm, vc_p1.displayValues[vdata.videoEncoder_preset1_idx])) {
								for (int n = 0; n < vc_p1.displayValues.size(); n++) {
									if (ImGui::Selectable(vc_p1.displayValues[n], vdata.videoEncoder_preset1_idx == n)) {
										vdata.set_encoder_preset1_idx(n);
									}
								}
								ImGui::EndCombo();
							}
							if (!vc->preset2.displayValues.empty()) {
								const GenericCodecPreset& vc_p2 = vc->preset2;
								if (ImGui::BeginCombo(vc_p2.displayTerm, vc_p2.displayValues[vdata.videoEncoder_preset2_idx])) {
									for (int n = 0; n < vc_p2.displayValues.size(); n++) {
										if (ImGui::Selectable(vc_p2.displayValues[n], vdata.videoEncoder_preset2_idx == n)) {
											vdata.set_encoder_preset2_idx(n);
										}
									}
									ImGui::EndCombo();
								}
							}
						}

						if (!vc->isLossless) {
							ImGui::SliderScalar("CRF", ImGuiDataType_U8, &vdata.video_crf_v, &vdata.video_crf_min, &vdata.video_crf_max);
							//ImGui::SameLine();
							//ImGuiHelpers::HelpMarker("CTRL+Click to input a value.");
						}

						ImGui::Unindent();

						//TODO: think about non-wav containers for audio-only mode
						ImGui::Combo("Container", &vdata.videoContainerArray_current, vdata.videoContainerArray.data(), vdata.videoContainerArray.size());

						ImGui::Indent();
						if (!vdata.get_faststart_available()) { ImGui::BeginDisabled(); }
						ImGui::Checkbox("-movflags=+faststart", &vdata.faststart_flag);
						ImGui::SameLine();
						ImGuiHelpers::HelpMarker("Moves the \"start\" information from the end to the front.\n"
						                         "Makes starting playing the video faster but unnecessary for this program's typical usage.");
						if (!vdata.get_faststart_available()) { ImGui::EndDisabled(); }
						ImGui::Unindent();

						ImGui::TableNextColumn();

						ImGui::Checkbox("Enable custom paths", &pdata.useCustomPaths);
						if (ImGui::Button("Reset##Main Paths")) {
							pdata.ResetFilePaths();
						}
						ImGuiHelpers::HelpMarker("Paths MUST end in '/'.");

						// These should probably be disabled when the lock icon is locked...
						if (!pdata.useCustomPaths) { ImGui::BeginDisabled(); }
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
						ImGui::InputText("Input Comments",        pdata.input_comments_path, IM_ARRAYSIZE(pdata.input_comments_path), ImGuiInputTextFlags_CallbackCharFilter, filepathCleaningFunc);
						ImGui::InputText("Splits",                pdata.input_splits_path,   IM_ARRAYSIZE(pdata.input_splits_path),   ImGuiInputTextFlags_CallbackCharFilter, filepathCleaningFunc);
						ImGui::InputText("Test Images",           pdata.test_images_path,    IM_ARRAYSIZE(pdata.test_images_path),    ImGuiInputTextFlags_CallbackCharFilter, filepathCleaningFunc);
						ImGui::InputText("Output Videos",         pdata.output_speech_path,  IM_ARRAYSIZE(pdata.output_speech_path),  ImGuiInputTextFlags_CallbackCharFilter, filepathCleaningFunc);
						ImGui::InputText("Video Settings",        pdata.video_settings_path, IM_ARRAYSIZE(pdata.video_settings_path), ImGuiInputTextFlags_CallbackCharFilter, filepathCleaningFunc);
						ImGui::InputText("Temp files (optional)", pdata.temporary_file_path, IM_ARRAYSIZE(pdata.temporary_file_path), ImGuiInputTextFlags_CallbackCharFilter, filepathCleaningFunc);
						ImGui::PopItemWidth();
						if (!pdata.useCustomPaths) { ImGui::EndDisabled(); }

						ImGui::TableNextColumn();

						ImGui::SeparatorText("Application");

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
						//see ImGui::ShowFontSelector()
						{
							ImFont* font_current = ImGui::GetFont();
							if (ImGui::BeginCombo("Font Selector", font_current->GetDebugName()))
							{
								for (ImFont* font : io.Fonts->Fonts) {
									ImGui::PushID((void*)font);
									if (ImGui::Selectable(font->GetDebugName(), font == font_current)) {
										newFontToSwitchTo = font;
										needToChangeFonts = true;
									}
									if (font == font_current) {
										ImGui::SetItemDefaultFocus();
									}
									ImGui::PopID();
								}
								ImGui::EndCombo();
							}
						}
						if (ImGui::DragFloat("Font Size", &pdata.application_font_size, 0.20f, 8.0f, 60.0f, "%.0f")) { // 20 is the "default" size, <=0 not accepted
							refreshApplicationFontSize();
						}
						ImGui::InputText("New Font Path", pdata.application_font_path, IM_ARRAYSIZE(pdata.application_font_path), ImGuiInputTextFlags_CallbackCharFilter, filepathCleaningFunc);
						#ifdef _WIN32
						ImGui::SameLine();
						ImGuiHelpers::HelpMarker("C:\\Windows\\Fonts does not have folders in it!\n"
						                         "For example, Noto Sans is not actually \"Noto Sans\\NotoSans-Regular.ttf\",\n"
						                         "but just \"NotoSans-Regular.ttf\". Windows Explorer is lying to you!");
						#endif
						ImGui::SameLine();
						if (ImGui::Button("Load##Font Path")) {
							needToChangeFonts = true;
						}
						ImGui::PopItemWidth();

						ImGui::ColorEdit3("Background Color", (float*)&pdata.background_color);
						ImGui::ColorEdit4("Window Color", (float*)&style.Colors[ImGuiCol_WindowBg]);

						if (ImGui::Checkbox("Extra Codecs", &pdata.useExtraCodecs)) {
							if (!pdata.useExtraCodecs) {
								if (adata.audioEncoder_idx >= AudioData::get_audioEncoderArraySize(false)) {
									adata.set_encoder_idx(0);
								}
								if (vdata.videoEncoder_idx >= VideoData::get_videoEncoderArraySize(false)) {
									vdata.set_encoder_idx(0);
								}
							}
						}

						ImGui::SeparatorText("Misc");

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
						ImGui::Combo("Python Command", &pdata.pythonCmdArray_current, pdata.pythonCmdArray.data(), pdata.pythonCmdArray.size());
						#ifdef _WIN32
						//explorer.exe is enough
						#else
						ImGui::Combo("File Explorer", &pdata.fileExplorerCmdArray_current, pdata.fileExplorerCmdArray.data(), pdata.fileExplorerCmdArray.size());
						#endif
						ImGui::PopItemWidth();

						#if _DEBUG
						ImGui::Checkbox("Demo Window", &show_demo_window);
						#endif

						ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 1.0f);
						ImGui::EndTable();
						ImGui::PopStyleVar();
					}

					if (ImGui::BeginTable("Configure##table2", 2, table_flags | ImGuiTableFlags_ScrollY, { 0.0f, 10.0f * ImGui::GetFrameHeightWithSpacing() })) {
						ImGui::TableSetupColumn("Delete Old Files");
						ImGui::TableSetupColumn("##Logger");
						ImGui::TableHeadersRow();
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						ImGui::SeparatorText("Query");

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::Combo("File Delete Age", &pdata.fileDeleteAgeList_current, pdata.fileDeleteAgeList.data(), pdata.fileDeleteAgeList.size());
						ImGui::PopItemWidth();

						if (ImGui::Button("Query images")) {
							deleteFileList.clear();
							deleteFileLogger.Clear();
							int result = ARVT::getListOfOldFiles(pdata.get_test_images_path().c_str(), pdata.fileDeleteAgeList_values[pdata.fileDeleteAgeList_current], deleteFileList);
							if (result) {
								global_log.AddLog("[error]", "Query Files", "Encountered an error when querying images");
							} else {
								global_log.AddLog("[info]", "Query Files", ("Successfully queried " + pdata.get_test_images_path()).c_str());
								deleteFileLogger.AddLogSpecific("Found %d files that are more than %s old:\n", deleteFileList.size(), pdata.fileDeleteAgeList[pdata.fileDeleteAgeList_current]);
								for (const auto& f : deleteFileList) {
									deleteFileLogger.AddLogSpecific("%s\n", f.c_str());
								}
							}
						}
						//ImGui::SameLine();
						if (ImGui::Button("Query videos")) {
							deleteFileList.clear();
							deleteFileLogger.Clear();
							int result = ARVT::getListOfOldFiles(pdata.get_output_speech_path().c_str(), pdata.fileDeleteAgeList_values[pdata.fileDeleteAgeList_current], deleteFileList);
							if (result) {
								global_log.AddLog("[error]", "Query Files", "Encountered an error when querying videos");
							} else {
								global_log.AddLog("[info]", "Query Files", ("Successfully queried " + pdata.get_output_speech_path()).c_str());
								deleteFileLogger.AddLogSpecific("Found %d files that are more than %s old:\n", deleteFileList.size(), pdata.fileDeleteAgeList[pdata.fileDeleteAgeList_current]);
								for (const auto& f : deleteFileList) {
									deleteFileLogger.AddLogSpecific("%s\n", f.c_str());
								}
							}
						}

						if (ImGui::Button("Clear queried files")) {
							deleteFileList.clear();
							deleteFileLogger.Clear();
						}

						ImGui::SeparatorText("Delete");

						// Only allowed to delete what's in the list: if something new suddenly meets the cutoff, it's spared; if something old gets its modified time changed, it's gone
						if (ImGui::BeginPopup("Delete queried files")) {
							//centering the text or buttons:
							//const float text_size = ImGui::CalcTextSize("Are you sure? This cannot be undone.").x;
							//const float buttons_size = 4*style.FramePadding.x + style.ItemSpacing.x + ImGui::CalcTextSize("No, take me back!").x + ImGui::CalcTextSize("Yes, I understand").x;

							ImGui::TextColored(ImVec4(1, 0, 0, 1), "Are you sure? This cannot be undone.");

							if (ImGui::Button("No, take me back!")) {
								ImGui::CloseCurrentPopup();
							}
							ImGui::SameLine();
							if (ImGui::Button("Yes, I understand")) {
								int result = ARVT::deleteAllOldFiles(deleteFileList);
								if (result) {
									global_log.AddLog("[error]", "Delete Files", "Encountered an error when deleting files");
								} else {
									deleteFileLogger.Clear();
									global_log.AddLog("[info]", "Delete Files", ("Successfully deleted " + std::to_string(deleteFileList.size()) + " files").c_str());
								}
								deleteFileList.clear();
								ImGui::CloseCurrentPopup();
							}
							ImGui::EndPopup();
						}

						// Check the logger's data instead of deleteFileList so it's possible to "delete" 0 files
						const bool deleteFileLogger_empty = deleteFileLogger.Buf.empty();
						if (deleteFileLogger_empty) { ImGui::BeginDisabled(); }
						if (ImGui::Button("Delete queried files")) {
							ImGui::OpenPopup("Delete queried files");
						}
						if (deleteFileLogger_empty) { ImGui::EndDisabled(); }

						ImGui::TableNextColumn();

						deleteFileLogger.Draw();

						//ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 1.0f);
						ImGui::EndTable();
						//ImGui::PopStyleVar();
					}

					if (THREAD_IS_WORKING) { ImGui::EndDisabled(); }

					ImGui::EndTabItem();
				}

				/*
				if (ImGui::BeginTabItem("SSH", nullptr, tab_flags[2])) {
					ImGui::Text("TODO");
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Help", nullptr, tab_flags[2])) {
					ImGui::Text("Requirements: TODO");
					//TODO: list detected programs
					ImGui::EndTabItem();
				}
				*/

				if (ImGui::BeginTabItem("About", nullptr, tab_flags[2])) {
					ImGui::Text("License: GNU General Public License v3.0");
					ImGui::Text("SPDX-License-Identifier: GPL-3.0-only");
					ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
					ImGui::TextLinkOpenURL("GitHub link", "https://github.com/khuiqel/automated-reddit-video-tool-gui");

					ImGui::NewLine();
					if (ImGui::TreeNodeEx("Extra Buttons", ImGuiTreeNodeFlags_FramePadding)) {
						if (ImGui::Button("Toggle window decorations (title bar & outline)")) {
							glfwSetWindowAttrib(window, GLFW_DECORATED, !glfwGetWindowAttrib(window, GLFW_DECORATED));
						}
						if (ImGui::Button("Maximize window")) {
							glfwMaximizeWindow(window);
						}
						if (ImGui::Button("Un-maximize window")) {
							glfwRestoreWindow(window);
						}
						if (ImGui::Button("Minimize window")) {
							glfwIconifyWindow(window);
						}
						ImGui::TreePop();
					}
					if (ImGui::Button("Close Application")) {
						glfwSetWindowShouldClose(window, GLFW_TRUE);
					}
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			ImGui::Separator();
			if (ImGui::BeginTable("Logging##table1", 2, ImGuiTableFlags_SizingStretchProp)) {
				//the "long text display" example is another option, but more utility is better for this situation
				//maybe use monospace font?

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				global_log.Draw();
				ImGui::TableNextColumn();
				global_log.Draw_Extras();

				ImGui::EndTable();
			}

			ImGui::End();
			ImGui::PopStyleVar(); //ImGuiStyleVar_WindowBorderSize
		}

		#if _DEBUG
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);
		#endif

		/* start ImGui code */

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(pdata.background_color.x * pdata.background_color.w, pdata.background_color.y * pdata.background_color.w, pdata.background_color.z * pdata.background_color.w, pdata.background_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

	/* end ImGui code */

    return 0;
}
