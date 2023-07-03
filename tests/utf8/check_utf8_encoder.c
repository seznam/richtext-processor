#include "../../src/bool.h"
#include "../../src/utf8/utf8_encoder.h"
#include "../../src/string.h"
#include "../unit.h"

/*
   This file does not bother to free heap-allocated memory because it's a suite
   of unit tests, ergo it's not a big deal.
 */

unsigned int tests_run = 0;
unsigned int tests_failed = 0;

static void all_tests(void);

int main(void);

static unsigned long identityDecoder(unsigned int byte, void *context);

static unsigned long trackingDecoder(unsigned int byte, void *context);

static unsigned long bigOutputDecoder(unsigned int byte, void *context);

START_TEST(getEncodedCodepointLength_returns1ForFirst127Codepoints)
{
	unsigned long codepoint;
	for (codepoint = 0; codepoint < 128; codepoint++) {
		assert(getEncodedCodepointLength(codepoint) == 1,
		       "Expected the encoded codepoint length to be 1");
	}
END_TEST}

START_TEST(getEncodedCodepointLength_returns2ForCodepoints128To2047)
{
	unsigned long codepoint;
	for (codepoint = 128; codepoint < 2048; codepoint++) {
		unsigned int length = getEncodedCodepointLength(codepoint);
		assert(length == 2,
		       "Expected the encoded codepoint length to be 2");
	}
END_TEST}

START_TEST(getEncodedCodepointLength_returns3ForCodepoints2048To65535)
{
	unsigned long codepoint;
	for (codepoint = 2048; codepoint < 65536; codepoint++) {
		unsigned int length = getEncodedCodepointLength(codepoint);
		assert(length == 3,
		       "Expected the encoded codepoint length to be 3");
	}
END_TEST}

START_TEST(getEncodedCodepointLength_returns4ForCodepoints65536To2097151)
{
	unsigned long codepoint;
	for (codepoint = 65536; codepoint < 2097151; codepoint++) {
		unsigned int length = getEncodedCodepointLength(codepoint);
		assert(length == 4,
		       "Expected the encoded codepoint length to be 4");
	}
END_TEST}

START_TEST(getEncodedCodepointLength_returns0ForCodepointsAbove2097151)
{
	unsigned int length = getEncodedCodepointLength(2097152);
	assert(length == 0,
	       "Expected the encoded codepoint length to be 0 (error)");
END_TEST}

START_TEST(encodeCodepoint_producesValidUtf8SequenceForAllEncodableCodepoints)
{
	unsigned long codepoint;
	unsigned char *value;
	for (codepoint = 0; codepoint < 2097152; codepoint++) {
		string *utf8 = encodeCodepoint(codepoint);
		value = utf8->content;
		switch ((int)utf8->length) {
		case 1:
			assert((*value & 0x80) == 0,
			       "Expected the first bit to be 0");
			break;
		case 2:
			assert((*value & 0xe0) == 0xc0,
			       "Expected the first three bits to be 110");
			assert((*(value + 1) & 0xc0) == 0x80,
			       "Expected the 9th and 10th bits to be 10");
			break;
		case 3:
			assert((*value & 0xf0) == 0xe0,
			       "Expected the first four bits to be 1110");
			assert((*(value + 1) & 0xc0) == 0x80,
			       "Expected the 9th and 10th bits to be 10");
			assert((*(value + 2) & 0xc0) == 0x80,
			       "Expected the 17th and 18th bits to be 10");
			break;
		case 4:
			assert((*value & 0xf8) == 0xf0,
			       "Expected the first five bits to be 11110");
			assert((*(value + 1) & 0xc0) == 0x80,
			       "Expected the 9th and 10th bits to be 10");
			assert((*(value + 2) & 0xc0) == 0x80,
			       "Expected the 17th and 18th bits to be 10");
			assert((*(value + 3) & 0xc0) == 0x80,
			       "Expected the 25th and 26th bits to be 10");
			break;
		default:
			assert(false,
			       "Expected the encoded sequence length to be 1 to 4 bytes");
			break;
		}
	}
END_TEST}

START_TEST(encodeCodepoint_encodesTheCodepoint)
{
	unsigned long codepoint;
	unsigned char *value;
	for (codepoint = 0; codepoint < 2097152; codepoint++) {
		string *utf8 = encodeCodepoint(codepoint);
		unsigned long decodedCodepoint;
		value = utf8->content;
		switch ((int)utf8->length) {
		case 1:
			decodedCodepoint = *value & 0x7f;
			break;
		case 2:
			decodedCodepoint = (*value & 0x1f) << 6;
			decodedCodepoint |= *(value + 1) & 0x3f;
			break;
		case 3:
			decodedCodepoint = (*value & 0xf) << 12;
			decodedCodepoint |= (*(value + 1) & 0x3f) << 6;
			decodedCodepoint |= *(value + 2) & 0x3f;
			break;
		case 4:
			decodedCodepoint = (*value & 0x7) << 18;
			decodedCodepoint |= (*(value + 1) & 0x3f) << 12;
			decodedCodepoint |= (*(value + 2) & 0x3f) << 6;
			decodedCodepoint |= *(value + 3) & 0x3f;
			break;
		default:
			assert(false,
			       "Expected the encoded sequence length to be 1 to 4 bytes");
			break;
		}
		assert(decodedCodepoint == codepoint,
		       "The codepoint was not encoded correctly");
	}
END_TEST}

START_TEST(encodeCodepoint_returnsNullForUnencodableCodepoint)
{
	string *result = encodeCodepoint(2097152);
	assert(result == NULL, "Expected the return value to be NULL");
END_TEST}

START_TEST(transcodeSingleByteEncodingToUtf8_returnsNullForNullInput)
{
	assert(transcodeSingleByteEncodingToUtf8(NULL, NULL, identityDecoder) ==
	       NULL, "Expected NULL result");
END_TEST}

START_TEST(transcodeSingleByteEncodingToUtf8_returnsNullForNullDecoder)
{
	string *input;
	input = string_from("a");
	assert(transcodeSingleByteEncodingToUtf8(input, NULL, NULL) ==
	       NULL, "Expected NULL result");
END_TEST}

START_TEST(transcodeSingleByteEncodingToUtf8_acceptsNullContext)
{
	string *input = string_from("a");
	assert(transcodeSingleByteEncodingToUtf8(input, NULL, identityDecoder)
	       != NULL, "Expected non-NULL result");
END_TEST}

START_TEST
    (transcodeSingleByteEncodingToUtf8_invokesDecoderWithContextAndEveryByte) {
	string *input = string_from("abcd");
	string *result;
	unsigned int context[4];
	context[0] = 0;
	context[1] = 0;
	context[2] = 0;
	context[3] = 0;
	result =
	    transcodeSingleByteEncodingToUtf8(input, &context, trackingDecoder);
	assert(result != NULL
	       && string_compare(result, string_from("cdef")) == 0,
	       "Expected the result to be \"cdef\"");
	assert(context[0] > 0,
	       "Expected the decoded to be invoked with the 1st byte at least once");
	assert(context[1] > 0,
	       "Expected the decoded to be invoked with the 2nd byte at least once");
	assert(context[2] > 0,
	       "Expected the decoded to be invoked with the 3rd byte at least once");
	assert(context[3] > 0,
	       "Expected the decoded to be invoked with the 4th byte at least once");
END_TEST}

START_TEST(transcodeSingleByteEncodingToUtf8_handlesMultibyteCodepointsInOutput)
{
	string *input = string_from("aaaa"), *output;
	output =
	    transcodeSingleByteEncodingToUtf8(input, NULL, bigOutputDecoder);
	assert(output != NULL
	       && string_compare(output, string_from("ŘŘŘŘ")) == 0,
	       "Expected the result to contain all longer UTF-8 sequences");
END_TEST}

static void all_tests()
{
	runTest(getEncodedCodepointLength_returns1ForFirst127Codepoints);
	runTest(getEncodedCodepointLength_returns2ForCodepoints128To2047);
	runTest(getEncodedCodepointLength_returns3ForCodepoints2048To65535);
	runTest(getEncodedCodepointLength_returns4ForCodepoints65536To2097151);
	runTest(getEncodedCodepointLength_returns0ForCodepointsAbove2097151);
	runTest
	    (encodeCodepoint_producesValidUtf8SequenceForAllEncodableCodepoints);
	runTest(encodeCodepoint_encodesTheCodepoint);
	runTest(encodeCodepoint_returnsNullForUnencodableCodepoint);
	runTest(transcodeSingleByteEncodingToUtf8_returnsNullForNullInput);
	runTest(transcodeSingleByteEncodingToUtf8_returnsNullForNullDecoder);
	runTest(transcodeSingleByteEncodingToUtf8_acceptsNullContext);
	runTest
	    (transcodeSingleByteEncodingToUtf8_invokesDecoderWithContextAndEveryByte);
	runTest
	    (transcodeSingleByteEncodingToUtf8_handlesMultibyteCodepointsInOutput);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}

static unsigned long identityDecoder(byte, context)
unsigned int byte;
void *context;
{
	return context ? byte : byte;
}

static unsigned long trackingDecoder(byte, context)
unsigned int byte;
void *context;
{
	unsigned int *byteOccurrenceCount;
	byteOccurrenceCount = (unsigned int *)context;
	*(byteOccurrenceCount + byte - 'a') =
	    *(byteOccurrenceCount + byte - 'a') + 1;
	return byte + 2;
}

static unsigned long bigOutputDecoder(byte, context)
unsigned int byte;
void *context;
{
	return (byte || context) ? 0x158 : 0x158;	/* Ř */
}
