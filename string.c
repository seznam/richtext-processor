#include <stdlib.h>
#include <string.h>
#include "string.h"

string *string_create(length)
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
char *text;
{
	unsigned long length = strlen(text);
	string *newString = string_create(length);
	if (newString == NULL) {
		return NULL;
	}

	memcpy(newString->content, text, newString->length);

	return newString;
}

string *string_substring(source, start, end)
string *source;
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
