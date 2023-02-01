#ifndef STRING_HEADER_FILE
#define STRING_HEADER_FILE 1

typedef struct string {
	unsigned long length;
	char *content;
} string;

string *string_create(unsigned long length);

string *string_from(char *text);

string *string_substring(string * source, unsigned long start,
			 unsigned long end);

void string_free(string * stringPtr);

#endif
