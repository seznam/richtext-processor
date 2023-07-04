#ifndef UTF8_US_ASCII_HEADER_FILE
#define UTF8_US_ASCII_HEADER_FILE 1

#include "../string.h"

/**
 * Converts a text in 7bit US-ASCII encoding to UTF8.
 *
 * The US-ASCII standard has just 128 code points, and these are encoded the
 * same way in UTF-8 (see https://en.wikipedia.org/wiki/ASCII).
 *
 * The function does support extended (8bit) US-ASCII, treating it as
 * ISO-8859-1 encoding, however, there are many proprietary and national
 * ASCII-derived 8-bit character sets out there, so anything marked as US-ASCII
 * and using the upper 128 characters cannot be decoded completely reliably -
 * see https://en.wikipedia.org/wiki/Extended_ASCII.
 * 
 * RFC 1341 (which defined text/richtext) sets expectations for only 7bit
 * US-ASCII support (see https://www.rfc-editor.org/rfc/rfc1341#ref-US-ASCII),
 * but also recommends clients to be more lenient in what they accept, so this
 * has been decided to be the best compromise.
 */
string *usAsciiToUtf8(string * text);

#endif
