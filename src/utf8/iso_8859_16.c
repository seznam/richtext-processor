#include "../string.h"
#include "iso_8859.h"
#include "iso_8859_16.h"

static unsigned long decodeIso885916Byte(unsigned int byte);

static unsigned long EXTENDED_CHARS_CODEPOINTS[128];

string *iso885916ToUtf8(text)
string *text;
{
	return transcodeIso8859ToUtf8(text, decodeIso885916Byte);
}

static unsigned long decodeIso885916Byte(byte)
unsigned int byte;
{
	unsigned long codepoint = EXTENDED_CHARS_CODEPOINTS[byte - 0x80];
	return codepoint != 0 ? codepoint : byte;
}

/* https://en.wikipedia.org/wiki/ISO/IEC_8859-16 */
static unsigned long EXTENDED_CHARS_CODEPOINTS[] = {
	/* 0x8x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0x9x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xAx */
	0, 0x104, 0x105, 0x141, 0x20ac, 0x201e, 0x160, 0,
	0x161, 0, 0x218, 0, 0x179, 0, 0x17a, 0x17b,
	/* 0xBx */
	0, 0, 0x10c, 0x142, 0x17d, 0x201d, 0, 0,
	0x17e, 0x10d, 0x219, 0, 0x152, 0x153, 0x178, 0x17c,
	/* 0xCx */
	0, 0, 0, 0x102, 0, 0x106, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xDx */
	0x110, 0x143, 0, 0, 0, 0x150, 0, 0x15a,
	0x170, 0, 0, 0, 0, 0x118, 0x21a, 0,
	/* 0xEx */
	0, 0, 0, 0x103, 0, 0x107, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xFx */
	0x111, 0x144, 0, 0, 0, 0x151, 0, 0x15b,
	0x171, 0, 0, 0, 0, 0x119, 0x21b, 0
};
