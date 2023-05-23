#include <stdlib.h>
#include <string.h>
#include "string.h"

string *string_new(length)
unsigned long length;
{
	string *newString = malloc(sizeof(string));
	if (newString == NULL) {
		return NULL;
	}

	newString->length = length;
	if (length) {
		newString->content = malloc(sizeof(char) * length);
		if (newString->content == NULL) {
			free(newString);
			return NULL;
		}
	}

	return newString;
}

string *string_from(text)
const char *text;
{
	unsigned long length = strlen(text);
	string *newString = string_new(length);
	if (newString == NULL) {
		return NULL;
	}

	memcpy(newString->content, text, newString->length);

	return newString;
}

string *string_substring(source, start, end)
const string *source;
unsigned long start;
unsigned long end;
{
	string *substring;

	if (end < start) {
		return NULL;
	}

	substring = malloc(sizeof(string));
	if (substring == NULL) {
		return NULL;
	}

	if (start > source->length) {
		start = source->length;
	}
	if (end > source->length) {
		end = source->length;
	}

	substring->length = end - start;
	substring->content = NULL;
	if (!substring->length) {
		return substring;
	}

	substring->content = malloc(sizeof(char) * substring->length);
	if (substring->content == NULL) {
		free(substring);
		return NULL;
	}

	memcpy(substring->content, source->content + start, substring->length);

	return substring;
}

int string_compare(string1, string2)
const string *string1;
const string *string2;
{
	unsigned long minLength;
	int memcmpResult;

	if (string1 == NULL && string2 == NULL) {
		return 0;
	}
	if (string1 == NULL) {
		return -1;
	}
	if (string2 == NULL) {
		return 1;
	}

	if (string1->length == string2->length) {
		return memcmp(string1->content, string2->content,
			      string1->length);
	}

	minLength =
	    string1->length <
	    string2->length ? string1->length : string2->length;
	memcmpResult = memcmp(string1->content, string2->content, minLength);
	if (memcmpResult == 0) {
		return string1->length < string2->length ? -1 : 1;
	}

	return memcmpResult;
}

int string_caseInsensitiveCompare(string1, string2)
const string *string1;
const string *string2;
{
	unsigned char *char1Pointer, *char2Pointer, char1, char2;
	unsigned long i = 0;
	unsigned long minLength;

	if (string1 == NULL && string2 == NULL) {
		return 0;
	}
	if (string1 == NULL) {
		return -1;
	}
	if (string2 == NULL) {
		return 1;
	}

	char1Pointer = string1->content;
	char2Pointer = string2->content;
	minLength =
	    string1->length <
	    string2->length ? string1->length : string2->length;
	for (; i < minLength; i++, char1Pointer++, char2Pointer++) {
		char1 = *char1Pointer;
		char2 = *char2Pointer;
		if (char1 == char2) {
			continue;
		}

		if (char1 >= 65 /* A */  && char1 <= 90 /* Z */ ) {
			char1 += 32;	/* convert to lowercase */
		}
		if (char2 >= 65 /* A */  && char2 <= 90 /* Z */ ) {
			char2 += 32;	/* convert to lowercase */
		}
		if (char1 == char2) {
			continue;
		}

		return char1 < char2 ? -1 : 1;
	}

	if (string1->length == string2->length) {
		return 0;
	}
	return string1->length < string2->length ? -1 : 1;
}

void string_free(stringPtr)
string *stringPtr;
{
	if (stringPtr == NULL) {
		return;
	}

	if (stringPtr->content != NULL) {
		free(stringPtr->content);
	}

	free(stringPtr);
}
