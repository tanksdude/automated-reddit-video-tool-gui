#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <stdio.h> //fprintf for glfwSetErrorCallback
#include <algorithm> //std::max_element
#include <cstring> //strerror, strcmp
#include <filesystem> //only for refreshApplicationFontName(), though plenty of other files also use <filesystem>

#include <GLFW/glfw3.h>
#define MINI_CASE_SENSITIVE
#include <mini/ini.h>

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
		global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Font", "Successfully loaded ProggyClean.ttf");
		return;
	}

	//TODO: should probably use ImFontFlags_NoLoadError instead, but this is good enough
	if (!std::filesystem::exists(pdata.application_font_path)) {
		global_log.AddLog("[%06.2fs] [error] %s: %s\n", ImGui::GetTime(), "Font", "Font does not exist");
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	ImFont* newFont = io.Fonts->AddFontFromFileTTF(pdata.application_font_path);
	if (newFont == nullptr) {
		//TODO: ImFontFlags_NoLoadError
		return;
	}
	io.FontDefault = newFont;

	//ideally this would be used but it doesn't change the font for some reason:
	//ImGui::PushFont(newFont, newSize);

	if (!loadingStartupFont) {
		global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Font", ("Successfully added font " + std::string(pdata.application_font_path)).c_str());
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
	    data->EventChar == '\"' || //TODO: should single quotes also be disallowed?
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
	//identical to filenameCleaningFunc but allows slashes (and colons)
	if (data->EventChar == '*'  ||
	    data->EventChar == '\"' ||
	    data->EventChar == '?'  ||
	    data->EventChar == '<'  ||
	    data->EventChar == '>'  ||
	    data->EventChar == '|')
		{ return 1; }
	return 0;
};
auto quoteAndSlashScrubbingFunc = [] (ImGuiInputTextCallbackData* data) {
	if (data->EventChar == '\"' || data->EventChar == '\'') { return 1; }
	if (data->EventChar == '\\' || data->EventChar == '/')  { return 1; }
	return 0;
};
auto video_replacement_scrubbingFunc = [] (ImGuiInputTextCallbackData* data) {
	if (data->EventChar >= '0' && data->EventChar <= '9') { return 0; }
	if (data->EventChar == ',' || data->EventChar == '-' || data->EventChar == ' ') { return 0; }
	return 1;
};

//called when the lock button is clicked
void clear_input_data(bool lockNewState) {
	if (lockNewState) {
		//now locked
		strcpy(pdata.input_comment_data, "");
		strcpy(pdata.input_split_1_data, "");
		strcpy(pdata.input_split_2_data, "");
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



static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
	ARVT::CreateDefaultIniIfNeeded("../arvt.ini");
	mINI::INIFile ini_file("../arvt.ini");
	mINI::INIStructure ini_object;
	ini_file.read(ini_object);

	ARVT::Fill_ProgramData(pdata, ini_object);
	ARVT::Fill_ImageData(idata, ini_object);
	ARVT::Fill_AudioData(adata, ini_object, pdata.useExtraCodecs);
	ARVT::Fill_VideoData(vdata, ini_object, pdata.useExtraCodecs);

	// If the user didn't set a speech engine in the INI, then fill its
	// available voices. (If the user did, then the values are already filled.)
	if (ini_object.get("AUDIO").get("SpeechEngine").empty()) {
		adata.update_voiceArray();
	}

	ARVT::CreateApplicationFoldersIfNeeded();

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

    // Create window with graphics context
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, pdata.application_scale_to_monitor ? GLFW_TRUE : GLFW_FALSE);
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow(pdata.initial_windowWidth, pdata.initial_windowHeight, "Automated Reddit Video Tool GUI v0.5.0", nullptr, nullptr);
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
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");

    // Our state
    bool show_demo_window = false;
	bool set_startup_tab = true;

	int createdTestImage_width = 0;
	int createdTestImage_height = 0;
	GLuint createdTestImage_texture = 0;
	bool ret;

	int lock_icon_width = 0;
	int lock_icon_height = 0;
	GLuint lock_icon_texture = 0;
	ret = ImGuiHelpers::LoadTextureFromFile("../res/locked_1f512.png", &lock_icon_texture, &lock_icon_width, &lock_icon_height);
	IM_ASSERT(ret);

	int unlock_icon_width = 0;
	int unlock_icon_height = 0;
	GLuint unlock_icon_texture = 0;
	ret = ImGuiHelpers::LoadTextureFromFile("../res/unlocked_1f513.png", &unlock_icon_texture, &unlock_icon_width, &unlock_icon_height);
	IM_ASSERT(ret);

	bool filenameIsLocked = false;

	GLuint recommended_awful, recommended_okay, recommended_good, recommended_best, recommended_noopinion;
	ret = ImGuiHelpers::LoadTextureFromFile("../res/cross-mark_274c.png", &recommended_awful, NULL, NULL);
	ret = ImGuiHelpers::LoadTextureFromFile("../res/warning_26a0-fe0f.png", &recommended_okay,  NULL, NULL);
	ret = ImGuiHelpers::LoadTextureFromFile("../res/heavy-check-mark_2714.png", &recommended_good,  NULL, NULL);
	ret = ImGuiHelpers::LoadTextureFromFile("../res/gem-stone_1f48e.png", &recommended_best,  NULL, NULL);
	ret = ImGuiHelpers::LoadTextureFromFile("../res/white-question-mark-ornament_2754.png", &recommended_noopinion, NULL, NULL);
	//wow, this is a bad emoji: ❎

	std::unordered_map<CodecRecommendedLevel, GLuint> recommendationStr_toTexId = {
		{ CodecRecommendedLevel::Awful, recommended_awful },
		{ CodecRecommendedLevel::Okay, recommended_okay },
		{ CodecRecommendedLevel::Good, recommended_good },
		{ CodecRecommendedLevel::Best, recommended_best },
		{ CodecRecommendedLevel::No_Opinion, recommended_noopinion },
	};

	GLuint return_symbol_texture, circle_arrows_texture;
	ret = ImGuiHelpers::LoadTextureFromFile("../res/leftwards-arrow-with-hook_21a9.png", &return_symbol_texture, NULL, NULL);
	ret = ImGuiHelpers::LoadTextureFromFile("../res/counterclockwise-arrows-button_1f504.png", &circle_arrows_texture, NULL, NULL);

	refreshApplicationFontSize();
	refreshApplicationFontName(true);
	style.Colors[ImGuiCol_WindowBg] = pdata.window_color; //TODO: have the color selector be for pdata.window_color, so the style is updated every frame

	global_log.AddLog("[%06.2fs] [info] %s\n", ImGui::GetTime(), "Startup");

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
		// Font handling
		if (needToChangeFonts) [[unlikely]] {
			if (newFontToSwitchTo == nullptr) {
				refreshApplicationFontName();
			} else {
				io.FontDefault = newFontToSwitchTo;
				global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Font", ("Successfully switched to font " + std::string(newFontToSwitchTo->GetDebugName())).c_str());
				newFontToSwitchTo = nullptr;
			}
			needToChangeFonts = false;
		}

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


		// main window
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			int window_width, window_height;
			glfwGetWindowSize(window, &window_width, &window_height);
			ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
			ImGui::SetWindowPos(ImVec2(0, 0));
			ImGui::SetWindowSize("Main Window", ImVec2(window_width, window_height));
			const ImVec2 ImageButtonSize = { ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight() };

			if (ImGui::BeginTabBar("MainTabBar", 0)) {
				ImGuiTabItemFlags tab_flags[5] = { 0, 0, 0, 0, 0 };
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

						if (filenameIsLocked) {
							ImGui::BeginDisabled();
						}

						ImGui::Text("File Name:"); //TODO: this isn't horizontally or vertically centered
						ImGui::SameLine();
						ImGui::InputText("##Main Input Comment", pdata.the_file_input_name, IM_ARRAYSIZE(pdata.the_file_input_name), ImGuiInputTextFlags_CallbackCharFilter, filenameCleaningFunc_inputFile);
						ImGui::SameLine();

						if (filenameIsLocked) {
							ImGui::EndDisabled();
						}

						if (ImGui::ImageButton("##Lock Icon", filenameIsLocked ? lock_icon_texture : unlock_icon_texture, ImageButtonSize)) {
							clear_input_data(filenameIsLocked);
							filenameIsLocked = !filenameIsLocked;
						}
						const float lock_icon_frame_height = 32.0f + ImGui::GetStyle().FramePadding.y;
						//HACK: apparently there's ImGui::GetStyle().FramePadding.y*2.0f extra size on image buttons, but that seems like 2px off
						//regardless, it's irrelevant because this column isn't the largest

						ARVT::copyEvaluatedFileName_toCommentSplitterPath(pdata.the_file_input_name, pdata.evaluated_input_file_name, IM_ARRAYSIZE(pdata.evaluated_input_file_name));
						ImGui::InputText("##Input Comment Path", pdata.evaluated_input_file_name, IM_ARRAYSIZE(pdata.evaluated_input_file_name), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);
						ImGui::SameLine();

						if (ImGui::Button("Preview File##Input Comment")) { //TODO: add?: https://github.com/mlabbe/nativefiledialog
							int result = ARVT::copyFileToCStr(pdata.evaluated_input_file_name, pdata.input_comment_data, IM_ARRAYSIZE(pdata.input_comment_data));
							if (result) {
								strcpy(pdata.input_comment_data, "error"); //TODO: red text
								global_log.AddLog("[%06.2fs] [warn] %s: %s\n", ImGui::GetTime(), "File Preview", strerror(result));
							}
						}
						if (ImGui::BeginItemTooltip()) {
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							ImGui::TextUnformatted("Display the first 16K characters.");
							ImGui::PopTextWrapPos();
							ImGui::EndTooltip();
						}

						/*
						ImGui::Checkbox("Word Wrap##Input Comment", &input_comment_word_wrap);
						//TODO: doesn't seem like word wrap affects TextMultiline so remove this option
						*/

						if (pdata.input_comment_word_wrap) {
							ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x - 400.f);
						}
						ImGui::InputTextMultiline("##input comment", pdata.input_comment_data, IM_ARRAYSIZE(pdata.input_comment_data), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);
						if (pdata.input_comment_word_wrap) {
							ImGui::PopTextWrapPos();
						}

						ImGui::TableNextColumn();

						if (!filenameIsLocked) {
							ImGui::BeginDisabled();
						}

						ImGui::SeparatorText("Image Text");

						ARVT::copyEvaluatedFileName_toCommentTestImagePath_Text(pdata.the_file_input_name, pdata.evaluated_input_split_1, IM_ARRAYSIZE(pdata.evaluated_input_split_1));
						ARVT::copyEvaluatedFileName_toCommentTestImagePath_Speech(pdata.the_file_input_name, pdata.evaluated_input_split_2, IM_ARRAYSIZE(pdata.evaluated_input_split_2));

						ImGui::InputText("##Input Split 1 Path", pdata.evaluated_input_split_1, IM_ARRAYSIZE(pdata.evaluated_input_split_1), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);
						ImGui::SameLine();
						if (ImGui::Button("Preview File##Input Split 1")) {
							int result = ARVT::copyFileToCStr(pdata.evaluated_input_split_1, pdata.input_split_1_data, IM_ARRAYSIZE(pdata.input_split_1_data));
							if (result) {
								strcpy(pdata.input_split_1_data, "error"); //TODO: red text
								global_log.AddLog("[%06.2fs] [warn] %s: %s\n", ImGui::GetTime(), "File Preview", strerror(result));
							}
						}

						ImGui::InputTextMultiline("##Input Split 1 Data", pdata.input_split_1_data, IM_ARRAYSIZE(pdata.input_split_1_data), ImVec2(-FLT_MIN, 0), ImGuiInputTextFlags_ReadOnly);
						//TODO: there should be a third box for ImageMagick text, which basically just replaces "&" with "&amp;" and other stuff
						if (ImGui::Button("Reveal in File Explorer##Input Split 1")) {
							int result = ARVT::revealFileExplorer(pdata.evaluated_input_split_1);
							if (result) {
								//strcpy(pdata.input_split_1_data, "error"); //TODO: red text
								//global_log.AddLog("[%06.2fs] [warn] %s: %s\n", ImGui::GetTime(), "File Explorer", strerror(result));
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("\u2193 Make Copy \u2193")) {
							int result = ARVT::copy_file(pdata.evaluated_input_split_1, pdata.evaluated_input_split_2);
							if (result) {
								strcpy(pdata.input_split_2_data, "error copying"); //TODO: red text
								global_log.AddLog("[%06.2fs] [error] %s: %s\n", ImGui::GetTime(), "Copy", strerror(result));
							} else {
								global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Copy", ("Successfully copied to " + std::string(pdata.evaluated_input_split_2)).c_str());
								int result = ARVT::copyFileToCStr(pdata.evaluated_input_split_2, pdata.input_split_2_data, IM_ARRAYSIZE(pdata.input_split_2_data));
								if (result) {
									strcpy(pdata.input_split_2_data, "error"); //TODO: red text
									global_log.AddLog("[%06.2fs] [warn] %s: %s\n", ImGui::GetTime(), "File Preview", strerror(result));
								}
							}
						}

						ImGui::SeparatorText("Speech Text (optional)");

						ImGui::Checkbox("Use Speech Text", &vdata.use_speech_text);

						if (!vdata.use_speech_text) {
							ImGui::BeginDisabled();
							//pushes to disabled stack
						}

						ImGui::InputText("##Input Split 2 Path", pdata.evaluated_input_split_2, IM_ARRAYSIZE(pdata.evaluated_input_split_2), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);
						ImGui::SameLine();
						if (ImGui::Button("Preview File##Input Split 2")) {
							int result = ARVT::copyFileToCStr(pdata.evaluated_input_split_2, pdata.input_split_2_data, IM_ARRAYSIZE(pdata.input_split_2_data));
							if (result) {
								strcpy(pdata.input_split_2_data, "error"); //TODO: red text
								global_log.AddLog("[%06.2fs] [warn] %s: %s\n", ImGui::GetTime(), "File Preview", strerror(result));
							}
						}

						ImGui::InputTextMultiline("##Input Split 2 Data", pdata.input_split_2_data, IM_ARRAYSIZE(pdata.input_split_2_data), ImVec2(-FLT_MIN, 0), ImGuiInputTextFlags_ReadOnly);
						if (ImGui::Button("Reveal in File Explorer##Input Split 2")) {
							int result = ARVT::revealFileExplorer(pdata.evaluated_input_split_2);
							if (result) {
								//strcpy(pdata.input_split_2_data, "error"); //TODO: red text
								//global_log.AddLog("[%06.2fs] [warn] %s: %s\n", ImGui::GetTime(), "File Explorer", strerror(result));
							}
						}

						if (!vdata.use_speech_text) {
							ImGui::EndDisabled();
							//pops from disabled stack
						}

						if (!filenameIsLocked) {
							ImGui::EndDisabled();
						}

						ImGui::TableNextColumn();

						if (!filenameIsLocked) {
							ImGui::BeginDisabled();
						}

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::SeparatorText("Image");
						ImGui::InputText("Image Width",     idata.image_width_input,    IM_ARRAYSIZE(idata.image_width_input),    ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);
						ImGui::InputText("Image Height",    idata.image_height_input,   IM_ARRAYSIZE(idata.image_height_input),   ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);
						ImGui::InputText("Width Border",    idata.image_w_border_input, IM_ARRAYSIZE(idata.image_w_border_input), ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);
						ImGui::InputText("Height Border",   idata.image_h_border_input, IM_ARRAYSIZE(idata.image_h_border_input), ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);

						ImGui::SeparatorText("Font"); //TODO: align SeparatorText to center
						ImGui::InputText("Font Size",             idata.font_size_input,           IM_ARRAYSIZE(idata.font_size_input),        ImGuiInputTextFlags_CallbackCharFilter, numberOnlyPositiveFunc);
						ImGui::InputText("Font Color",            idata.font_color_input,          IM_ARRAYSIZE(idata.font_color_input),       ImGuiInputTextFlags_CallbackCharFilter, quoteAndSlashScrubbingFunc);
						ImGui::InputText("Background Color",      idata.background_color_input,    IM_ARRAYSIZE(idata.background_color_input), ImGuiInputTextFlags_CallbackCharFilter, quoteAndSlashScrubbingFunc);
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
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::Combo("Image Format", &idata.imageFormatArray_current, idata.imageFormatArray.data(), idata.imageFormatArray.size());
						ImGui::PopItemWidth();

						ARVT::copyEvaluatedFileName_toCommentTestImagePath_TestImage(pdata.the_file_input_name, idata, pdata.evaluated_test_image_path, IM_ARRAYSIZE(pdata.evaluated_test_image_path));
						ImGui::InputText("##Test Image Path", pdata.evaluated_test_image_path, IM_ARRAYSIZE(pdata.evaluated_test_image_path), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);

						if (ImGui::Button("Create \u2192", ImVec2(-FLT_MIN, 0.0f))) {
							int result = ARVT::call_comment_test_image(pdata.the_file_input_name, idata);
							if (result) {
								//TODO: better messages
								global_log.AddLog("[%06.2fs] [error] %s: %s\n", ImGui::GetTime(), "Image", strerror(result));
							} else {
								global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Image", ("Successfully created test image " + std::string(pdata.evaluated_test_image_path)).c_str());
								ret = ImGuiHelpers::LoadTextureFromFile(pdata.evaluated_test_image_path, &createdTestImage_texture, &createdTestImage_width, &createdTestImage_height);
								//TODO: check if success
							}
						}
						lock_filename_tooltip(filenameIsLocked);

						ImGui::SeparatorText("Video Settings (optional)");
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::InputText("Video Replacement", vdata.video_replacement_numbers_input, IM_ARRAYSIZE(vdata.video_replacement_numbers_input), ImGuiInputTextFlags_CallbackCharFilter, video_replacement_scrubbingFunc);
						//TODO: clear these when unlocking?
						ImGui::SameLine();
						ImGuiHelpers::HelpMarker("ex. \"1,2,3\" or \"4,6-8,15,20-30\"\n"
						                         "\"-3\" is start to 3; \"30-\" is 30 to end");
						ImGui::PopItemWidth();
						ImGui::Checkbox("Audio Only", &vdata.audio_only_option_input);

						ARVT::copyEvaluatedFileName_toCommentToSpeechPath(pdata.the_file_input_name, vdata, pdata.evaluated_output_speech_path, IM_ARRAYSIZE(pdata.evaluated_output_speech_path));
						ImGui::InputText("##Output Videos Path", pdata.evaluated_output_speech_path, IM_ARRAYSIZE(pdata.evaluated_output_speech_path), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);

						if (!filenameIsLocked) {
							ImGui::EndDisabled();
						}

						if (adata.voiceArray_current < 0) {
							ImGui::TextColored(ImVec4(1, 0, 0, 1), "You haven't set a voice yet!\nGo to the Configure tab.");
						}
						const float no_voice_height = (adata.voiceArray_current < 0) ? 2 * ImGui::GetTextLineHeight() : 0.0f; //no ItemSpacing.y

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
						columnHeights[0] = lock_icon_frame_height + ImGui::GetFrameHeightWithSpacing() + ImGuiHelpers::getMultilineInputHeight(ImGui::GetTextLineHeight() * 16);
						columnHeights[1] = 7 * ImGui::GetFrameHeightWithSpacing() + 2 * ImGuiHelpers::getMultilineInputHeight(0);
						columnHeights[2] = (5 + test_image_font_item_count + 4 + 4) * ImGui::GetFrameHeightWithSpacing() + no_voice_height;

						const float largestColumn = *std::max_element(columnHeights, columnHeights + IM_ARRAYSIZE(columnHeights));
						const float contentAvailableY = ImGui::GetContentRegionAvail().y + 2*ImGui::GetStyle().ItemSpacing.y;
						const float availableHeight = std::max(0.0f, std::min(largestColumn, contentAvailableY));
						const float availableWidth = std::max(0.0f, ImGui::GetContentRegionAvail().x);

						if ((availableWidth / createdTestImage_width) * createdTestImage_height > availableHeight) {
							if (availableHeight > createdTestImage_height) {
								ImGui::Image((ImTextureID)(intptr_t)createdTestImage_texture, ImVec2(createdTestImage_width, createdTestImage_height));
							} else {
								ImGui::Image((ImTextureID)(intptr_t)createdTestImage_texture, ImVec2((availableHeight / createdTestImage_height) * createdTestImage_width, availableHeight));
							}
						} else {
							if (availableWidth > createdTestImage_width) {
								ImGui::Image((ImTextureID)(intptr_t)createdTestImage_texture, ImVec2(createdTestImage_width, createdTestImage_height));
							} else {
								ImGui::Image((ImTextureID)(intptr_t)createdTestImage_texture, ImVec2(availableWidth, (availableWidth / createdTestImage_width) * createdTestImage_height));
							}
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						if (!filenameIsLocked) {
							ImGui::BeginDisabled();
						}

						if (ImGui::Button("Split!", ImVec2(-FLT_MIN, 0.0f))) {
							int result = ARVT::call_comment_splitter(pdata.the_file_input_name);
							if (result) {
								//TODO: better messages
								global_log.AddLog("[%06.2fs] [error] %s: %s\n", ImGui::GetTime(), "Splitter", strerror(result));
							} else {
								global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Splitter", ("Successfully split " + std::string(pdata.evaluated_input_file_name)).c_str());
								int result = ARVT::copyFileToCStr(ARVT::inputFileName_toCommentTestImagePath_Text(pdata.the_file_input_name).c_str(), pdata.input_split_1_data, IM_ARRAYSIZE(pdata.input_split_1_data));
								if (result) {
									strcpy(pdata.input_split_1_data, "error"); //TODO: red text
									global_log.AddLog("[%06.2fs] [warn] %s: %s\n", ImGui::GetTime(), "File Preview", strerror(result));
								}
							}
						}
						lock_filename_tooltip(filenameIsLocked);

						if (!filenameIsLocked) {
							ImGui::EndDisabled();
						}

						ImGui::TableNextColumn();
						ImGui::TableNextColumn();

						if (!filenameIsLocked) {
							ImGui::BeginDisabled();
						}

						const float write_settings_width = ImGui::GetContentRegionAvail().x * .75f;
						if (ImGui::BeginPopup("Settings")) {
							ARVT::copyEvaluatedFileName_toVideoSettingsPath(pdata.the_file_input_name, pdata.evaluated_video_settings_path, IM_ARRAYSIZE(pdata.evaluated_video_settings_path));
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
									//TODO: better messages
									global_log.AddLog("[%06.2fs] [error] %s: %s\n", ImGui::GetTime(), "Settings", strerror(result));
								} else {
									global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Settings", ("Successfully wrote " + std::string(pdata.evaluated_video_settings_path)).c_str());
								}
							}

							if (ImGui::Button("Import Settings", ImVec2(write_settings_width, 0.0f))) {
								mINI::INIFile video_settings_file(pdata.evaluated_video_settings_path);
								mINI::INIStructure video_settings_object;
								bool result = video_settings_file.read(video_settings_object);

								if (!result) {
									//TODO: better messages
									global_log.AddLog("[%06.2fs] [error] %s: %s\n", ImGui::GetTime(), "Settings", strerror(result));
								} else {
									ARVT::Fill_ImageData(idata, video_settings_object);
									ARVT::Fill_AudioData(adata, video_settings_object, pdata.useExtraCodecs);
									ARVT::Fill_VideoData(vdata, video_settings_object, pdata.useExtraCodecs);
									//if the user changed pdata.useExtraCodecs, that's their fault
									global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Settings", ("Successfully loaded " + std::string(pdata.evaluated_video_settings_path)).c_str());
								}
							}

							if (ImGui::Button("Reveal in File Explorer##Video Settings", ImVec2(write_settings_width, 0.0f))) {
								int result = ARVT::revealFileExplorer(pdata.evaluated_video_settings_path);
								if (result) {
									//strcpy(, "error"); //TODO: red text
									//global_log.AddLog("[%06.2fs] [warn] %s: %s\n", ImGui::GetTime(), "File Explorer", strerror(result));
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

						if (!filenameIsLocked) {
							ImGui::EndDisabled();
						}

						ImGui::TableNextColumn();

						if (!filenameIsLocked) {
							ImGui::BeginDisabled();
						}

						if (adata.voiceArray_current < 0) { ImGui::BeginDisabled(); }
						if (ImGui::Button("GO!", ImVec2(-FLT_MIN, 0.0f))) {
							//TODO: progress bar and async
							int result = ARVT::call_comment_to_speech(pdata.the_file_input_name, idata, adata, vdata);
							if (result) {
								//TODO: better messages
								global_log.AddLog("[%06.2fs] [error] %s: %s\n", ImGui::GetTime(), "Video", strerror(result));
							} else {
								global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Video", ("Successfully created videos " + std::string(pdata.evaluated_output_speech_path)).c_str());
							}
							//TODO: at program start-up, check programs' existence and maybe ffmpeg version
						}
						lock_filename_tooltip(filenameIsLocked);
						if (adata.voiceArray_current < 0) { ImGui::EndDisabled(); }
						if (ImGui::Button("Reveal in File Explorer##Final Video", ImVec2(-FLT_MIN, 0.0f))) {
							//TODO: this should open in the folder if the file doesn't exist
							//yes it's *kinda* a hack to open on just the first video, but it's better than iterating through every file in the folder and checking what's available
							int result = ARVT::revealFileExplorer(ARVT::inputFileName_toCommentToSpeechPath_getFileExplorerName(pdata.the_file_input_name, vdata.videoContainerArray[vdata.videoContainerArray_current], vdata.audio_only_option_input).c_str());
							if (result) {
								//strcpy(, "error"); //TODO: red text
								//global_log.AddLog("[%06.2fs] [warn] %s: %s\n", ImGui::GetTime(), "File Explorer", strerror(result));
							}
						}

						if (!filenameIsLocked) {
							ImGui::EndDisabled();
						}

						ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 1.0f);
						ImGui::EndTable();
						ImGui::PopStyleVar();
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Configure", nullptr, tab_flags[1])) {
					if (ImGui::BeginTable("Configure##table1", 3, table_flags)) {
						ImGui::TableSetupColumn("Audio");
						ImGui::TableSetupColumn("Video");
						ImGui::TableSetupColumn("Other");
						ImGui::TableHeadersRow();
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						//to align text to the left: use a table
						if (ImGui::Combo("Speech Engine", &adata.speechEngineArray_current, adata.speechEngineArray.data(), adata.speechEngineArray.size())) {
							//there was a change
							adata.update_voiceArray();
						}

						ImGui::Combo("Voice", &adata.voiceArray_current, adata.voiceArray, adata.voiceArray_length);
						//TODO: some kind of visual indicator when one isn't selected
						ImGui::SameLine();
						if (ImGui::Button("Refresh")) {
							adata.update_voiceArray();
						}

						if (ImGui::BeginCombo("Audio Encoder", adata.get_audioEncoder()->displayName)) {
							for (int n = 0; n < AudioData::get_audioEncoderArraySize(pdata.useExtraCodecs); n++) {
								if (ImGui::Selectable(AudioData::audioEncoderArrayExtended[n]->displayName, adata.audioEncoderArray_current == n)) {
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
							if (ImGui::BeginCombo(ac_p1.displayTerm, ac_p1.displayValues[adata.audioEncoder_preset1_current])) {
								for (int n = 0; n < ac_p1.displayValues.size(); n++) {
									if (ImGui::Selectable(ac_p1.displayValues[n], adata.audioEncoder_preset1_current == n)) {
										adata.set_encoder_preset1_idx(n);
									}
								}
								ImGui::EndCombo();
							}
						}

						if (!ac->isLossless) {
							//ImGui doesn't support steps for sliders, oh well
							ImGui::SliderScalar("Bitrate (kbps)", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max);
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
								if (ImGui::Selectable(VideoData::videoEncoderArrayExtended[n]->displayName, vdata.videoEncoderArray_current == n)) {
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
							if (ImGui::BeginCombo(vc_p1.displayTerm, vc_p1.displayValues[vdata.videoEncoder_preset1_current])) {
								for (int n = 0; n < vc_p1.displayValues.size(); n++) {
									if (ImGui::Selectable(vc_p1.displayValues[n], vdata.videoEncoder_preset1_current == n)) {
										vdata.set_encoder_preset1_idx(n);
									}
								}
								ImGui::EndCombo();
							}
							if (!vc->preset2.displayValues.empty()) {
								const GenericCodecPreset& vc_p2 = vc->preset2;
								if (ImGui::BeginCombo(vc_p2.displayTerm, vc_p2.displayValues[vdata.videoEncoder_preset2_current])) {
									for (int n = 0; n < vc_p2.displayValues.size(); n++) {
										if (ImGui::Selectable(vc_p2.displayValues[n], vdata.videoEncoder_preset2_current == n)) {
											vdata.set_encoder_preset2_idx(n);
										}
									}
									ImGui::EndCombo();
								}
							}
						}

						if (!vc->isLossless) {
							ImGui::SliderScalar("CRF", ImGuiDataType_S8, &vdata.video_crf_v, &vdata.video_crf_min, &vdata.video_crf_max);
							//ImGui::SameLine();
							//ImGuiHelpers::HelpMarker("CTRL+Click to input a value.");
						}

						ImGui::Unindent();

						//TODO: probably move to main settings, think about non-wav containers for audio-only mode
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
						if (ImGui::DragFloat("Font Size", &pdata.application_font_size, 0.20f, 8.0f, 60.0f, "%.0f")) {
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
						ImGui::ColorEdit4("Window Color", (float*)&style.Colors[ImGuiCol_WindowBg]); //TODO

						if (ImGui::Checkbox("Extra Codecs", &pdata.useExtraCodecs)) {
							if (!pdata.useExtraCodecs) {
								if (adata.audioEncoderArray_current >= AudioData::get_audioEncoderArraySize(false)) {
									adata.set_encoder_idx(0);
								}
								if (vdata.videoEncoderArray_current >= VideoData::get_videoEncoderArraySize(false)) {
									vdata.set_encoder_idx(0);
								}
							}
						}

						ImGui::SeparatorText("Paths");
						ImGui::Text("TODO: three main dirs and a temp dir");
						//other TODO: display what the commands will be (though maybe this should be in the main section?)

						ImGui::SeparatorText("Misc");

						#if 1
						ImGui::Checkbox("Demo Window", &show_demo_window);
						#endif

						ImGui::Text("TODO: reset all to default button");

						ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 1.0f);
						ImGui::EndTable();
						ImGui::PopStyleVar();
					}

					if (ImGui::BeginTable("Configure##table2", 2, table_flags | ImGuiTableFlags_ScrollY, {0.0f, 10.0f * ImGui::GetFrameHeightWithSpacing()})) {
						ImGui::TableSetupColumn("Delete Old Files");
						ImGui::TableSetupColumn("##Logger");
						ImGui::TableHeadersRow();
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						ImGui::SeparatorText("Query");

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						if (ImGui::Combo("File Delete Age", &pdata.fileDeleteAgeList_current, pdata.fileDeleteAgeList.data(), pdata.fileDeleteAgeList.size())) {
							//TODO: not sure
							//deleteFileList.clear();
							//deleteFileLogger.Clear();
						}
						ImGui::PopItemWidth();

						if (ImGui::Button("Query images")) {
							deleteFileList.clear();
							deleteFileLogger.Clear();
							int result = ARVT::getListOfOldFiles(ARVT::TEST_IMAGES.c_str(), pdata.fileDeleteAgeList_values[pdata.fileDeleteAgeList_current], deleteFileList);
							if (result) {
								global_log.AddLog("[%06.2fs] [error] %s: %s\n", ImGui::GetTime(), "Query Files", strerror(result));
							} else {
								global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Query Files", ("Successfully queried " + ARVT::TEST_IMAGES).c_str());
								deleteFileLogger.AddLog("Found %d files that are more than %s old:\n", deleteFileList.size(), pdata.fileDeleteAgeList[pdata.fileDeleteAgeList_current]);
								for (const auto& f : deleteFileList) {
									deleteFileLogger.AddLog("%s\n", f.c_str());
								}
							}
						}
						//ImGui::SameLine();
						if (ImGui::Button("Query videos")) {
							deleteFileList.clear();
							deleteFileLogger.Clear();
							int result = ARVT::getListOfOldFiles(ARVT::OUTPUT_SPEECH.c_str(), pdata.fileDeleteAgeList_values[pdata.fileDeleteAgeList_current], deleteFileList);
							if (result) {
								global_log.AddLog("[%06.2fs] [error] %s: %s\n", ImGui::GetTime(), "Query Files", strerror(result));
							} else {
								global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Query Files", ("Successfully queried " + ARVT::OUTPUT_SPEECH).c_str());
								deleteFileLogger.AddLog("Found %d files that are more than %s old:\n", deleteFileList.size(), pdata.fileDeleteAgeList[pdata.fileDeleteAgeList_current]);
								for (const auto& f : deleteFileList) {
									deleteFileLogger.AddLog("%s\n", f.c_str());
								}
							}
						}

						if (ImGui::Button("Clear queried files")) {
							deleteFileList.clear();
							deleteFileLogger.Clear();
						}

						ImGui::SeparatorText("Delete");

						//only allowed to delete what's in the list: if something new suddenly meets the cutoff, it's spared; if something old gets its modified time changed, it's gone
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
									global_log.AddLog("[%06.2fs] [error] %s: %s\n", ImGui::GetTime(), "Delete Files", strerror(result));
								} else {
									deleteFileLogger.Clear();
									global_log.AddLog("[%06.2fs] [info] %s: %s\n", ImGui::GetTime(), "Delete Files", ("Successfully deleted " + std::to_string(deleteFileList.size()) + " files").c_str());
								}
								deleteFileList.clear();
								ImGui::CloseCurrentPopup();
							}
							ImGui::EndPopup();
						}

						// Check the logger's data instead of deleteFileList so it's possible to "delete" 0 files
						if (deleteFileLogger.Buf.empty()) { ImGui::BeginDisabled(); }
						if (ImGui::Button("Delete queried files")) {
							ImGui::OpenPopup("Delete queried files");
						}
						if (deleteFileLogger.Buf.empty()) { ImGui::EndDisabled(); }

						ImGui::TableNextColumn();

						deleteFileLogger.Draw();

						//ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 1.0f);
						ImGui::EndTable();
						//ImGui::PopStyleVar();
					}

					ImGui::EndTabItem();
				}

				/*
				if (ImGui::BeginTabItem("SSH", nullptr, tab_flags[2])) {
					ImGui::Text("TODO");
					ImGui::EndTabItem();
				}
				*/

				if (ImGui::BeginTabItem("Help", nullptr, tab_flags[2])) {
					ImGui::Text("TODO");
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("About", nullptr, tab_flags[3])) {
					ImGui::Text("License: GNU General Public License v3.0");
					ImGui::Text("SPDX-License-Identifier: GPL-3.0-only");
					ImGui::Text("Requirements: TODO");
					ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
					ImGui::TextLinkOpenURL("GitHub link", "https://github.com/tanksdude/automated-reddit-video-tool-gui");
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

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

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

    return 0;
}
