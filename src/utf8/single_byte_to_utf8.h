#ifndef SINGLE_BYTE_TO_UTF8_HEADER_FILE
#define SINGLE_BYTE_TO_UTF8_HEADER_FILE 1

#include "../string.h"
#include "single_byte_encoding.h"

string *transcodeSingleByteEncodedTextToUtf8(SingleByteEncoding inputEncoding,
					     string * input);

#endif
