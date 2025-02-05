# (Semi-)Automated Reddit Video Tool with a GUI [ALPHA]

**WARNING**: This software is in a very alpha state. There are countless things that don't work, and countless more that don't display error messages. Use at your own discretion.

A program that takes text input and can generate the sentence-by-sentence reading and text-to-speech stuff that's common in Reddit reading videos. Now with a GUI!

This program does not automate the entire process of making a "Reddit reading" video, because the intent is for you to take the clips it generates and add your own special flare to the video. I prefer quality over quantity so that's what this program is designed for.

## Usage

![readme video](readme-video-alpha.gif)

1. Obtain the comment you want to make a video for as a .txt file. Put that file in the `input_comments` folder.
1. Type in the file name in the "File Name" section. Click "Preview File" to make sure you got the right file.
1. **Click the lock icon**, then click "Split!" to have it split.
1. (Optional) If you want to have the speech and image text be different, click "Make Copy" to make a copy of it, then "Reveal in File Explorer" to find it, then edit it as you wish.
1. (Optional, strongly recommended) Edit the image parameters as you desire. Click "Refresh" to see your changes.
1. Click "GO!" to make the videos!
1. Click "Reveal in File Explorer" to reveal the files' location.
1. Throw those videos into your favorite video editor and do what you want!

## System Requirements

The minmums listed here are "recommended" minimums, not true minimums. Anything lower might work but no guarantees.

Windows:

* OS: Windows 10
* GPU: OpenGL 3 compatible

Linux: Currently not supported. Will be added later.

Mac OS:

* Mac OS is not supported. Source code is right here though, so feel free to try your luck.

## Program Requirements

### Pre-built binaries (TODO)

Note that these programs must be accessible from a plain command line, meaning they must be local to wherever this program is running or be on the system PATH.

* Python 3.8+
* ImageMagick
* Balabolka
* FFmpeg 4.4+

Place `balcon.exe` in this project's root folder.

### Compiling from source

Currently only Windows x64 with MSVC 2022 is supported.

* `cd imgui-application`
* Enable the MSVC environment variables in your command prompt: `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`
* `build_win64.bat`
* The executable is in the `Debug` folder

## TODO list

* make all the options do something
* *robustness*
* ImGui window fullscreen
* support for other platforms and compilers (use Cmake)
* ClangFormat
* Doxygen
* an INI file for saving and loading settings
* support for SSH-ing into a virtual machine (don't count on it)
* option to embed subtitles in the videos (would anyone use it?)
* unit tests (GTest)

## Options that currently work

* Image settings (except paragraph separator)
* Audio voice
* Video FPS & CRF
* Video container

## Options that kinda work

* File name: cannot have spaces without breaking everything
* Audio encoder: there is no translation from codec name to FFmpeg encoder, so only "copy" works
* Image paragraph separator: only handles `\n` escape character
* Audio only: actually does work, just not the way I want

## Options that don't work

* Word wrap for the input comment
* The speech file option (though creating it does work, it's just not used anywhere)
* Speech engine
* Video encoder & video preset & `-movflags=+faststart`
* Video replacement

## Example usage

TODO

## License

GNU General Public License v3.0

### Externals' licenses

* ImGui: MIT
* GLFW: zlib
* stb_image: MIT or The Unlicense

## Acknowledgments

* Reddit readings
* [The Cherno](https://www.youtube.com/@TheCherno/videos) for exposing me to ImGui's ease of use
    * I was planning on using Qt for the GUI, but for some stupid reason it requires making an account. There were apparently ways to get around that in Qt5, but not anymore in Qt6. Pretty much every project I know uses Qt, but ImGui is a far less painful setup process so I just had to choose ImGui.
