#include "../string.h"
#include "iso_8859.h"
#include "iso_8859_1.h"

static unsigned long decodeIso88591Byte(unsigned int byte);

string *iso88591ToUtf8(text)
string *text;
{
	return transcodeIso8859ToUtf8(text, decodeIso88591Byte);
}

static unsigned long decodeIso88591Byte(byte)
unsigned int byte;
{
	/*
	 * All characters match their Unicode codepoints in ISO-8859-1
	 * (although some characters are undefined in the code page).
	 */
	return byte;
}
