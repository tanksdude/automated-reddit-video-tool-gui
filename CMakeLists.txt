cmake_minimum_required(VERSION 3.16)
project(arvt-gui VERSION 0.3.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED False)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
#find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(${PROJECT_NAME}
	imgui-application/helpers.cpp
	imgui-application/audio_data.cpp
	imgui-application/video_data.cpp
	imgui-application/main.cpp
	imgui-application/imgui/imgui_impl_glfw.cpp
	imgui-application/imgui/imgui_impl_opengl3.cpp
	imgui-application/imgui/imconfig.h
	imgui-application/imgui/imgui_demo.cpp
	imgui-application/imgui/imgui_draw.cpp
	imgui-application/imgui/imgui_impl_glfw.cpp
	imgui-application/imgui/imgui_impl_glfw.h
	imgui-application/imgui/imgui_impl_opengl3_loader.h
	imgui-application/imgui/imgui_impl_opengl3.cpp
	imgui-application/imgui/imgui_impl_opengl3.h
	imgui-application/imgui/imgui_internal.h
	imgui-application/imgui/imgui_tables.cpp
	imgui-application/imgui/imgui_widgets.cpp
	imgui-application/imgui/imgui.cpp
	imgui-application/imgui/imgui.h
	imgui-application/imgui/imstb_rectpack.h
	imgui-application/imgui/imstb_textedit.h
	imgui-application/imgui/imstb_truetype.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE GL)
#target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::glew)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw ${GLEW_LIBRARIES})
