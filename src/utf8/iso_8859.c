#include <stddef.h>
#include "../string.h"
#include "utf8_encoder.h"
#include "iso_8859.h"

struct CodepointDecoderContext {
	Iso8859ByteDecoder *byteDecoder;
};

static unsigned long codepointDecoder(unsigned int byte,
				      void *codepointDecoderContext);

string *transcodeIso8859ToUtf8(input, byteToCodepoint)
string *input;
Iso8859ByteDecoder *byteToCodepoint;
{
	struct CodepointDecoderContext context;
	if (byteToCodepoint == NULL) {
		return NULL;
	}

	context.byteDecoder = byteToCodepoint;
	return transcodeSingleByteEncodingToUtf8(input, &context,
						 codepointDecoder);
}

static unsigned long codepointDecoder(byte, codepointDecoderContext)
unsigned int byte;
void *codepointDecoderContext;
{
	struct CodepointDecoderContext *context;
	if (byte < 0x80) {
		/* The first 128 characters match Unicode codepoints */
		return byte;
	}

	context = (struct CodepointDecoderContext *)codepointDecoderContext;
	return context->byteDecoder(byte);
}
