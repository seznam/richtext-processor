#include "../string.h"
#include "iso_8859.h"
#include "iso_8859_8.h"

static unsigned long decodeIso88598Byte(unsigned int byte);

static unsigned long EXTENDED_CHARS_CODEPOINTS[128];

string *iso88598ToUtf8(text)
string *text;
{
	return transcodeIso8859ToUtf8(text, decodeIso88598Byte);
}

static unsigned long decodeIso88598Byte(byte)
unsigned int byte;
{
	unsigned long codepoint = EXTENDED_CHARS_CODEPOINTS[byte - 0x80];
	return codepoint != 0 ? codepoint : byte;
}

/* https://en.wikipedia.org/wiki/ISO/IEC_8859-8 */
static unsigned long EXTENDED_CHARS_CODEPOINTS[] = {
	/* 0x8x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0x9x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xAx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xd7, 0, 0, 0, 0, 0,
	/* 0xBx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xf7, 0, 0, 0, 0, 0,
	/* 0xCx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xDx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x2017,
	/* 0xEx */
	0x5d0, 0x5d1, 0x5d2, 0x5d3, 0x5d4, 0x5d5, 0x5d6, 0x5d7,
	0x5d8, 0x5d9, 0x5da, 0x5db, 0x5dc, 0x5dd, 0x5de, 0x5df,
	/* 0xFx */
	0x5e0, 0x5e1, 0x5e2, 0x5e3, 0x5e4, 0x5e5, 0x5e6, 0x5e7,
	0x5e8, 0x5e9, 0x5ea, 0, 0, 0x200e, 0x200f, 0,
};
