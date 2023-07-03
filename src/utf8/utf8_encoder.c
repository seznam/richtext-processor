#include <limits.h>
#include <stddef.h>
#include <string.h>
#include "../string.h"
#include "utf8_encoder.h"

unsigned int getEncodedCodepointLength(codepoint)
unsigned long codepoint;
{
	if (codepoint < 0x80) {
		return 1;
	}
	if (codepoint < 0x800) {
		return 2;
	}
	if (codepoint < 0x10000) {
		return 3;
	}
	if (codepoint < 0x200000) {
		/*
		 * All encodable codepoints are allowed, just in case that new
		  * codepoints are added in the future.
		 */
		return 4;
	}
	return 0;		/* Cannot be expressed in UTF-8 */
}

string *encodeCodepoint(codepoint)
unsigned long codepoint;
{
	unsigned int length = getEncodedCodepointLength(codepoint);
	string *result;

	if (length == 0) {
		return NULL;
	}

	result = string_new(length);
	if (result == NULL) {
		return NULL;
	}

	switch (length) {
	case 1:
		*result->content = codepoint;
		break;

	case 2:
		*result->content = 0xc0 | (codepoint >> 6);
		*(result->content + 1) = 0x80 | (0x3f & codepoint);
		break;

	case 3:
		*result->content = 0xe0 | (codepoint >> 12);
		*(result->content + 1) = 0x80 | (0x3f & (codepoint >> 6));
		*(result->content + 2) = 0x80 | (0x3f & codepoint);
		break;

	case 4:
		*result->content = 0xf0 | (codepoint >> 18);
		*(result->content + 1) = 0x80 | (0x3f & (codepoint >> 12));
		*(result->content + 2) = 0x80 | (0x3f & (codepoint >> 6));
		*(result->content + 3) = 0x80 | (0x3f & codepoint);
		break;

	default:
		string_free(result);
		return NULL;
	}

	return result;
}

string *transcodeSingleByteEncodingToUtf8(input, codepointDecoderContext,
					  codepointDecoder)
string *input;
void *codepointDecoderContext;
CodepointDecoder *codepointDecoder;
{
	unsigned long length = 0, i;
	unsigned char *inputChar, *outputChar;
	string *result;

	if (input == NULL || codepointDecoder == NULL) {
		return NULL;
	}

	inputChar = input->content;
	for (i = 0; i < input->length; i++, inputChar++) {
		unsigned long codepoint, encodedCodepointLength;
		codepoint =
		    codepointDecoder(*inputChar, codepointDecoderContext);
		encodedCodepointLength = getEncodedCodepointLength(codepoint);
		if (ULONG_MAX - encodedCodepointLength < length) {
			return NULL;
		}
		length += encodedCodepointLength;
	}

	result = string_new(length);
	if (result == NULL) {
		return NULL;
	}

	inputChar = input->content;
	outputChar = result->content;
	for (i = 0; i < input->length; i++, inputChar++) {
		unsigned long codepoint;
		string *encodedCodepoint;
		codepoint =
		    codepointDecoder(*inputChar, codepointDecoderContext);
		encodedCodepoint = encodeCodepoint(codepoint);
		if (encodedCodepoint == NULL) {
			string_free(result);
			return NULL;
		}

		memcpy(outputChar, encodedCodepoint->content,
		       encodedCodepoint->length);
		outputChar += encodedCodepoint->length;

		string_free(encodedCodepoint);
	}

	return result;
}
