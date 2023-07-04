#include "../string.h"
#include "iso_8859.h"
#include "iso_8859_9.h"

static unsigned long decodeIso88599Byte(unsigned int byte);

static unsigned long EXTENDED_CHARS_CODEPOINTS[128];

string *iso88599ToUtf8(text)
string *text;
{
	return transcodeIso8859ToUtf8(text, decodeIso88599Byte);
}

static unsigned long decodeIso88599Byte(byte)
unsigned int byte;
{
	unsigned long codepoint = EXTENDED_CHARS_CODEPOINTS[byte - 0x80];
	return codepoint != 0 ? codepoint : byte;
}

/* https://en.wikipedia.org/wiki/ISO/IEC_8859-9 */
static unsigned long EXTENDED_CHARS_CODEPOINTS[] = {
	/* 0x8x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0x9x */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xAx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xBx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xCx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xDx */
	0x11e, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x130, 0x15e, 0,
	/* 0xEx */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 0xFx */
	0x11f, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x131, 0x15f, 0
};
