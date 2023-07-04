#include "../string.h"
#include "iso_8859_1.h"
#include "us_ascii.h"

string *usAsciiToUtf8(text)
string *text;
{
	return iso88591ToUtf8(text);
}
