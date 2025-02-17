import os, sys
import re
import argparse
import time

# python comment_splitter.py input output

SPLIT_CHARS = "".join([".", "!", "\\?"])
SPLIT_REGEX = "[" + SPLIT_CHARS + r"]+\S*\s+"

parser = argparse.ArgumentParser()
parser.add_argument("input_comment_file", help="comment to split")
parser.add_argument("output_comment_file", help="output split comment file")

args = parser.parse_args()

input_file_path = args.input_comment_file
output_file_path = args.output_comment_file

start_time = time.time()

try:
	input_file = open(input_file_path, "r", encoding="utf8")
except FileNotFoundError:
	sys.exit("File \"" + input_file_path + "\" not found!")
except IsADirectoryError:
	sys.exit("\"" + input_file_path + "\" is a directory; could not read")
except PermissionError:
	sys.exit("Could not read \"" + input_file_path + "\" due to permissions granted!")
except Exception as e:
	sys.exit("Other error while reading file \"" + input_file_path + "\":", e)

file_text = input_file.read().rstrip()
input_file.close()

# split file text on newline (mostly to trim excess newlines)

file_lines = []

while len(file_text) > 0:
	re_match = re.search("[\n]+", file_text)
	if re_match == None:
		# not found, rest of the text gets appended
		file_lines.append(file_text)
		break
	else:
		start = re_match.span()[0]
		end = re_match.span()[1]
		#print(file_text[0:end])
		file_lines.append(file_text[0:start+1])
		file_text = file_text[end:]

# split each line on a period or whatever

file_sentences = []

for line in file_lines:
	file_sentences.append([])
	while len(line) > 0:
		re_match = re.search(SPLIT_REGEX, line)
		if re_match == None:
			# not found, rest of the text gets appended
			file_sentences[-1].append(line)
			break
		else:
			start = re_match.span()[0]
			end = re_match.span()[1]
			#print(line[0:end])
			file_sentences[-1].append(line[0:end])
			line = line[end:]

# write each line to the output file

output_file = open(output_file_path, "w", encoding="utf8")
for line in file_sentences:
	for sentence in line:
		output_file.write(sentence + "\n")
output_file.close()

end_time = time.time()
print("Wrote " + output_file_path + " in " + str(end_time - start_time) + "s")
