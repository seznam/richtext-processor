#ifndef STRING_HEADER_FILE
#define STRING_HEADER_FILE 1

typedef struct string {
	unsigned long length;
	unsigned char *content;
} string;

string *string_create(unsigned long length);

string *string_from(const char *text);

string *string_substring(const string * source, unsigned long start,
			 unsigned long end);

int string_compare(const string * string1, const string * string2);

void string_free(string * stringPtr);

#endif
