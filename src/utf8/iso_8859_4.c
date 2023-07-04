#include "../string.h"
#include "iso_8859.h"
#include "iso_8859_4.h"

static unsigned long decodeIso88594Byte(unsigned int byte);

static unsigned long EXTENDED_CHARS_CODEPOINTS[128];

string *iso88594ToUtf8(text)
string *text;
{
	return transcodeIso8859ToUtf8(text, decodeIso88594Byte);
}

static unsigned long decodeIso88594Byte(byte)
unsigned int byte;
{
	unsigned long codepoint = EXTENDED_CHARS_CODEPOINTS[byte - 0x80];
	return codepoint != 0 ? codepoint : byte;
}

/* https://en.wikipedia.org/wiki/ISO/IEC_8859-4 */
static unsigned long EXTENDED_CHARS_CODEPOINTS[] = {
	/* 0x8x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0x9x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xAx */
	0, 0x104, 0x138, 0x156, 0, 0x128, 0x13b,
	0, 0, 0x160, 0x112, 0x122, 0x166, 0, 0x17d, 0,
	/* 0xBx */
	0, 0x105, 0x2db, 0x157, 0, 0x129, 0x13c, 0x2c7,
	0, 0x161, 0x113, 0x123, 0x167, 0x14a, 0x17e, 0x14b,
	/* 0xCx */
	0x100, 0, 0, 0, 0, 0, 0, 0x12e,
	0x10c, 0, 0x118, 0, 0x116, 0, 0, 0x12a,
	/* 0xDx */
	0x110, 0x145, 0x14c, 0x136, 0, 0, 0, 0,
	0, 0x172, 0, 0, 0, 0x168, 0x16a, 0,
	/* 0xEx */
	0x101, 0, 0, 0, 0, 0, 0, 0x12f,
	0x10d, 0, 0x119, 0, 0x117, 0, 0, 0x12b,
	/* 0xFx */
	0x111, 0x146, 0x14d, 0x137, 0, 0, 0, 0,
	0, 0x173, 0, 0, 0, 0x169, 0x16b, 0x2d9
};
