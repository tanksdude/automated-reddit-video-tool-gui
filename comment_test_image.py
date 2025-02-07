import os, sys
import subprocess
import argparse
import time

# python comment_test_image.py input_text test_images/output IMAGE_PARAMETERS

"""
int call_comment_test_image(const char* textPath, const char* output,
	char image_width_input[],
	char image_height_input[],
	char image_w_border_input[],
	char image_h_border_input[],
	char font_size_input[],
	char font_color_input[],
	char background_color_input[],
	char paragraph_separator_input[]
);
"""

def text_to_image_func(img_file_name, text_file_name):
	#return subprocess.run(["magick", "-size", IMAGE_SIZE, "-background", IMAGE_BACKGROUND_COLOR, "-fill", IMAGE_TEXT_COLOR, "-family", "Times New Roman", "-pointsize", IMAGE_FONT_SIZE, "pango:@" + text_file_name, "-gravity", "center", "-extent", IMAGE_SIZE_EXTENDED, img_file_name])
	return subprocess.run(["magick", "-size", IMAGE_SIZE, "-background", IMAGE_BACKGROUND_COLOR, "-fill", IMAGE_TEXT_COLOR, "-font", "Verdana", "-pointsize", IMAGE_FONT_SIZE, "pango:@" + text_file_name, "-gravity", "center", "-extent", IMAGE_SIZE_EXTENDED, img_file_name])
	# https://imagemagick.org/Usage/text/#caption

parser = argparse.ArgumentParser()
parser.add_argument("input_split_comment_file", help="split comment input")
parser.add_argument("output_image_file", help="output image file")

parser.add_argument("image_width_input")
parser.add_argument("image_height_input")
parser.add_argument("image_w_border_input")
parser.add_argument("image_h_border_input")
parser.add_argument("font_size_input")
parser.add_argument("font_color_input")
parser.add_argument("background_color_input")
parser.add_argument("paragraph_separator_input")
# TODO: option for "new paragraph starter" (basically for \t starts)

# TODO: don't force parameter order

args = parser.parse_args()

input_image_text_file_path = args.input_split_comment_file
output_img_file_path = args.output_image_file

# Image parameters:
IMAGE_W_BORDER = int(args.image_w_border_input)
IMAGE_H_BORDER = int(args.image_h_border_input)
IMAGE_WIDTH = int(args.image_width_input) - 2*IMAGE_W_BORDER
IMAGE_HEIGHT = int(args.image_height_input) - 2*IMAGE_H_BORDER
IMAGE_FONT_SIZE = args.font_size_input
IMAGE_TEXT_COLOR = args.font_color_input
IMAGE_BACKGROUND_COLOR = args.background_color_input
IMAGE_NEW_PARAGRAPH_SEP = args.paragraph_separator_input.replace("\\n", "\n") # TODO: properly escape every character
# evaluated image parameters:
IMAGE_SIZE = str(IMAGE_WIDTH) + "x" + str(IMAGE_HEIGHT)
IMAGE_SIZE_EXTENDED = str(IMAGE_WIDTH + 2*IMAGE_W_BORDER) + "x" + str(IMAGE_HEIGHT + 2*IMAGE_H_BORDER)

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
	sys.exit("Other error while reading file \"" + input_image_text_file_path + "\":", e)

image_text_file_lines = input_image_text_file.readlines()
input_image_text_file.close()

# split the sentences into their own files, append them, then convert it to an image:

curr_file_read = ""

for line in image_text_file_lines:
	line = line[0:-1] # every line should end in a \n
	#print(line, end="")
	if len(line) == 0:
		curr_file_read += IMAGE_NEW_PARAGRAPH_SEP
		continue
	curr_file_read += line

output_file = open(output_img_file_path+".temp", "w", encoding="utf8")
output_file.write(curr_file_read)
output_file.close()

result = text_to_image_func(output_img_file_path, output_img_file_path+".temp")
#print(result)
os.remove(output_img_file_path+".temp")

end_time = time.time()
print("Test image made in " + str(end_time - start_time) + "s")
