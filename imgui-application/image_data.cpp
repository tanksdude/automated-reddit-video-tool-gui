#include "image_data.h"

//TODO: unit test the heck out of this
std::string ImageData::get_paragraph_separator() const {
	std::string paragraph_separator = std::string(paragraph_separator_input);
	if (paragraph_separator.empty()) [[unlikely]] {
		return paragraph_separator;
	}
	int i = 0;
	while (i < paragraph_separator.size()) {
		if (i == paragraph_separator.size() - 1) [[unlikely]] {
			//can't look ahead
			if (paragraph_separator[i] == '\\') {
				paragraph_separator.replace(i, 1, "\\\\");
			} else if (paragraph_separator[i] == '\"') {
				paragraph_separator.replace(i, 1, "\\\"");
			}
			break;
		}

		if (paragraph_separator[i] == '\"') {
			paragraph_separator.replace(i, 1, "\\\"");
			i += 2;
		} else if (paragraph_separator[i] == '\\') {
			int backslashEndPos = i+1; //exclusive
			while (backslashEndPos < paragraph_separator.size() && paragraph_separator[backslashEndPos] == '\\') {
				backslashEndPos++;
			}
			//backslash range: [i,backslashEndPos)
			int backslashCount = backslashEndPos - i;
			std::string replacementBackslashes = "";
			for (int j = 0; j < backslashCount; j++) {
				replacementBackslashes += "\\\\";
			}

			if (backslashEndPos < paragraph_separator.size()) {
				if (paragraph_separator[backslashEndPos] == '\"') {
					paragraph_separator.replace(i, backslashCount+1, replacementBackslashes + "\\\"");
					i += (backslashCount+1) * 2;
				} else {
					//do not double up on the backslashes (I was *this* close to removing non-\n paragraph separators entirely)
					i += backslashCount + 1;
				}
			} else {
				paragraph_separator.replace(i, backslashCount, replacementBackslashes);
				i += backslashCount * 2;
				break;
			}
		} else {
			i++;
		}
	}
	return paragraph_separator;
}
