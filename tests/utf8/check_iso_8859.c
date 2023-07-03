#include "../../src/utf8/iso_8859.h"
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

static unsigned long identityDecoder(unsigned int byte);

static unsigned long trackingDecoder(unsigned int byte);

static unsigned long transformingDecoder(unsigned int byte);

static int decodedBytes[256];

START_TEST(transcodeIso8859ToUtf8_rejectsNullInput)
{
	assert(transcodeIso8859ToUtf8(NULL, identityDecoder) == NULL,
	       "Expected NULL result");
END_TEST}

START_TEST(transcodeIso8859ToUtf8_rejectsNullDecoder)
{
	assert(transcodeIso8859ToUtf8(string_from("1"), NULL) == NULL,
	       "Expected NULL result");
END_TEST}

START_TEST(transcodeIso8859ToUtf8_invokesDecodedForOnlyUpper128Characters)
{
	string *input = string_new(256), *output;
	unsigned int i;

	for (i = 0; i < 256; i++) {
		decodedBytes[i] = 0;
		*(input->content + i) = i;
	}

	output = transcodeIso8859ToUtf8(input, trackingDecoder);
	assert(output != NULL, "Expected non-NULL result");

	for (i = 0; i < 128; i++) {
		assert(decodedBytes[i] == 0,
		       "The decoder was erroneously invoked for some of the low 128 character");
	}
	for (i = 128; i < 256; i++) {
		assert(decodedBytes[i] > 0,
		       "The decoder was erroneously not invoked for some of the upper 128 character");
	}
END_TEST}

START_TEST(transcodeIso8859ToUtf8_usesProvidedDecoderToDecodeUpper128Characters)
{
	string *input = string_from("\247bc \247b"), *output;
	output = transcodeIso8859ToUtf8(input, transformingDecoder);
	assert(string_compare(output, string_from("Šbc Šb")) == 0,
	       "The expected body did not match expectations");
END_TEST}

static void all_tests()
{
	runTest(transcodeIso8859ToUtf8_rejectsNullInput);
	runTest(transcodeIso8859ToUtf8_rejectsNullDecoder);
	runTest(transcodeIso8859ToUtf8_invokesDecodedForOnlyUpper128Characters);
	runTest
	    (transcodeIso8859ToUtf8_usesProvidedDecoderToDecodeUpper128Characters);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}

static unsigned long identityDecoder(byte)
unsigned int byte;
{
	return byte;
}

static unsigned long trackingDecoder(byte)
unsigned int byte;
{
	decodedBytes[byte]++;
	return byte;
}

static unsigned long transformingDecoder(byte)
unsigned int byte;
{
	return byte == 0xa7 ? 0x160 : byte;
}
