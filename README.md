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

Windows:

* OS: Windows 10
* GPU: OpenGL 3 compatible

Linux:

* GPU: OpenGL 3 compatible

Mac OS: Not supported. Source code is right here though, so feel free to try your luck.

## Program Requirements

Note that these programs must be accessible from a plain command line, meaning they must be local to wherever this program is running or be on the system PATH.

* Python 3.8+
* ImageMagick (Linux: see below for issues)
* Windows: Balabolka or eSpeak | Linux: eSpeak
* FFmpeg 4.4+ (corresponds to Ubuntu 22.04 (Jammy Jellyfish) or higher)

### Compiling from source (Windows)

Currently only Windows x64 with MSVC 2022 is officially supported.

1. Enable the MSVC environment variables in your command prompt: `"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"`
1. `cd imgui-application`
1. `build_win64.bat`
1. The executable is in the `Debug` folder (don't `cd`).

[Pre-compiled executables](https://github.com/tanksdude/automated-reddit-video-tool-gui/releases) are provided for Windows.

### Compiling from source (Linux)

Linux mostly works. Only tested on Ubuntu.

1. Prerequisite: `sudo apt install libglfw3-dev`
1. `cd imgui-application`
1. `make -j$(nproc)`
1. The executable is in the same folder

I have spent many hours trying to get ImageMagick to work. If you want something that "just works", you should [compile from source](https://imagemagick.org/script/install-source.php#linux), as you won't have to worry about all sorts of stupid permission issues. If you want to use your distro's package manager, this is the best I could come up with:

* Edit `/etc/ImageMagick-<6 or 7 or whatever>/policy.xml` with the following changes:
* add: `<policy domain="coder" rights="read|write" pattern="{GIF,JPEG,PNG,WEBP,TXT}" />`
* add: `<policy domain="delegate" rights="read|write" pattern="{GIF,JPEG,PNG,WEBP,TXT}" />`
* add: `<policy domain="module" rights="read|write" pattern="{GIF,JPEG,PNG,WEBP,TXT}" />`
* add: `<policy domain="filter" rights="read|write" pattern="{GIF,JPEG,PNG,WEBP,TXT}" />`
* add: `<policy domain="coder" rights="read|write" pattern="PANGO" />` (this is supposedly the only "officially" required part)
* remove conflicting policies (mainly this: `<policy domain="path" rights="none" pattern="@*"/>` (this might be the only thing that *has* to be changed))
* In ImageMagick 6, this doesn't actually work, despite what documentation I could find saying this should be more than enough. In ImageMagick 7, the background color works just fine, but not the text color.

If you know how to get it working, please do share, because you would probably be the only person on the Internet that knows how to fix this issue. Which I find exceedingly strange no one has documented a fix for this, because if ImageMagick is truly as critical to infrastructure as [this xkcd comic claims (read the alt text)](https://xkcd.com/2347/), then it has to be running on a lot more Linux machines than Windows, so this should be a very-well documented solution.

## Option list (WIP)

By default, this program uses H.264 and AAC codecs by default for high compatibility, however these are not the "recommended" codecs. You will probably want lossless codecs, especially when the filesize increase is rather small. I recommend changing H.264 to FFV1, or Ut Video if your video editor does not support FFV1, and change AAC to FLAC.

## TODO list

* ~~make all the options do something~~
* *robustness*
* handle cmd errors and python errors
* ~~ImGui window fullscreen~~
* support for other platforms and compilers
* ClangFormat
* Doxygen
* an INI file for saving and loading settings
* support for SSH-ing into a virtual machine (don't count on it) (plus pack up files using zstd)
* option to embed subtitles in the videos (would anyone use it?) (supposedly needs a .mkv but it seems to work in a .mp4)
* unit tests (GTest)
* ~~other font options~~

## Audio-only note

Kdenlive seems to have trouble correctly detecting the length of the .wav files produced with audio-only mode. Simply reload/refresh the clips (after a restart?) to fix this.

## Example usage

TODO

## License

GNU General Public License v3.0

### Externals' licenses

* [ImGui](https://github.com/ocornut/imgui): MIT
* [GLFW](https://www.glfw.org/): zlib
* [stb_image](https://github.com/nothings/stb): MIT or The Unlicense
* [mINI](https://github.com/metayeti/mINI): MIT

## Acknowledgments

* Reddit readings
* [The Cherno](https://www.youtube.com/@TheCherno/videos) for exposing me to ImGui's ease of use
    * I was planning on using Qt for the GUI, but for some stupid reason it requires making an account. There were apparently ways to get around that in Qt5, but not anymore in Qt6. Pretty much every project I know uses Qt, but ImGui is a far less painful setup process so I just had to choose ImGui.
