#ifndef TOKEN_TYPE_HEADER_FILE
#define TOKEN_TYPE_HEADER_FILE 1

typedef enum TokenType {
	TokenType_COMMAND_START,
	TokenType_COMMAND_END,
	TokenType_TEXT,
	TokenType_WHITESPACE
} TokenType;

#endif
