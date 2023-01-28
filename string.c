#include <stdlib.h>
#include <string.h>
#include "string.h"

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
