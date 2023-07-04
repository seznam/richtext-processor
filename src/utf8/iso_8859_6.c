#include "../string.h"
#include "iso_8859.h"
#include "iso_8859_6.h"

static unsigned long decodeIso88596Byte(unsigned int byte);

static unsigned long EXTENDED_CHARS_CODEPOINTS[128];

string *iso88596ToUtf8(text)
string *text;
{
	return transcodeIso8859ToUtf8(text, decodeIso88596Byte);
}

static unsigned long decodeIso88596Byte(byte)
unsigned int byte;
{
	unsigned long codepoint = EXTENDED_CHARS_CODEPOINTS[byte - 0x80];
	return codepoint != 0 ? codepoint : byte;
}

/* https://en.wikipedia.org/wiki/ISO/IEC_8859-6 */
static unsigned long EXTENDED_CHARS_CODEPOINTS[] = {
	/* 0x8x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0x9x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xAx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x60c, 0, 0, 0,
	/* 0xBx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x61b, 0, 0, 0, 0x61f,
	/* 0xCx */
	0, 0x621, 0x622, 0x623, 0x624, 0x625, 0x626, 0x627,
	0x628, 0x629, 0x62a, 0x62b, 0x62c, 0x62d, 0x62e, 0x62f,
	/* 0xDx */
	0x630, 0x631, 0x632, 0x633, 0x634, 0x635, 0x636, 0x637,
	0x638, 0x639, 0x63a, 0, 0, 0, 0, 0,
	/* 0xEx */
	0x640, 0x641, 0x642, 0x643, 0x644, 0x645, 0x646, 0x647,
	0x648, 0x649, 0x64a, 0x64b, 0x64c, 0x64d, 0x64e, 0x64f,
	/* 0xFx */
	0x650, 0x651, 0x652, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
