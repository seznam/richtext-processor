#include "../string.h"
#include "iso_8859.h"
#include "iso_8859_3.h"

static unsigned long decodeIso88593Byte(unsigned int byte);

static unsigned long EXTENDED_CHARS_CODEPOINTS[128];

string *iso88593ToUtf8(text)
string *text;
{
	return transcodeIso8859ToUtf8(text, decodeIso88593Byte);
}

static unsigned long decodeIso88593Byte(byte)
unsigned int byte;
{
	unsigned long codepoint = EXTENDED_CHARS_CODEPOINTS[byte - 0x80];
	return codepoint != 0 ? codepoint : byte;
}

/* https://en.wikipedia.org/wiki/ISO/IEC_8859-3 */
static unsigned long EXTENDED_CHARS_CODEPOINTS[] = {
	/* 0x8x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0x9x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xAx */
	0, 0x126, 0x2d8, 0, 0, 0, 0x124, 0,
	0, 0x130, 0x15e, 0x11e, 0x134, 0, 0, 0x17b,
	/* 0xBx */
	0, 0x127, 0, 0, 0, 0, 0x125, 0,
	0, 0x131, 0x15f, 0x11f, 0x135, 0, 0, 0x17c,
	/* 0xCx */
	0, 0, 0, 0, 0, 0x10a, 0x108, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xDx */
	0, 0, 0, 0, 0, 0x120, 0, 0, 0x11c, 0, 0, 0, 0, 0x16c, 0x15c, 0,
	/* 0xEx */
	0, 0, 0, 0, 0, 0x10b, 0x109, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xFx */
	0, 0, 0, 0, 0, 0x121, 0, 0, 0x11d, 0, 0, 0, 0, 0x16d, 0x15d, 0x2d9
};
