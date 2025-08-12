#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <stdio.h> //fprintf for glfwSetErrorCallback
#include <algorithm> //std::max_element

#include <GLFW/glfw3.h>
#define MINI_CASE_SENSITIVE
#include <mini/ini.h>

#include "imgui_helpers.h"
#include "arvt_helpers.h"
#include "ini_helper.h"

#include "image_data.h"
#include "audio_data.h"
#include "video_data.h"
#include "program_data.h"

ImageData idata;
AudioData adata;
VideoData vdata;
ProgramData pdata;

bool needToChangeFonts = false;

void refreshApplicationFontName() {
	ImGuiIO& io = ImGui::GetIO();
	ImFont* newFont = io.Fonts->AddFontFromFileTTF(pdata.application_font_path);
	if (newFont == nullptr) {
		//TODO: error handle
		return;
	}
	io.FontDefault = newFont;

	//ideally this would be used but it doesn't change the font for some reason:
	//ImGui::PushFont(newFont, newSize);
}

void refreshApplicationFontSize() {
	//TODO: change this when ImGui fixes it
	ImGuiStyle& style = ImGui::GetStyle();
	style.FontSizeBase = pdata.application_font_size;
	style._NextFrameFontSizeBase = style.FontSizeBase; // FIXME: Temporary hack until we finish remaining work.
}

auto integerOnlyPositiveFunc = [] (ImGuiInputTextCallbackData* data) {
	if (data->EventChar >= '0' && data->EventChar <= '9') { return 0; }
	return 1;
};
auto filenameCleaningFunc = [] (ImGuiInputTextCallbackData* data) {
	//main goal: don't allow quotes
	//additionally, don't allow NTFS-invalid characters, list from https://en.wikipedia.org/wiki/NTFS
	if (data->EventChar == '/'  ||
	    data->EventChar == '\\' ||
	    data->EventChar == ':'  ||
	    data->EventChar == '*'  ||
	    data->EventChar == '\"' ||
	    data->EventChar == '?'  ||
	    data->EventChar == '<'  ||
	    data->EventChar == '>'  ||
	    data->EventChar == '|')
	{ return 1; }
	return 0;
};
auto filepathCleaningFunc = [] (ImGuiInputTextCallbackData* data) {
	//identical to filenameCleaningFunc but allows slashes
	if (data->EventChar == ':'  ||
	    data->EventChar == '*'  ||
	    data->EventChar == '\"' ||
	    data->EventChar == '?'  ||
	    data->EventChar == '<'  ||
	    data->EventChar == '>'  ||
	    data->EventChar == '|')
	{ return 1; }
	return 0;
};
auto quoteScrubbingFunc = [] (ImGuiInputTextCallbackData* data) {
	if (data->EventChar == '\"' || data->EventChar == '\'') { return 1; }
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



static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
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
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow(1600, 900, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
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
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
	refreshApplicationFontSize();
	refreshApplicationFontName();

    // Our state
    bool show_demo_window = false;
	bool set_default_tab = true;

	int my_image_width = 0;
	int my_image_height = 0;
	GLuint my_image_texture = 0;
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

	ARVT::CreateDefaultIniIfNeeded("../arvt.ini");
	mINI::INIFile ini_file("../arvt.ini");
	mINI::INIStructure ini_object;
	ini_file.read(ini_object);

	ARVT::Fill_ProgramData(pdata, ini_object);
	ARVT::Fill_ImageData(idata, ini_object);
	ARVT::Fill_AudioData(adata, ini_object, pdata.useExtraCodecs);
	ARVT::Fill_VideoData(vdata, ini_object, pdata.useExtraCodecs);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
		// Font handling
		if (needToChangeFonts) [[unlikely]] {
			refreshApplicationFontName();
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
			const auto old_style_WindowBorderSize = style.WindowBorderSize; //push
			style.WindowBorderSize = 0.0f;
			int window_width, window_height;
			glfwGetWindowSize(window, &window_width, &window_height);
			ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);
			ImGui::SetWindowPos(ImVec2(0, 0));
			ImGui::SetWindowSize("Main Window", ImVec2(window_width+2, window_height+2));

			if (ImGui::BeginTabBar("MainTabBar", 0)) {
				ImGuiTabItemFlags tab_flags[6] = { 0, 0, 0, 0, 0, 0 };
				if (set_default_tab) [[unlikely]] {
					//instead of clamping, set to zero on a bad value, because that's more obvious (maybe Help or About would be better?):
					const int idx = (pdata.default_tab_idx > 6 || pdata.default_tab_idx < 0) ? 0 : pdata.default_tab_idx;
					tab_flags[idx] |= ImGuiTabItemFlags_SetSelected;
					set_default_tab = false;
				}

				if (ImGui::BeginTabItem("Execute", nullptr, tab_flags[0])) {
					if (ImGui::BeginTable("Execute##table1", 4, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_PadOuterX)) {
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
						ImGui::InputText("##Main Input Comment", pdata.the_file_input_name, IM_ARRAYSIZE(pdata.the_file_input_name), ImGuiInputTextFlags_CallbackCharFilter, filenameCleaningFunc);
						ImGui::SameLine();

						if (filenameIsLocked) {
							ImGui::EndDisabled();
						}

						if (ImGui::ImageButton("##Lock Icon", filenameIsLocked ? lock_icon_texture : unlock_icon_texture, ImVec2(pdata.application_font_size, pdata.application_font_size))) {
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
							}
						}

						ImGui::InputTextMultiline("##Input Split 1 Data", pdata.input_split_1_data, IM_ARRAYSIZE(pdata.input_split_1_data), ImVec2(-FLT_MIN, 0), ImGuiInputTextFlags_ReadOnly);
						//TODO: there should be a third box for ImageMagick text, which basically just replaces "&" with "&amp;" and other stuff
						if (ImGui::Button("Reveal in File Explorer##Input Split 1")) {
							int result = ARVT::revealFileExplorer(pdata.evaluated_input_split_1);
							if (result) {
								strcpy(pdata.input_split_2_data, "error"); //TODO: red text
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("↓ Make Copy ↓")) {
							int result = ARVT::copy_file(pdata.evaluated_input_split_1, pdata.evaluated_input_split_2);
							if (result) {
								strcpy(pdata.input_split_2_data, "error copying"); //TODO: red text
							} else {
								int result = ARVT::copyFileToCStr(pdata.evaluated_input_split_2, pdata.input_split_2_data, IM_ARRAYSIZE(pdata.input_split_2_data));
								if (result) {
									strcpy(pdata.input_split_2_data, "error"); //TODO: red text
								}
							}
						}
						//TODO: docs/fonts.md

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
							}
						}

						ImGui::InputTextMultiline("##Input Split 2 Data", pdata.input_split_2_data, IM_ARRAYSIZE(pdata.input_split_2_data), ImVec2(-FLT_MIN, 0), ImGuiInputTextFlags_ReadOnly);
						if (ImGui::Button("Reveal in File Explorer##Input Split 2")) {
							int result = ARVT::revealFileExplorer(pdata.evaluated_input_split_2);
							if (result) {
								strcpy(pdata.input_split_2_data, "error"); //TODO: red text
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
						ImGui::InputText("Image Width",     idata.image_width_input,    IM_ARRAYSIZE(idata.image_width_input),    ImGuiInputTextFlags_CharsDecimal);
						ImGui::InputText("Image Height",    idata.image_height_input,   IM_ARRAYSIZE(idata.image_height_input),   ImGuiInputTextFlags_CharsDecimal);
						ImGui::InputText("Width Border",    idata.image_w_border_input, IM_ARRAYSIZE(idata.image_w_border_input), ImGuiInputTextFlags_CharsDecimal);
						ImGui::InputText("Height Border",   idata.image_h_border_input, IM_ARRAYSIZE(idata.image_h_border_input), ImGuiInputTextFlags_CharsDecimal);

						ImGui::SeparatorText("Font"); //TODO: align SeparatorText to center
						ImGui::InputText("Font Size",             idata.font_size_input,           IM_ARRAYSIZE(idata.font_size_input),        ImGuiInputTextFlags_CharsDecimal);
						ImGui::InputText("Font Color",            idata.font_color_input,          IM_ARRAYSIZE(idata.font_color_input),       ImGuiInputTextFlags_CallbackCharFilter, quoteScrubbingFunc);
						ImGui::InputText("Background Color",      idata.background_color_input,    IM_ARRAYSIZE(idata.background_color_input), ImGuiInputTextFlags_CallbackCharFilter, quoteScrubbingFunc); //TODO: think these need to also scrub backslashes
						ImGui::SliderScalar("Newline Count", ImGuiDataType_U8, &idata.paragraph_newline_v, &idata.paragraph_newline_min, &idata.paragraph_newline_max);
						ImGui::Checkbox("Paragraph Tabbed Start", &idata.paragraph_tabbed_start_input);
						ImGui::PopItemWidth();

						//TODO: export/import settings (use INI file) (buttons: export, quick import (same file name), import specific (file select)) (also a separate gui for batch remake, where it will re-run the script with the settings files)

						ImGui::SeparatorText("Export");
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::Combo("Image Format", &idata.imageFormatArray_current, idata.imageFormatArray, IM_ARRAYSIZE(idata.imageFormatArray));
						ImGui::PopItemWidth();

						ARVT::copyEvaluatedFileName_toCommentTestImagePath_TestImage(pdata.the_file_input_name, idata, pdata.evaluated_test_image_path, IM_ARRAYSIZE(pdata.evaluated_test_image_path));
						ImGui::InputText("##Test Image Path", pdata.evaluated_test_image_path, IM_ARRAYSIZE(pdata.evaluated_test_image_path), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);

						if (ImGui::Button("Create →", ImVec2(-FLT_MIN, 0.0f))) {
							ARVT::call_comment_test_image(pdata.the_file_input_name, idata);
							//TODO: check if success
							ret = ImGuiHelpers::LoadTextureFromFile(pdata.evaluated_test_image_path, &my_image_texture, &my_image_width, &my_image_height);
						}

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
						columnHeights[2] = (5 + 6 + 4 + 4) * ImGui::GetFrameHeightWithSpacing() + no_voice_height;

						const float largestColumn = *std::max_element(columnHeights, columnHeights + IM_ARRAYSIZE(columnHeights));
						const float contentAvailableY = ImGui::GetContentRegionAvail().y + 2*ImGui::GetStyle().ItemSpacing.y;
						const float availableHeight = std::max(0.0f, std::min(largestColumn, contentAvailableY));
						const float availableWidth = std::max(0.0f, ImGui::GetContentRegionAvail().x);

						if ((availableWidth / my_image_width) * my_image_height > availableHeight) {
							ImGui::Image((ImTextureID)(intptr_t)my_image_texture, ImVec2((availableHeight / my_image_height) * my_image_width, availableHeight));
						} else {
							ImGui::Image((ImTextureID)(intptr_t)my_image_texture, ImVec2(availableWidth, (availableWidth / my_image_width) * my_image_height));
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						if (!filenameIsLocked) {
							ImGui::BeginDisabled();
						}

						if (ImGui::Button("Split!", ImVec2(-FLT_MIN, 0.0f))) {
							ARVT::call_comment_splitter(pdata.the_file_input_name);
							ARVT::copyFileToCStr(ARVT::inputFileName_toCommentTestImagePath_Text(pdata.the_file_input_name).c_str(), pdata.input_split_1_data, IM_ARRAYSIZE(pdata.input_split_1_data));
						}
						if (ImGui::BeginItemTooltip()) {
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							ImGui::TextUnformatted("Lock the file name before clicking this!");
							ImGui::PopTextWrapPos();
							ImGui::EndTooltip();
						}

						if (!filenameIsLocked) {
							ImGui::EndDisabled();
						}

						ImGui::TableNextColumn();
						ImGui::TableNextColumn();
						ImGui::TableNextColumn();

						if (!filenameIsLocked) {
							ImGui::BeginDisabled();
						}

						if (adata.voiceArray_current < 0) { ImGui::BeginDisabled(); }
						if (ImGui::Button("GO!", ImVec2(-FLT_MIN, 0.0f))) {
							//TODO: progress bar and async
							ARVT::call_comment_to_speech(pdata.the_file_input_name, idata, adata, vdata);
							//TODO: at program start-up, check programs' existence and maybe ffmpeg version
						}
						if (adata.voiceArray_current < 0) { ImGui::EndDisabled(); }
						if (ImGui::Button("Reveal in File Explorer##final video", ImVec2(-FLT_MIN, 0.0f))) {
							//TODO: this should open in the folder if the file doesn't exist
							//yes it's *kinda* a hack to open on just the first video, but it's better than iterating through every file in the folder and checking what's available
							int result = ARVT::revealFileExplorer(ARVT::inputFileName_toCommentToSpeechPath_getFileExplorerName(pdata.the_file_input_name, vdata.videoContainerArray[vdata.videoContainerArray_current], vdata.audio_only_option_input).c_str());
							if (result) {
								//strcpy(, "error"); //TODO: red text
							}
						}

						if (!filenameIsLocked) {
							ImGui::EndDisabled();
						}

						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Configure", nullptr, tab_flags[1])) {
					if (ImGui::BeginTable("Configure##table1", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_PadOuterX)) {
						ImGui::TableSetupColumn("Audio");
						ImGui::TableSetupColumn("Video");
						ImGui::TableSetupColumn("Other");
						ImGui::TableHeadersRow();
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);

						//to align text to the left: use a table
						if (ImGui::Combo("Speech Engine", &adata.voiceEngineArray_current, adata.voiceEngineArray, IM_ARRAYSIZE(adata.voiceEngineArray))) {
							//there was a change
							adata.update_voiceArray();
						}

						ImGui::Combo("Voice", &adata.voiceArray_current, adata.voiceArray, adata.voiceArray_length);
						//TODO: some kind of visual indicator when one isn't selected
						ImGui::SameLine();
						if (ImGui::Button("Refresh")) {
							adata.update_voiceArray();
						}

						if (ImGui::Combo("Audio Encoder", &adata.audioEncoderArray_current, AudioData::get_audioEncoderArray(pdata.useExtraCodecs), AudioData::get_audioEncoderArraySize(pdata.useExtraCodecs))) {
							adata.update_audioBitrateValues();
							adata.update_audioPresetArray();
						}

						ImGui::Indent();

						ImGui::Text("Recommendation:");
						ImGui::SameLine();
						ImGui::Image(recommendationStr_toTexId[adata.get_audioEncoderRecommendation()], ImVec2(pdata.application_font_size, pdata.application_font_size));

						ImGui::Text("Information:");
						ImGui::SameLine();
						ImGuiHelpers::HelpMarker(adata.get_audioEncoderInformationText().c_str());

						if (adata.audioCodec_hasPreset) {
							ImGui::Combo(adata.audioCodec_presetTerm.c_str(), &adata.audioPresetArray_current, adata.get_audioPresetArray(), adata.get_audioPresetArray_size(), adata.get_audioPresetArray_size());
						}

						if (!adata.get_audioEncoderIsLossless()) {
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
							ImGui::Combo("FPS##Integer", &vdata.fpsArray_current, vdata.fpsArray, IM_ARRAYSIZE(vdata.fpsArray), IM_ARRAYSIZE(vdata.fpsArray));
						}
						ImGui::Unindent();

						if (ImGui::Combo("Video Encoder", &vdata.videoEncoderArray_current, VideoData::get_videoEncoderArray(pdata.useExtraCodecs), VideoData::get_videoEncoderArraySize(pdata.useExtraCodecs))) {
							vdata.update_videoCrfValues();
							vdata.update_videoPresetArray();
						}

						ImGui::Indent();

						ImGui::Text("Recommendation:");
						ImGui::SameLine();
						ImGui::Image(recommendationStr_toTexId[vdata.get_videoEncoderRecommendation()], ImVec2(pdata.application_font_size, pdata.application_font_size));

						ImGui::Text("Information:");
						ImGui::SameLine();
						ImGuiHelpers::HelpMarker(vdata.get_videoEncoderInformationText().c_str());
						ImGui::SameLine();
						ImGui::Text(vdata.get_videoEncoderSupportsAlpha() ? "  Alpha channel: Yes" : "  Alpha channel: No");

						if (vdata.videoCodec_hasPreset1) {
							ImGui::Combo(vdata.videoCodec_preset1Term.c_str(), &vdata.videoPresetArray1_current, vdata.get_videoPresetArray1(), vdata.get_videoPresetArray1_size(), vdata.get_videoPresetArray1_size());
							//doesn't take flags like ImGuiComboFlags_HeightLargest
							if (vdata.videoCodec_hasPreset2) {
								ImGui::Combo(vdata.videoCodec_preset2Term.c_str(), &vdata.videoPresetArray2_current, vdata.get_videoPresetArray2(), vdata.get_videoPresetArray2_size(), vdata.get_videoPresetArray2_size());
							}
						}

						if (!vdata.get_videoEncoderIsLossless()) {
							ImGui::SliderScalar("CRF", ImGuiDataType_S8, &vdata.crf_v, &vdata.crf_min, &vdata.crf_max);
							//ImGui::SameLine();
							//HelpMarker("CTRL+Click to input a value.");
						}

						ImGui::Unindent();

						//TODO: probably move to main settings, think about non-wav containers for audio-only mode
						ImGui::Combo("Container", &vdata.videoContainerArray_current, vdata.videoContainerArray, IM_ARRAYSIZE(vdata.videoContainerArray));

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

						//TODO: change this to a loader, then use the example font selector?
						ImGui::InputText("Font Path", pdata.application_font_path, IM_ARRAYSIZE(pdata.application_font_path), ImGuiInputTextFlags_CallbackCharFilter, filepathCleaningFunc);
						ImGui::SameLine();
						if (ImGui::Button("Refresh##Font Path")) {
							needToChangeFonts = true;
						}
						if (ImGui::DragFloat("Font Size", &pdata.application_font_size, 0.20f, 8.0f, 60.0f, "%.0f")) {
							refreshApplicationFontSize();
						}

						ImGui::ColorEdit3("Background Color", (float*)&pdata.background_color);

						if (ImGui::Checkbox("Extra Codecs", &pdata.useExtraCodecs)) {
							if (!pdata.useExtraCodecs) {
								if (adata.audioEncoderArray_current >= AudioData::get_audioEncoderArraySize(false)) {
									adata.audioEncoderArray_current = 0;
									adata.update_audioBitrateValues();
									adata.update_audioPresetArray();
								}
								if (vdata.videoEncoderArray_current >= VideoData::get_videoEncoderArraySize(false)) {
									vdata.videoEncoderArray_current = 0;
									vdata.update_videoCrfValues();
									vdata.update_videoPresetArray();
								}
							}
						}

						ImGui::SeparatorText("Paths");
						ImGui::Text("TODO: three main dirs and a temp dir");
						//other TODO: display what the commands will be (though maybe this should be in the main section?)

						ImGui::SeparatorText("Misc");

						//TODO: application settings (fonts mainly)

						ImGui::BeginDisabled(); //TODO: un-disable when there's saving and loading of video settings
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::Combo("File Delete Age", &pdata.imageDeleteAgeList_current, pdata.imageDeleteAgeList, IM_ARRAYSIZE(pdata.imageDeleteAgeList));
						ImGui::PopItemWidth();
						ImGui::SameLine();
						if (ImGui::Button("Delete videos")) {
							int result = ARVT::deleteAllOldFiles(ARVT::OUTPUT_SPEECH.c_str(), pdata.imageDeleteAgeList_values[pdata.imageDeleteAgeList_current]);
							if (result) {
								//TODO
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Delete images")) {
							int result = ARVT::deleteAllOldFiles(ARVT::TEST_IMAGES.c_str(), pdata.imageDeleteAgeList_values[pdata.imageDeleteAgeList_current]);
							if (result) {
								//TODO
							}
						}
						ImGui::EndDisabled();

						ImGui::Text("TODO: reset all to default button");

						#if 1
						ImGui::Checkbox("Demo Window", &show_demo_window);
						#endif

						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				/*
				if (ImGui::BeginTabItem("SSH", nullptr, tab_flags[2])) {
					ImGui::Text("TODO");
					ImGui::EndTabItem();
				}
				*/

				if (ImGui::BeginTabItem("Defaults", nullptr, tab_flags[3])) {
					ImGui::BeginDisabled();

					ImGui::Text("Write Changes:"); ImGui::SameLine();
					if (ImGui::ImageButton("##Write Changes", return_symbol_texture, ImVec2(pdata.application_font_size, pdata.application_font_size))) {
						//TODO
						//ini_file.write(ini_object, true);
					}
					ImGui::Text("Load Defaults:"); ImGui::SameLine();
					if (ImGui::ImageButton("##Load Defaults", circle_arrows_texture, ImVec2(pdata.application_font_size, pdata.application_font_size))) {
						//TODO
					}

					if (ImGui::BeginTable("Defaults##table1", 4, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_PadOuterX)) {
						ImGui::TableSetupColumn("Image");
						ImGui::TableSetupColumn("Audio");
						ImGui::TableSetupColumn("Video");
						ImGui::TableSetupColumn("Other");
						ImGui::TableHeadersRow();
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);

						//TODO: these should all be text inputs

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::SeparatorText("Size");
						ImGui::InputText("Image Width",     idata.image_width_input,    IM_ARRAYSIZE(idata.image_width_input),    ImGuiInputTextFlags_CharsDecimal);
						ImGui::InputText("Image Height",    idata.image_height_input,   IM_ARRAYSIZE(idata.image_height_input),   ImGuiInputTextFlags_CharsDecimal);
						ImGui::InputText("Width Border",    idata.image_w_border_input, IM_ARRAYSIZE(idata.image_w_border_input), ImGuiInputTextFlags_CharsDecimal);
						ImGui::InputText("Height Border",   idata.image_h_border_input, IM_ARRAYSIZE(idata.image_h_border_input), ImGuiInputTextFlags_CharsDecimal);

						ImGui::SeparatorText("Font"); //TODO: align SeparatorText to center
						ImGui::InputText("Font Size",             idata.font_size_input,           IM_ARRAYSIZE(idata.font_size_input),        ImGuiInputTextFlags_CharsDecimal);
						ImGui::InputText("Font Color",            idata.font_color_input,          IM_ARRAYSIZE(idata.font_color_input),       ImGuiInputTextFlags_CallbackCharFilter, quoteScrubbingFunc);
						ImGui::InputText("Background Color",      idata.background_color_input,    IM_ARRAYSIZE(idata.background_color_input), ImGuiInputTextFlags_CallbackCharFilter, quoteScrubbingFunc); //TODO: think these need to also scrub backslashes
						ImGui::SliderScalar("Newline Count", ImGuiDataType_U8, &idata.paragraph_newline_v, &idata.paragraph_newline_min, &idata.paragraph_newline_max);
						ImGui::Checkbox("Paragraph Tabbed Start", &idata.paragraph_tabbed_start_input);
						ImGui::PopItemWidth();

						ImGui::SeparatorText("Export");
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::Combo("Image Format", &idata.imageFormatArray_current, idata.imageFormatArray, IM_ARRAYSIZE(idata.imageFormatArray));
						ImGui::PopItemWidth();

						ImGui::TableNextColumn();

						if (ImGui::Combo("Speech Engine", &adata.voiceEngineArray_current, adata.voiceEngineArray, IM_ARRAYSIZE(adata.voiceEngineArray))) {
							//there was a change
							adata.update_voiceArray();
						}

						ImGui::Combo("Voice", &adata.voiceArray_current, adata.voiceArray, adata.voiceArray_length);
						//TODO: some kind of visual indicator when one isn't selected
						ImGui::SameLine();
						if (ImGui::Button("Refresh")) {
							adata.update_voiceArray();
						}

						if (ImGui::Combo("Audio Encoder", &adata.audioEncoderArray_current, AudioData::get_audioEncoderArray(pdata.useExtraCodecs), AudioData::get_audioEncoderArraySize(pdata.useExtraCodecs))) {
							adata.update_audioBitrateValues();
							adata.update_audioPresetArray();
						}

						ImGui::Indent();

						if (adata.audioCodec_hasPreset) {
							ImGui::Combo(adata.audioCodec_presetTerm.c_str(), &adata.audioPresetArray_current, adata.get_audioPresetArray(), adata.get_audioPresetArray_size(), adata.get_audioPresetArray_size());
						}

						//ImGui doesn't support steps for sliders, oh well
						ImGui::SliderScalar("Bitrate (kbps)", ImGuiDataType_S16, &adata.audio_bitrate_v, &adata.audio_bitrate_min, &adata.audio_bitrate_max);

						ImGui::Unindent();

						ImGui::TableNextColumn();

						ImGui::Checkbox("Audio Only", &vdata.audio_only_option_input);

						//TODO: probably merge into one InputText
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.4f);
						ImGui::InputText("##FPS Numerator",      vdata.fps_numerator_input,   IM_ARRAYSIZE(vdata.fps_numerator_input),   ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);
						ImGui::SameLine();
						ImGui::Text(" / ");
						ImGui::SameLine();
						ImGui::InputText("FPS##FPS Denominator", vdata.fps_denominator_input, IM_ARRAYSIZE(vdata.fps_denominator_input), ImGuiInputTextFlags_CallbackCharFilter, integerOnlyPositiveFunc);
						ImGui::PopItemWidth();
						ImGui::Combo("FPS##Integer", &vdata.fpsArray_current, vdata.fpsArray, IM_ARRAYSIZE(vdata.fpsArray), IM_ARRAYSIZE(vdata.fpsArray));

						if (ImGui::Combo("Video Encoder", &vdata.videoEncoderArray_current, VideoData::get_videoEncoderArray(pdata.useExtraCodecs), VideoData::get_videoEncoderArraySize(pdata.useExtraCodecs))) {
							vdata.update_videoCrfValues();
							vdata.update_videoPresetArray();
						}

						ImGui::Indent();

						if (vdata.videoCodec_hasPreset1) {
							ImGui::Combo(vdata.videoCodec_preset1Term.c_str(), &vdata.videoPresetArray1_current, vdata.get_videoPresetArray1(), vdata.get_videoPresetArray1_size(), vdata.get_videoPresetArray1_size());
							//doesn't take flags like ImGuiComboFlags_HeightLargest
							if (vdata.videoCodec_hasPreset2) {
								ImGui::Combo(vdata.videoCodec_preset2Term.c_str(), &vdata.videoPresetArray2_current, vdata.get_videoPresetArray2(), vdata.get_videoPresetArray2_size(), vdata.get_videoPresetArray2_size());
							}
						}

						ImGui::SliderScalar("CRF", ImGuiDataType_S8, &vdata.crf_v, &vdata.crf_min, &vdata.crf_max);

						ImGui::Unindent();

						ImGui::Combo("Container", &vdata.videoContainerArray_current, vdata.videoContainerArray, IM_ARRAYSIZE(vdata.videoContainerArray));

						ImGui::Indent();
						ImGui::Checkbox("-movflags=+faststart", &vdata.faststart_flag);
						ImGui::Unindent();

						ImGui::TableNextColumn();

						ImGui::SeparatorText("Application");

						ImGui::InputText("Font Path", pdata.application_font_path, IM_ARRAYSIZE(pdata.application_font_path), ImGuiInputTextFlags_CallbackCharFilter, filepathCleaningFunc);
						ImGui::DragFloat("Font Size", &pdata.application_font_size, 0.20f, 8.0f, 60.0f, "%.0f");

						ImGui::ColorEdit3("Background Color", (float*)&pdata.background_color);

						if (ImGui::Checkbox("Extra Codecs", &pdata.useExtraCodecs)) {
							if (!pdata.useExtraCodecs) {
								if (adata.audioEncoderArray_current >= AudioData::get_audioEncoderArraySize(false)) {
									adata.audioEncoderArray_current = 0;
									adata.update_audioBitrateValues();
									adata.update_audioPresetArray();
								}
								if (vdata.videoEncoderArray_current >= VideoData::get_videoEncoderArraySize(false)) {
									vdata.videoEncoderArray_current = 0;
									vdata.update_videoCrfValues();
									vdata.update_videoPresetArray();
								}
							}
						}

						ImGui::SeparatorText("Paths");
						ImGui::Text("TODO: three main dirs and a temp dir");
						//other TODO: display what the commands will be (though maybe this should be in the main section?)

						ImGui::EndDisabled();
						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Help", nullptr, tab_flags[4])) {
					ImGui::Text("TODO");
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("About", nullptr, tab_flags[5])) {
					ImGui::Text("License: GNU General Public License v3.0");
					ImGui::Text("SPDX-License-Identifier: GPL-3.0-only");
					ImGui::Text("Requirements: TODO");
					ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			ImGui::End();
			style.WindowBorderSize = old_style_WindowBorderSize; //pop
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
