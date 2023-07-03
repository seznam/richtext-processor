#ifndef UTF8_ENCODER_HEADER_FILE
#define UTF8_ENCODER_HEADER_FILE 1

#include "../string.h"

unsigned int getEncodedCodepointLength(unsigned long codepoint);

string *encodeCodepoint(unsigned long codepoint);

typedef unsigned long CodepointDecoder(unsigned int byte, void *context);

string *transcodeSingleByteEncodingToUtf8(string * input,
					  void *codepointDecoderContext,
					  CodepointDecoder * codepointDecoder);

#endif
