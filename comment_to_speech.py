import os, sys
import subprocess
import argparse
import time
import platform

# python comment_to_speech.py input_text.txt [-s input_speech] output_speech/vid_$.mp4 [-n replace] [-a] VIDEO_PARAMTERS

audioProgramLookup = None
MAGICK_CMD = None
if platform.system() == "Windows":
	audioProgramLookup = {
		"Balabolka": "balcon",
		"Espeak":    "espeak",
		"Espeak NG": "espeak-ng",
	}
	MAGICK_CMD = "magick"
elif platform.system() == "Darwin":
	sys.exit("Mac OS not supported")
elif platform.system() == "Linux" or platform.system() in ["FreeBSD", "OpenBSD", "NetBSD"]:
	audioProgramLookup = {
		"Espeak":    "espeak",
		"Espeak NG": "espeak-ng",
	}
	MAGICK_CMD = "convert"
else:
	# Jython and mobile devices
	sys.exit("Unknown/unsupported platform")
# checking the OS can also be done with os.uname().sysname

textAlignmentArgsLookup = {
	"default":   [],
	"left":      ["-define", "pango:align=left"],
	"center":    ["-define", "pango:align=center"],
	"right":     ["-define", "pango:align=right"],
	"justify":   ["-define", "pango:justify=true"], # possible to combine align and justify
	# pango options: https://imagemagick.org/script/formats.php
}

audioCodecLookup = {
	"copy":     "copy",
	"AAC":      "aac",
	"Opus":     "libopus",
	"FLAC":     "flac",
	"Vorbis":   "libvorbis",
	"MP3":      "libmp3lame",
	"ALAC":     "alac",

	"AC-3":     "ac3",
	"E-AC-3":   "eac3",
	"Speex":    "libspeex",
	"TTA":      "tta",
	"Windows":  "wmav2",
	"MPEG-2":   "libtwolame", # mp2 also exists, though it only does 160k bitrate; ffprobe reports this as mp3 either way
}
audioCodecIsLosslessList = ["copy", "FLAC", "ALAC", "TTA"]

audioPresetKeywordLookup = {
	"copy":     [],
	"AAC":      ["-aac_coder"], # enabling anmr requires -strict experimental
	"Opus":     ["-compression_level"],
	"FLAC":     ["-compression_level"],
	"Vorbis":   [],
	"MP3":      ["-compression_level"],
	"ALAC":     [],

	"AC-3":     [],
	"E-AC-3":   [],
	"Speex":    [],
	"TTA":      [],
	"Windows":  [],
	"MPEG-2":   [],
}

videoCodecLookup = {
	"H.264":   "libx264",
	"H.265":   "libx265",
	"VP8":     "libvpx",
	"VP9":     "libvpx-vp9",
	"AV1":     "libaom-av1",
	"FFV1":    "ffv1",
	"Ut":      "utvideo",

	"Apple":            "prores",
	"QuickTime":        "qtrle",
	"CineForm":         "cfhd",
	"Lossless_H.264":   "libx264", #TODO: libx264rgb?
	"VVC":              "libvvenc",
	"EVC":              "libxeve",
}
videoCodecIsLosslessList = ["FFV1", "Ut", "QuickTime", "Lossless_H.264"] + ["Apple", "CineForm", "VVC"] # The second list isn't lossless, but they don't support CRF mode

videoPresetKeywordLookup = {
	"H.264":   ["-preset"],
	"H.265":   ["-preset"],
	"VP8":     ["-deadline", "-cpu-used"],
	"VP9":     ["-deadline", "-cpu-used"],
	"AV1":     ["-usage",    "-cpu-used"],
	"FFV1":    [],
	"Ut":      ["-pred"],

	"Apple":            [],
	"QuickTime":        [],
	"CineForm":         ["-quality"],
	"Lossless_H.264":   ["-preset"],
	"VVC":              ["-preset"],
	"EVC":              ["-preset"],
}

videoExtraArgsLookup = {
	"H.264":   [],
	"H.265":   ["-x265-params", "log-level=error"],     # silence unnecessary output, as FFmpeg does not tell libx265 the log level
	"VP8":     ["-b:v", "1G", "-auto-alt-ref", "0" ],   # bitrate flag possibly required: https://goughlui.com/2023/12/27/video-codec-round-up-2023-part-4-libvpx-vp8/
	"VP9":     ["-b:v", "0" ],                          # definitely required (for CRF mode)
	"AV1":     [],                                      # requires FFmpeg 4.3+ to not require "-b:v 0", 4.4+ to avoid a lossless bug
	"FFV1":    [],                                      # no need for -level 3 as that's the default (found empirically, required ffprobe -debug 1)
	"Ut":      [],

	"Apple":            [],
	"QuickTime":        [],
	"CineForm":         [],
	"Lossless_H.264":   ["-crf", "0"], # not needed if the codec is removed from videoCodecIsLosslessList, however ["-qp", "0"] might be preferred
	"VVC":              [],
	"EVC":              [],
}

#print(sys.argv[1:]);

parser = argparse.ArgumentParser()
parser.add_argument("input_text_file", help="text to show on screen")
parser.add_argument("-s", "--input_speech_file", metavar="input_speech_file", required=False, help="text to read aloud")
parser.add_argument("output_mp4_files", help="output video files (needs a '$' in its name)")
parser.add_argument("-a", "--audio_only", required=False, help="only output audio files, no video", action="store_true")

parser.add_argument("image_width_input")
parser.add_argument("image_height_input")
parser.add_argument("image_w_border_input")
parser.add_argument("image_h_border_input")
parser.add_argument("font_size_input")
parser.add_argument("font_color_input")
parser.add_argument("background_color_input")
parser.add_argument("paragraph_newline_count")
parser.add_argument("paragraph_tabbed_start")
parser.add_argument("font_name")
parser.add_argument("font_is_family")
parser.add_argument("text_alignment")
parser.add_argument("skip_lf_line")
parser.add_argument("imageFormat")

parser.add_argument("-n", "--video_replacement_numbers", metavar="video_replacement_numbers", required=False, help="update/generate a specific video")
parser.add_argument("speechEngine")
parser.add_argument("voice")
parser.add_argument("audioEncoder")
parser.add_argument("audioBitrate")
parser.add_argument("audioPreset")
parser.add_argument("videoEncoder")
parser.add_argument("videoPreset1")
parser.add_argument("videoPreset2")
parser.add_argument("faststart_flag")
parser.add_argument("fps")
parser.add_argument("crf")
#TODO: option to erase all old videos (of the same project name)? needed to "replace" one container with another
args = parser.parse_args()

# Image parameters:
IMAGE_W_BORDER = int(args.image_w_border_input)
IMAGE_H_BORDER = int(args.image_h_border_input)
IMAGE_WIDTH = int(args.image_width_input) - 2*IMAGE_W_BORDER
IMAGE_HEIGHT = int(args.image_height_input) - 2*IMAGE_H_BORDER
IMAGE_FONT_SIZE = args.font_size_input
IMAGE_FONT_COLOR = args.font_color_input
IMAGE_BACKGROUND_COLOR = args.background_color_input
IMAGE_PARAGRAPH_SEP = "\n" * int(args.paragraph_newline_count)
IMAGE_PARAGRAPH_START = "\t" if int(args.paragraph_tabbed_start) else ""
IMAGE_FONT_NAME = args.font_name
IMAGE_FONT_IS_FAMILY = int(args.font_is_family)
IMAGE_TEXT_ALIGN_ARGS = textAlignmentArgsLookup[args.text_alignment]
IMAGE_SKIP_LF_LINE = int(args.skip_lf_line)
IMAGE_FORMAT = args.imageFormat
# evaluated image parameters:
IMAGE_SIZE = str(IMAGE_WIDTH) + "x" + str(IMAGE_HEIGHT)
IMAGE_SIZE_EXTENDED = str(IMAGE_WIDTH + 2*IMAGE_W_BORDER) + "x" + str(IMAGE_HEIGHT + 2*IMAGE_H_BORDER)

# Video parameters:
VIDEO_AUD_CODEC_name = args.audioEncoder.split(' ')[0]
VIDEO_VID_CODEC_name = args.videoEncoder.split(' ')[0]
AUDIO_PROGRAM_CMD = audioProgramLookup[args.speechEngine]
AUDIO_VOICE = args.voice
VIDEO_FPS = args.fps # Fun fact: FFmpeg's default framerate is 25. Only answer I could find for why is PAL compatibility, but I think that's just a coincidence.
VIDEO_VID_CRF = args.crf
VIDEO_AUD_CODEC_lib = audioCodecLookup[VIDEO_AUD_CODEC_name]
VIDEO_AUD_BITRATE = args.audioBitrate
VIDEO_AUD_PRESET = args.audioPreset.split(' ')[0]
VIDEO_VID_CODEC_lib = videoCodecLookup[VIDEO_VID_CODEC_name]
VIDEO_VID_PRESET_1 = args.videoPreset1.split(' ')[0]
VIDEO_VID_PRESET_2 = args.videoPreset2.split(' ')[0]
VIDEO_VID_EXTRA_ARGS = videoExtraArgsLookup[VIDEO_VID_CODEC_name]
VIDEO_VID_FASTSTART = int(args.faststart_flag)

input_image_text_file_path = args.input_text_file
output_vid_file_path = args.output_mp4_files
if output_vid_file_path.find('$') == -1:
	sys.exit("Bad output vid file names")
input_speech_text_file_path = args.input_speech_file
AUDIO_ONLY = args.audio_only

speech_and_image_to_vid_command_args = []
# FFmpeg video args
speech_and_image_to_vid_command_args.extend(["-c:v", VIDEO_VID_CODEC_lib])
speech_and_image_to_vid_command_args.extend(VIDEO_VID_EXTRA_ARGS)
if VIDEO_VID_PRESET_1 != "default":
	speech_and_image_to_vid_command_args.extend([videoPresetKeywordLookup[VIDEO_VID_CODEC_name][0], VIDEO_VID_PRESET_1])
if VIDEO_VID_PRESET_2 != "default":
	speech_and_image_to_vid_command_args.extend([videoPresetKeywordLookup[VIDEO_VID_CODEC_name][1], VIDEO_VID_PRESET_2])
speech_and_image_to_vid_command_args.extend(["-r", VIDEO_FPS])
if VIDEO_VID_CODEC_name not in videoCodecIsLosslessList:
	# unnecessary check as CRF is ignored for lossless codecs; mainly used for the codecs that don't support CRF mode
	speech_and_image_to_vid_command_args.extend(["-crf", VIDEO_VID_CRF])
if VIDEO_VID_FASTSTART:
	speech_and_image_to_vid_command_args.extend(["-movflags", "+faststart"])
# FFmpeg audio args
speech_and_image_to_vid_command_args.extend(["-c:a", VIDEO_AUD_CODEC_lib])
if VIDEO_AUD_CODEC_name not in audioCodecIsLosslessList:
	# unnecessary check as bitrate is ignored for lossless codecs
	speech_and_image_to_vid_command_args.extend(["-b:a", VIDEO_AUD_BITRATE])
if VIDEO_AUD_PRESET != "default":
	speech_and_image_to_vid_command_args.extend([audioPresetKeywordLookup[VIDEO_AUD_CODEC_name][0], VIDEO_AUD_PRESET])
# FFmpeg other args
speech_and_image_to_vid_command_args.extend(["-loglevel", "error", "-y"]) # loglevels: quiet, fatal, error, warning; note that libx265 needs this set separately: none, error, warning

def text_to_speech_func_balabolka(wav_file_name, text_file_name):
	# make sure to do -w arg before the -f arg, because sometimes it just won't write to a wav file otherwise
	return subprocess.run([AUDIO_PROGRAM_CMD, "-n", AUDIO_VOICE, "-enc", "utf8", "-w", wav_file_name, "-f", text_file_name])
def text_to_speech_func_espeak(wav_file_name, text_file_name):
	# Voice names have spaces replaced with underscores, so undo that to make the voice work
	# Theoretically there could be a voice that has underscores *and* spaces, but that let's say that's user error because that's above my pay grade to handle
	return subprocess.run([AUDIO_PROGRAM_CMD, "-v", AUDIO_VOICE.replace("_", " "), "-w", wav_file_name, "-f", text_file_name])

ttsFunctionLookup = {
	"Balabolka": text_to_speech_func_balabolka,
	"Espeak":    text_to_speech_func_espeak,
	"Espeak NG": text_to_speech_func_espeak,
}

text_to_speech_func = ttsFunctionLookup[args.speechEngine]

def text_to_image_func(img_file_name, text_file_name):
	command_args = [MAGICK_CMD, "-size", IMAGE_SIZE, "-background", IMAGE_BACKGROUND_COLOR, "-fill", IMAGE_FONT_COLOR, "-pointsize", IMAGE_FONT_SIZE]
	if IMAGE_FONT_IS_FAMILY:
		command_args.extend(["-family", IMAGE_FONT_NAME])
	else:
		command_args.extend(["-font", IMAGE_FONT_NAME])
	command_args.extend(IMAGE_TEXT_ALIGN_ARGS)
	command_args.extend(["pango:@" + text_file_name, "-gravity", "center", "-extent", IMAGE_SIZE_EXTENDED, img_file_name])
	return subprocess.run(command_args)
	# https://imagemagick.org/Usage/text/#caption

def speech_and_image_to_vid_func(vid_file_name, wav_file_name, img_file_name):
	command_args = ["ffmpeg", "-i", wav_file_name, "-i", img_file_name]
	command_args.extend(speech_and_image_to_vid_command_args)
	command_args.append(vid_file_name)
	return subprocess.run(command_args)
	# https://ffmpeg.org/ffmpeg.html#Main-options

def gen_output_vid_file_path(num):
	return output_vid_file_path.replace("$", str(num))

def gen_output_wav_file_path_audio_only(num):
	return output_vid_file_path.replace("$", str(num)) # if it's audio-only, then ".wav" is already in the file path
def gen_output_wav_file_path_regular(num):
	return output_vid_file_path.replace("$", str(num)) + ".wav"

gen_output_wav_file_path = gen_output_wav_file_path_audio_only if AUDIO_ONLY else gen_output_wav_file_path_regular

def gen_output_img_file_path(num):
	return output_vid_file_path.replace("$", str(num)) + IMAGE_FORMAT

start_time = time.time()

try:
	input_image_text_file = open(input_image_text_file_path, "r", encoding="utf8")
except FileNotFoundError:
	sys.exit("File \"" + input_image_text_file_path + "\" not found!")
except IsADirectoryError:
	sys.exit("\"" + input_image_text_file_path + "\" is a directory; could not read")
except PermissionError:
	sys.exit("Could not read \"" + input_image_text_file_path + "\" due to permissions granted!")
except Exception as e:
	sys.exit("Other error while reading file \"" + input_image_text_file_path + "\": ", e)

image_text_file_lines = input_image_text_file.readlines()
input_image_text_file.close()

speech_text_file_lines = None
if input_speech_text_file_path != None:
	try:
		input_speech_text_file = open(input_speech_text_file_path, "r", encoding="utf8")
	except FileNotFoundError:
		sys.exit("File \"" + input_speech_text_file_path + "\" not found!")
	except IsADirectoryError:
		sys.exit("\"" + input_speech_text_file_path + "\" is a directory; could not read")
	except PermissionError:
		sys.exit("Could not read \"" + input_speech_text_file_path + "\" due to permissions granted!")
	except Exception as e:
		sys.exit("Other error while reading file \"" + input_speech_text_file_path + "\":", e)

	speech_text_file_lines = input_speech_text_file.readlines()
	input_speech_text_file.close()
else:
	speech_text_file_lines = image_text_file_lines

if len(image_text_file_lines) != len(speech_text_file_lines):
	sys.exit("Lines in image text file and speech text file don't match")

video_replacement_set = set() # {} is a dictionary
for nums in args.video_replacement_numbers.split(","):
	numRange = nums.split("-")
	if len(numRange) > 2:
		sys.exit("Bad video replacement list")
	if len(numRange) == 1:
		# no hyphen
		try:
			video_replacement_set.add(int(numRange[0]))
		except (TypeError, ValueError):
			pass
	else:
		lowerBound = upperBound = None
		try:
			lowerBound = int(numRange[0])
		except (TypeError, ValueError):
			lowerBound = 1
		try:
			upperBound = int(numRange[1])
		except (TypeError, ValueError):
			upperBound = len(image_text_file_lines) # this can go over the actual number of video files, but it doesn't matter

		for i in range(lowerBound, upperBound+1):
			# not necessary to check for existence in a set before adding to it
			video_replacement_set.add(i)
if len(video_replacement_set) == 0:
	# empty arg, so replace/make every video
	for i in range(1, len(image_text_file_lines)+1):
		video_replacement_set.add(i)

files_count = 0 # for the vid_$.mp4 file; the file number won't match the line number
replaced_files_count = 0 #TODO: this is a pretty hacky solution
#print(len([line for line in speech_text_file_lines if len(line) > 1])) #TODO: does this always get the file count?
curr_text_file_read = IMAGE_PARAGRAPH_START

for i in range(len(image_text_file_lines)):
	speech_line = speech_text_file_lines[i]
	image_line = image_text_file_lines[i]
	speech_line = speech_line[0:-1] # every line should end in a \n
	image_line = image_line[0:-1]
	if IMAGE_SKIP_LF_LINE and image_line == "\\n":
		continue
	if len(speech_line) == 0:
		curr_text_file_read += IMAGE_PARAGRAPH_SEP + IMAGE_PARAGRAPH_START
		continue
	curr_text_file_read += image_line

	files_count += 1

	if files_count in video_replacement_set:
		# speech file:
		output_file = open(gen_output_wav_file_path(files_count)+".txt", "w", encoding="utf8")
		output_file.write(speech_line)
		output_file.close()

		result = text_to_speech_func(gen_output_wav_file_path(files_count), gen_output_wav_file_path(files_count)+".txt")
		os.remove(gen_output_wav_file_path(files_count)+".txt")

		if result.returncode:
			sys.exit("ERROR: Could not generate the audio file for video " + str(files_count))

		if not AUDIO_ONLY:
			# image file:
			output_file = open(gen_output_img_file_path(files_count)+".txt", "w", encoding="utf8")
			output_file.write(curr_text_file_read)
			output_file.close()

			result = text_to_image_func(gen_output_img_file_path(files_count), gen_output_img_file_path(files_count)+".txt")
			os.remove(gen_output_img_file_path(files_count)+".txt")

			if result.returncode:
				os.remove(gen_output_wav_file_path(files_count))
				sys.exit("ERROR: Could not generate the image file for video " + str(files_count))

			# video:
			result = speech_and_image_to_vid_func(gen_output_vid_file_path(files_count), gen_output_wav_file_path(files_count), gen_output_img_file_path(files_count))

			# cleanup:
			os.remove(gen_output_wav_file_path(files_count))
			os.remove(gen_output_img_file_path(files_count))

			if result.returncode:
				sys.exit("ERROR: Could not generate the video file for video " + str(files_count))

		replaced_files_count += 1

end_time = time.time()
if replaced_files_count == files_count:
	print(f"Made {files_count} videos in {(end_time - start_time):.3f}s")
else:
	print(f"Replaced {replaced_files_count} videos in {(end_time - start_time):.3f}s")
	#TODO: state which videos were replaced?
