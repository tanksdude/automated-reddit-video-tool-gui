#include "../imgui-application/ini_helper.h"
#include <gtest/gtest.h>

#include <algorithm> //std::fill
#include <cstring> //strlen

namespace {

TEST(IniHelper, CopyStringToBuffer1) {
	char dest[32];
	std::fill(dest, dest+32, 127);
	const char* src = "asdf";

	ARVT::copyUserStringToCharBuffer(dest, 32, src, strlen(src));
	EXPECT_EQ(dest[0], 'a');
	EXPECT_EQ(dest[1], 's');
	EXPECT_EQ(dest[2], 'd');
	EXPECT_EQ(dest[3], 'f');
	EXPECT_EQ(dest[4], '\0');
}

TEST(IniHelper, CopyStringToBuffer2) {
	char dest[32];
	std::fill(dest, dest+32, 127);
	const char* src = "asdf";

	ARVT::copyUserStringToCharBuffer(dest, strlen(src)+1, src, strlen(src));
	EXPECT_EQ(dest[0], 'a');
	EXPECT_EQ(dest[1], 's');
	EXPECT_EQ(dest[2], 'd');
	EXPECT_EQ(dest[3], 'f');
	EXPECT_EQ(dest[4], '\0');
	EXPECT_EQ(dest[5], 127);
}

TEST(IniHelper, CopyStringToBuffer3) {
	char dest[32];
	std::fill(dest, dest+32, 127);
	const char* src = "asdf";

	ARVT::copyUserStringToCharBuffer(dest, 2, src, strlen(src));
	EXPECT_EQ(dest[0], 'a');
	EXPECT_EQ(dest[1], '\0');
	EXPECT_EQ(dest[2], 127);
}

} // namespace
