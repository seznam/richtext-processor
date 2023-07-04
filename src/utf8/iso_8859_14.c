#include "../string.h"
#include "iso_8859.h"
#include "iso_8859_14.h"

static unsigned long decodeIso885914Byte(unsigned int byte);

static unsigned long EXTENDED_CHARS_CODEPOINTS[128];

string *iso885914ToUtf8(text)
string *text;
{
	return transcodeIso8859ToUtf8(text, decodeIso885914Byte);
}

static unsigned long decodeIso885914Byte(byte)
unsigned int byte;
{
	unsigned long codepoint = EXTENDED_CHARS_CODEPOINTS[byte - 0x80];
	return codepoint != 0 ? codepoint : byte;
}

/* https://en.wikipedia.org/wiki/ISO/IEC_8859-14 */
static unsigned long EXTENDED_CHARS_CODEPOINTS[] = {
	/* 0x8x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0x9x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xAx */
	0, 0x1e02, 0x1e03, 0, 0x10a, 0x10b, 0x1e0a, 0,
	0x1e80, 0, 0x1e82, 0x1e0b, 0x1ef2, 0, 0, 0x178,
	/* 0xBx */
	0x1e1e, 0x1e1f, 0x120, 0x121, 0x1e40, 0x1e41, 0, 0x1e56,
	0x1e81, 0x1e57, 0x1e83, 0x1e60, 0x1ef3, 0x1e84, 0x1e85, 0x1e61,
	/* 0xCx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xDx */
	0x174, 0, 0, 0, 0, 0, 0, 0x1e6a, 0, 0, 0, 0, 0, 0, 0x176, 0,
	/* 0xEx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xFx */
	0x175, 0, 0, 0, 0, 0, 0, 0x1e6b, 0, 0, 0, 0, 0, 0, 0x177, 0,
};
