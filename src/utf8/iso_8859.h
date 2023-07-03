#ifndef ISO_8859_HEADER_FILE
#define ISO_8859_HEADER_FILE 1

#include "../string.h"

typedef unsigned long Iso8859ByteDecoder(unsigned int);

string *transcodeIso8859ToUtf8(string * input,
			       Iso8859ByteDecoder * byteToCodepoint);

#endif
