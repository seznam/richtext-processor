#include "../string.h"
#include "iso_8859.h"
#include "iso_8859_15.h"

static unsigned long decodeIso885915Byte(unsigned int byte);

static unsigned long EXTENDED_CHARS_CODEPOINTS[128];

string *iso885915ToUtf8(text)
string *text;
{
	return transcodeIso8859ToUtf8(text, decodeIso885915Byte);
}

static unsigned long decodeIso885915Byte(byte)
unsigned int byte;
{
	unsigned long codepoint = EXTENDED_CHARS_CODEPOINTS[byte - 0x80];
	return codepoint != 0 ? codepoint : byte;
}

/* https://en.wikipedia.org/wiki/ISO/IEC_8859-15 */
static unsigned long EXTENDED_CHARS_CODEPOINTS[] = {
	/* 0x8x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0x9x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xAx */
	0, 0, 0, 0, 0x20ac, 0, 0x160, 0, 0x161, 0, 0, 0, 0, 0, 0, 0,
	/* 0xBx */
	0, 0, 0, 0, 0x17d, 0, 0, 0, 0x17e, 0, 0, 0, 0x152, 0x153, 0x178, 0,
	/* 0xCx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xDx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xEx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xFx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
