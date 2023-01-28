#ifndef STRING_HEADER_FILE
#define STRING_HEADER_FILE 1

typedef struct string {
	unsigned long length;
	char *content;
} string;

string *string_substring(string * source, unsigned long start,
			 unsigned long end);

#endif
