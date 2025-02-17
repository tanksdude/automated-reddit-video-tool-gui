@REM Build for Visual Studio compiler. Run your copy of vcvars32.bat or vcvarsall.bat to setup command-line compiler.
@set OUT_DIR=Debug
@set OUT_EXE=example_glfw_opengl3
@set INCLUDES=/I.\libs\glfw\include
@set SOURCES=helpers.cpp audio_data.cpp image_data.cpp video_data.cpp main.cpp imgui\imgui_impl_glfw.cpp imgui\imgui_impl_opengl3.cpp imgui\imgui*.cpp
@REM main.cpp must be last, so the classes can initialize their static members
@set LIBS=/LIBPATH:libs\glfw\lib-vc2022-64 glfw3.lib opengl32.lib gdi32.lib shell32.lib
Taskkill /IM example_glfw_opengl3.exe /F
mkdir %OUT_DIR%
cl /std:c++20 /nologo /Zi /MD /utf-8 %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS% /SUBSYSTEM:windows /ENTRY:mainCRTStartup
