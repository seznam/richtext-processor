#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "../bool.h"
#include "../string.h"
#include "unit.h"

unsigned int tests_run = 0;
unsigned int tests_failed = 0;

static void all_tests(void);

int main(void);

START_TEST(string_create_createsStringOfDesiredSize)
{
	string *str = string_create(5);
	char *testingBuffer;
	size_t bigBufferSize = 1024 * 1024;

	if (str->length != 5) {
		string_free(str);
		assert(false, "string length was not 5");
	}

	string_free(str);

	/* 1MiB should be big enough to span multiple memory blocks */

	str = string_create(bigBufferSize);
	testingBuffer = malloc(bigBufferSize);
	/*
	   This *might* cause segfault or other issues if the string is not allocated
	   big enough.
	 */
	memcpy(str->content, testingBuffer, bigBufferSize);

	free(testingBuffer);
	string_free(str);
END_TEST}

START_TEST(string_from_copiesTheSourceString)
{
	string *instance1 = string_from("abcd");
	char *instance2Chars = malloc(sizeof(char) * 4);
	string *instance2;
	bool result;

	memcpy(instance2Chars, "ABC", 4);
	instance2 = string_from(instance2Chars);
	memcpy(instance2Chars, "123", 3);

	result = instance1->length == 4
	    && memcmp(instance1->content, "abcd", 4) == 0
	    && instance2->length == 3 && memcmp(instance2Chars, "123", 3) == 0
	    && memcmp(instance2->content, "ABC", 3) == 0;

	string_free(instance1);
	free(instance2Chars);
	string_free(instance2);

	assert(result, "string contents were referenced instead of copied");
END_TEST}

START_TEST(string_substring_returnsNullIfEndIsLowerThanStart)
{
	string *testingString = string_create(4);
	string *substring = string_substring(testingString, 2, 1);
	string_free(testingString);
	string_free(substring);
	assert(substring == NULL, "returned substring was not NULL");
END_TEST}

START_TEST(string_substring_returnsEmptyStringForIndexesOutOfBounds)
{
	string *testingString = string_create(2);
	string *substring = string_substring(testingString, 3, 4);
	bool result = substring != NULL && substring->length == 0;
	string_free(testingString);
	string_free(substring);
	assert(result, "the returned string was not empty");
END_TEST}

START_TEST(string_substring_returnsNullContentForEmptySubstring)
{
	string *testingString = string_create(2);
	string *substring = string_substring(testingString, 2, 4);
	int result = substring != NULL && substring->content == NULL;
	string_free(testingString);
	string_free(substring);
	assert(result, "returned substring did not have null content pointer");
END_TEST}

START_TEST(string_substring_returnsRequestedSlice)
{
	string *testingString = string_from("abcdef");
	string *substring = string_substring(testingString, 2, 5);
	int result = substring != NULL && substring->length == 3
	    && memcmp(substring->content, "cde", 3) == 0;
	string_free(substring);
	string_free(testingString);
	assert(result, "the returned slice was incorrect");
END_TEST}

START_TEST(string_substring_returnsSubstringIsolatedFromSource)
{
	string *testingString = string_from("abcd");
	string *substring = string_substring(testingString, 0, 4);
	int result;
	memcpy(testingString->content, "1234", sizeof(char) * 4);
	result = substring != NULL
	    && memcmp(substring->content, "abcd", 4) == 0;
	string_free(testingString);
	string_free(substring);
	assert(result,
	       "the returned substring referenced the source byte array");
END_TEST}

START_TEST(string_free_acceptsNullInput)
{
	string_free(NULL);
END_TEST}

START_TEST(string_free_acceptsStringWillNullContent)
{
	string *testingString = string_create(0);
	string_free(testingString);
END_TEST}

START_TEST(string_free_freesStringContent)
{
	string *testingString = string_from("abc");
	string_free(testingString);
END_TEST}

static void all_tests()
{
	run_test(string_create_createsStringOfDesiredSize);
	run_test(string_from_copiesTheSourceString);
	run_test(string_substring_returnsNullIfEndIsLowerThanStart);
	run_test(string_substring_returnsEmptyStringForIndexesOutOfBounds);
	run_test(string_substring_returnsNullContentForEmptySubstring);
	run_test(string_substring_returnsRequestedSlice);
	run_test(string_substring_returnsSubstringIsolatedFromSource);
	run_test(string_free_acceptsNullInput);
	run_test(string_free_acceptsStringWillNullContent);
	run_test(string_free_freesStringContent);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
