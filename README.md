# (Semi-)Automated Reddit Video Tool with a GUI [post-BETA ("GAMMA"?)]

**NOTE**: This software is in a post-beta ("gamma") state. Some things don't work without explanations or warnings (sometimes crashing), but there shouldn't be anything that seriously breaks.

A program that takes text input and can generate the sentence-by-sentence reading and text-to-speech stuff that's common in Reddit reading videos. Now with a GUI!

This program does not automate the entire process of making a "Reddit reading" video, because the intent is for you to take the clips it generates and add your own special flare to the video. I prefer quality over quantity so that's what this program is designed for.

## Usage

![readme video](readme-video-alpha.gif)

1. Obtain the comment you want to make a video for as a .txt file. Put that file in the `input_comments` folder.
1. Type in the file name in the "File Name" section. Click "Preview File" to make sure you got the right file.
1. **Click the lock icon**, then click "Split!" to have it split.
1. (Optional) If you want to have the speech and image text be different, click "Make Copy" to make a copy of it, then "Reveal in File Explorer" to find it, then edit it as you wish. Check "Use Speech Text" to enable using it.
1. (Optional, strongly recommended) Edit the image parameters as you desire. Click "Preview" to see your changes.
1. Click "GO!" to make the videos!
1. Click "Reveal in File Explorer" to reveal the files' location.
1. Throw those videos into your favorite video editor and do what you want!

## System Requirements

The minmums listed here are "recommended" minimums, not true minimums. Anything lower might work but no guarantees.

Windows:

* OS: Windows 10
* GPU: OpenGL 3 compatible

Linux: Currently not supported. Will be added later.

Mac OS: Not supported. Source code is right here though, so feel free to try your luck.

## Program Requirements

### Pre-built binaries (TODO)

Note that these programs must be accessible from a plain command line, meaning they must be local to wherever this program is running or be on the system PATH.

* Python 3.8+
* ImageMagick
* Balabolka or eSpeak
* FFmpeg 4.4+ (corresponds to Ubuntu 22.04 (Jammy Jellyfish) or higher)

Place `balcon.exe` in this project's root folder (if you plan on using Balabolka).

### Compiling from source (Windows)

Currently only Windows x64 with MSVC 2022 is supported.

* Enable the MSVC environment variables in your command prompt: `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`
* `cd imgui-application`
* `build_win64.bat`
* The executable is in the `Debug` folder (don't `cd`).

### Compiling from source (Linux)

Linux support is WIP. Only tested on Ubuntu.

* get a compiler and CMake
* `sudo apt install libglfw3-dev`
* `mkdir build && cd build`
* `cmake ..`
* `./arvt-gui`
* in the Python scripts, change `.temp` extensions to `.txt` (will fix this step later)
* unicode doesn't show in imgui, fix later

ImageMagick is extremely picky, so you need to fix some of its security policies. Edit `/etc/ImageMagick-6/policy.xml` with the following changes:

* Fix this comment not ending, just so the syntax highlighter stops being confused: `<!-- <policy domain="cache" name="shared-secret" value="passphrase" stealth="true"/>`
* add: `<policy domain="coder" rights="read|write" pattern="{GIF,JPEG,PNG,WEBP}" />`
* add: `<policy domain="coder" rights="read|write" pattern="PANGO" />`
* add: `<policy domain="path" rights="read" pattern="*" />` (might not be needed)
* remove conflicting policies (mainly this: `<policy domain="path" rights="none" pattern="@*"/>`)

Currently can't execute an ImageMagick command because `pango@*` is treated like a file instead of telling pango to open the file. Will hopefully fix later (and if not, then no Linux support... unless compiling ImageMagick from source is different).

## TODO list

* ~~make all the options do something~~
* *robustness*
* ImGui window fullscreen
* support for other platforms and compilers (use Cmake)
* ClangFormat
* Doxygen
* an INI file for saving and loading settings
* support for SSH-ing into a virtual machine (don't count on it) (plus pack up files using zstd)
* option to embed subtitles in the videos (would anyone use it?) (supposedly needs a .mkv but it seems to work in a .mp4)
* unit tests (GTest)
* ~~other font options~~ (should be a bit easier though)

## Audio-only note

Kdenlive seems to have trouble correctly detecting the length of the .wav files produced with audio-only mode. Simply reload/refresh the clips (after a restart?) to fix this.

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
