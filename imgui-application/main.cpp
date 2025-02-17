#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include <iostream> //TODO
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers


// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}



#include "helpers.h"
#include "image_data.h"
#include "audio_data.h"
#include "video_data.h"

char the_file_input_name[1024] = "lorem_ipsum";
char evaluated_input_file_name[1024] = "evaluated_input_file_name"; //TODO: these can overflow

char input_comment_data[1024 * 16] = "input_comment_data";
bool input_comment_word_wrap = true; //TODO

char evaluated_input_split_1[1024] = "evaluated_input_split_1";
char evaluated_input_split_2[1024] = "evaluated_input_split_2";
char input_split_1_data[1024 * 16] = "input_split_1_data";
char input_split_2_data[1024 * 16] = "input_split_2_data";

char evaluated_test_image_path[1024] = "evaluated_test_image_path";


auto filnameCleaningFunc = [] (ImGuiInputTextCallbackData* data) {
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
		strcpy(input_comment_data, "");
		strcpy(input_split_1_data, "");
		strcpy(input_split_2_data, "");
	} else {
		//now unlocked
		//nothing
	}
}

ImageData idata;
AudioData adata;
VideoData vdata;

const char* imageDeleteAgeList[] = { "0 seconds", "1 hour", "24 hours", "2 weeks", "1 month", "6 months" };
const int imageDeleteAgeList_values[] = { 0, 1, 24, 14*24, 30*24, 180*24 };
static_assert(IM_ARRAYSIZE(imageDeleteAgeList) == IM_ARRAYSIZE(imageDeleteAgeList_values));
int imageDeleteAgeList_current = 0;



#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromMemory(const void* data, size_t data_size, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}

// Open and read a file, then forward to LoadTextureFromMemory()
bool LoadTextureFromFile(const char* file_name, GLuint* out_texture, int* out_width, int* out_height)
{
    FILE* f = fopen(file_name, "rb");
    if (f == NULL)
        return false;
    fseek(f, 0, SEEK_END);
    size_t file_size = (size_t)ftell(f);
    if (file_size == -1)
        return false;
    fseek(f, 0, SEEK_SET);
    void* file_data = IM_ALLOC(file_size);
    fread(file_data, 1, file_size, f);
    bool ret = LoadTextureFromMemory(file_data, file_size, out_texture, out_width, out_height);
    IM_FREE(file_data);
    return ret;
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

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 24.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	bool set_default_tab = true;
	int default_tab_idx = 0;

	int my_image_width = 0;
	int my_image_height = 0;
	GLuint my_image_texture = 0;
	bool ret;

	int lock_icon_width = 0;
	int lock_icon_height = 0;
	GLuint lock_icon_texture = 0;
	ret = LoadTextureFromFile("../locked_1f512.png", &lock_icon_texture, &lock_icon_width, &lock_icon_height);
	IM_ASSERT(ret);

	int unlock_icon_width = 0;
	int unlock_icon_height = 0;
	GLuint unlock_icon_texture = 0;
	ret = LoadTextureFromFile("../unlocked_1f513.png", &unlock_icon_texture, &unlock_icon_width, &unlock_icon_height);
	IM_ASSERT(ret);

	bool filenameIsLocked = false;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
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

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }


		// main window
		{
			ImGui::Begin("Main Window", nullptr);

			if (ImGui::BeginTabBar("MainTabBar", 0)) {
				ImGuiTabItemFlags tab_flags[5] = { 0, 0, 0, 0, 0 };
				if (set_default_tab) [[unlikely]] {
					tab_flags[default_tab_idx] |= ImGuiTabItemFlags_SetSelected;
					set_default_tab = false;
				}

				if (ImGui::BeginTabItem("Execute", nullptr, tab_flags[0])) {
					if (ImGui::BeginTable("table1", 4, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_PadOuterX)) {
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
						ImGui::InputText("##Main Input Comment", the_file_input_name, IM_ARRAYSIZE(the_file_input_name), ImGuiInputTextFlags_CallbackCharFilter, filnameCleaningFunc);
						//TODO: filenames can have space in them, make sure that's handled
						ImGui::SameLine();

						if (filenameIsLocked) {
							ImGui::EndDisabled();
						}

						if (ImGui::ImageButton("##Lock Icon", filenameIsLocked ? lock_icon_texture : unlock_icon_texture, ImVec2(32.0f, 32.0f))) {
							clear_input_data(filenameIsLocked);
							filenameIsLocked = !filenameIsLocked;
						}

						strcpy(evaluated_input_file_name, ARVT::inputFileName_toCommentSplitterPath(the_file_input_name).c_str());
						ImGui::InputText("##Input Comment Path", evaluated_input_file_name, IM_ARRAYSIZE(evaluated_input_file_name), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);
						ImGui::SameLine();

						if (ImGui::Button("Preview File")) { //TODO: add?: https://github.com/mlabbe/nativefiledialog
							int result = ARVT::copyFileToCStr(evaluated_input_file_name, input_comment_data, IM_ARRAYSIZE(input_comment_data));
							if (result) {
								strcpy(input_comment_data, "error"); //TODO: red text
							}
						}
						if (ImGui::BeginItemTooltip()) {
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							ImGui::TextUnformatted("Display the first 16K characters.");
							ImGui::PopTextWrapPos();
							ImGui::EndTooltip();
						}

						ImGui::Checkbox("Word Wrap##Input Comment", &input_comment_word_wrap);
						//TODO: doesn't seem like word wrap affects TextMultiline so remove this option

						if (input_comment_word_wrap) {
							ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x - 400.f);
						}
						ImGui::InputTextMultiline("##input comment", input_comment_data, IM_ARRAYSIZE(input_comment_data), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_ReadOnly);
						if (input_comment_word_wrap) {
							ImGui::PopTextWrapPos();
						}

						ImGui::TableNextColumn();

						if (!filenameIsLocked) {
							ImGui::BeginDisabled();
						}

						ImGui::SeparatorText("Image Text");

						strcpy(evaluated_input_split_1, ARVT::inputFileName_toCommentTestImagePath_Text(the_file_input_name).c_str());
						strcpy(evaluated_input_split_2, ARVT::inputFileName_toCommentTestImagePath_Speech(the_file_input_name).c_str());

						ImGui::InputText("##Input Split 1 Path", evaluated_input_split_1, 1024, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);
						ImGui::SameLine();
						if (ImGui::Button("Refresh##Input Split 1")) {
							int result = ARVT::copyFileToCStr(evaluated_input_split_1, input_split_1_data, IM_ARRAYSIZE(input_split_1_data));
							if (result) {
								strcpy(input_split_1_data, "error"); //TODO: red text
							}
						}

						//TODO: only fill if split has happened, and remove if file name changes
						ImGui::InputTextMultiline("##Input Split 1 Data", input_split_1_data, IM_ARRAYSIZE(input_split_1_data), ImVec2(-FLT_MIN, 0), ImGuiInputTextFlags_ReadOnly);
						//TODO: there should be a third box for ImageMagick text, which basically just replaces "&" with "&amp;" and other stuff
						if (ImGui::Button("Reveal in File Explorer##Input Split 1")) {
							int result = ARVT::revealFileExplorer(evaluated_input_split_1);
							if (result) {
								strcpy(input_split_2_data, "error"); //TODO: red text
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("↓ Make Copy ↓")) {
							int result = ARVT::copy_file(evaluated_input_split_1, evaluated_input_split_2);
							if (result) {
								strcpy(input_split_2_data, "error copying"); //TODO: red text
							} else {
								int result = ARVT::copyFileToCStr(evaluated_input_split_2, input_split_2_data, IM_ARRAYSIZE(input_split_2_data));
								if (result) {
									strcpy(input_split_2_data, "error"); //TODO: red text
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

						ImGui::InputText("##Input Split 2 Path", evaluated_input_split_2, IM_ARRAYSIZE(evaluated_input_split_2), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);
						ImGui::SameLine();
						if (ImGui::Button("Refresh##Input Split 2")) {
							int result = ARVT::copyFileToCStr(evaluated_input_split_2, input_split_2_data, IM_ARRAYSIZE(input_split_2_data));
							if (result) {
								strcpy(input_split_2_data, "error"); //TODO: red text
							}
						}

						ImGui::InputTextMultiline("##Input Split 2 Data", input_split_2_data, IM_ARRAYSIZE(input_split_2_data), ImVec2(-FLT_MIN, 0), ImGuiInputTextFlags_ReadOnly);
						if (ImGui::Button("Reveal in File Explorer##Input Split 2")) {
							int result = ARVT::revealFileExplorer(evaluated_input_split_2);
							if (result) {
								strcpy(input_split_2_data, "error"); //TODO: red text
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

						ImGui::SeparatorText("Font"); //TODO: align these to center
						ImGui::InputText("Font Size",             idata.font_size_input,           IM_ARRAYSIZE(idata.font_size_input),        ImGuiInputTextFlags_CharsDecimal);
						ImGui::InputText("Font Color",            idata.font_color_input,          IM_ARRAYSIZE(idata.font_color_input),       ImGuiInputTextFlags_CallbackCharFilter, quoteScrubbingFunc);
						ImGui::InputText("Background Color",      idata.background_color_input,    IM_ARRAYSIZE(idata.background_color_input), ImGuiInputTextFlags_CallbackCharFilter, quoteScrubbingFunc);
						ImGui::InputText("Paragraph Separator",   idata.paragraph_separator_input, IM_ARRAYSIZE(idata.paragraph_separator_input), 0); //TODO: when reading from this, make sure to replace " with \"
						ImGui::PopItemWidth();

						//TODO: export/import settings (use INI file)

						ImGui::SeparatorText("##Image Parameters button separator");
						ImGui::Combo("Image Format", &idata.imageFormatArray_current, idata.imageFormatArray, IM_ARRAYSIZE(idata.imageFormatArray));

						strcpy(evaluated_test_image_path, ARVT::inputFileName_toCommentTestImagePath_TestImage(the_file_input_name, idata.imageFormatArray[idata.imageFormatArray_current]).c_str());
						ImGui::InputText("##Test Image Path", evaluated_test_image_path, IM_ARRAYSIZE(evaluated_test_image_path), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_ElideLeft);

						if (ImGui::Button("Refresh →", ImVec2(-FLT_MIN, 0.0f))) {
							ARVT::call_comment_test_image(evaluated_input_split_1, evaluated_test_image_path, idata);
							//TODO: check if success
							ret = LoadTextureFromFile(evaluated_test_image_path, &my_image_texture, &my_image_width, &my_image_height);
						}

						if (!filenameIsLocked) {
							ImGui::EndDisabled();
						}

						ImGui::SeparatorText("Video Settings (optional)");
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::InputText("Video Replacement", vdata.video_replacement_numbers_input, IM_ARRAYSIZE(vdata.video_replacement_numbers_input), ImGuiInputTextFlags_CallbackCharFilter, video_replacement_scrubbingFunc);
						ImGui::SameLine();
						HelpMarker("ex. \"1,2,3\" or \"4,6-8,15,20-30\"");
						ImGui::PopItemWidth();
						ImGui::Checkbox("Audio Only (.wav)", &vdata.audio_only_option_input); //TODO: put a textbox for the path to videos (which will change its extension based on this setting)
						if (adata.voiceArray_current < 0) {
							ImGui::TextColored(ImVec4(1, 0, 0, 1), "You haven't set a voice yet!\nGo to the Configure tab.");
							//TODO: lock stuff
						}

						ImGui::TableNextColumn();
						ImGui::Image((ImTextureID)(intptr_t)my_image_texture, ImVec2(ImGui::GetContentRegionAvail().x, (ImGui::GetContentRegionAvail().x / my_image_width) * my_image_height)); //TODO: figure out whether to scale to x or y

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						if (!filenameIsLocked) {
							ImGui::BeginDisabled();
						}

						if (ImGui::Button("Split!", ImVec2(-FLT_MIN, 0.0f))) {
							ARVT::call_comment_splitter(evaluated_input_file_name, ARVT::inputFileName_toCommentTestImagePath_Text(the_file_input_name).c_str());
							ARVT::copyFileToCStr(ARVT::inputFileName_toCommentTestImagePath_Text(the_file_input_name).c_str(), input_split_1_data, IM_ARRAYSIZE(input_split_1_data));
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

						if (ImGui::Button("GO!", ImVec2(-FLT_MIN, 0.0f))) {
							//TODO: progress bar and async
							//TODO: this needs to be much cleaner (probably remove the name function calls), use the data structs instead
							ARVT::call_comment_to_speech(evaluated_input_split_1, evaluated_input_split_2,
								vdata.audio_only_option_input ? ARVT::inputFileName_toCommentToSpeechPath_AudioOnly(the_file_input_name).c_str()
									: ARVT::inputFileName_toCommentToSpeechPath(the_file_input_name, vdata.videoContainerArray[vdata.videoContainerArray_current]).c_str(),
								idata, adata, vdata);
							//TODO: at program start-up, check programs' existence and maybe ffmpeg version
						}
						if (ImGui::Button("Reveal in File Explorer##final video", ImVec2(-FLT_MIN, 0.0f))) {
							//TODO: this should open in the folder if the file doesn't exist
							//yes it's *kinda* a hack to open on just the first video, but it's better than iterating through every file in the folder and checking what's available
							int result = ARVT::revealFileExplorer(ARVT::inputFileName_toCommentToSpeechPath_getFileExplorerName(the_file_input_name, vdata.videoContainerArray[vdata.videoContainerArray_current], vdata.audio_only_option_input).c_str());
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
					if (ImGui::BeginTable("table1", 3, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_PadOuterX)) {
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
						ImGui::SameLine();
						if (ImGui::Button("Refresh🔃U+1F503\xf0\x9f\x94\x83")) {
							adata.update_voiceArray();
						}

						ImGui::Combo("Audio Encoder", &adata.audioEncoderArray_current, adata.audioEncoderArray, IM_ARRAYSIZE(adata.audioEncoderArray));

						ImGui::TableNextColumn();

						ImGui::SliderScalar("FPS", ImGuiDataType_S8, &vdata.fps_v, &vdata.fps_min, &vdata.fps_max); //TODO: option between common values and custom fraction

						if (ImGui::Combo("Video Encoder", &vdata.videoEncoderArray_current, vdata.videoEncoderArray, IM_ARRAYSIZE(vdata.videoEncoderArray))) {
							vdata.update_videoCrfValues();
						}

						ImGui::Indent();
						//TODO (disable and completely swap out on different codecs)
						ImGui::Combo("Preset", &vdata.videoPresetArray_current, vdata.videoPresetArray, IM_ARRAYSIZE(vdata.videoPresetArray), IM_ARRAYSIZE(vdata.videoPresetArray)); //doesn't take flags like ImGuiComboFlags_HeightLargest

						ImGui::SliderScalar("CRF", ImGuiDataType_S8, &vdata.crf_v, &vdata.crf_min, &vdata.crf_max);
						//ImGui::SameLine();
						//HelpMarker("CTRL+Click to input a value.");
						ImGui::Unindent();

						ImGui::Combo("Container", &vdata.videoContainerArray_current, vdata.videoContainerArray, IM_ARRAYSIZE(vdata.videoContainerArray));

						ImGui::Indent();
						if (!vdata.get_faststart_available()) { ImGui::BeginDisabled(); }
						ImGui::Checkbox("-movflags=+faststart", &vdata.faststart_flag);
						ImGui::SameLine();
						HelpMarker("Moves the \"start\" information from the end to the front.\n"
						           "Makes starting playing the video faster but unnecessary for this program's typical usage.");
						if (!vdata.get_faststart_available()) { ImGui::EndDisabled(); }
						ImGui::Unindent();

						ImGui::TableNextColumn();

						ImGui::SeparatorText("Paths");
						ImGui::Text("TODO: three main dirs and a temp dir");
						//other TODO: display what the commands will be (though maybe this should be in the main section?)

						ImGui::SeparatorText("Misc");

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.3f);
						ImGui::Combo("File Delete Age", &imageDeleteAgeList_current, imageDeleteAgeList, IM_ARRAYSIZE(imageDeleteAgeList));
						ImGui::PopItemWidth();
						ImGui::SameLine();
						if (ImGui::Button("Delete files")) {
							int result = ARVT::deleteAllOldFiles(ARVT::OUTPUT_SPEECH.c_str(), imageDeleteAgeList_values[imageDeleteAgeList_current]);
							if (result) {
								//TODO
							}
						}

						ImGui::Text("TODO: reset all to default button");

						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				/*
				if (ImGui::BeginTabItem("SSH", nullptr, tab_flags[3])) {
					ImGui::Text("TODO");
					ImGui::EndTabItem();
				}
				*/

				if (ImGui::BeginTabItem("Help", nullptr, tab_flags[2])) {
					ImGui::Text("TODO");
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("About", nullptr, tab_flags[4])) {
					ImGui::Text("License: GNU General Public License v3.0");
					ImGui::Text("SPDX-License-Identifier: GPL-3.0-only");
					ImGui::Text("Requirements: TODO");
					ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			ImGui::End();
		}


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
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
