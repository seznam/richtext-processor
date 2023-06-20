#ifndef TOKEN_HEADER_FILE
#define TOKEN_HEADER_FILE 1

#include "string.h"
#include "token_type.h"

typedef struct Token {
	unsigned long byteIndex;
	unsigned long codepointIndex;
	TokenType type;
	string *value;
} Token;

#endif
