#ifndef LEXER_HEADER_FILE
#define LEXER_HEADER_FILE 1

#include "string.h"
#include "vector.h"

/*
  Note: UTF-8 uses 1-4 bytes to encode a codepoint, but some "characters" may
	consists of multiple codepoints (for example national flags use 2 4-byte
	codepoints, therefore occupying 8 bytes).
	To keep things reasonably simple, this code provides both byte indexes
	and codepoint indexes of lexes and errors, but does not provide
	"character" indexes (this is also due to the fact that new characters
	constructed of multiple codepoints are still being added to Unicode as
	of writing this). See https://en.wikipedia.org/wiki/UTF-8#Encoding for
	more details on UTF-8 encoding.
 */

typedef enum TokenType {
	TokenType_COMMAND_START,
	TokenType_COMMAND_END,
	TokenType_TEXT,
	TokenType_WHITESPACE
} TokenType;

typedef struct Token {
	unsigned long byteIndex;
	unsigned long codepointIndex;
	TokenType type;
	string *value;
} Token;

typedef struct TokenVector {
	VectorSize size;
	Token *items;
} TokenVector;

typedef enum LexerErrorCode {
	LexerErrorCode_UNEXPECTED_COMMAND_START,
	LexerErrorCode_UNTERMINATED_COMMAND,
	LexerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING,
	LexerErrorCode_OUT_OF_MEMORY_FOR_TOKENS,
	LexerErrorCode_OUT_OF_MEMORY_FOR_WARNINGS
} LexerErrorCode;

typedef struct LexerError {
	unsigned long byteIndex;
	unsigned long codepointIndex;
	LexerErrorCode code;
} LexerError;

typedef enum LexerWarningCode {
	LexerWarningCode_INVALID_UTF8_CHARACTER,
	LexerWarningCode_UNEXPECTED_UTF8_CONTINUATION_BYTE
} LexerWarningCode;

typedef struct LexerWarning {
	unsigned long byteIndex;
	unsigned long codepointIndex;
	LexerWarningCode code;
} LexerWarning;

typedef struct LexerWarningVector {
	VectorSize size;
	LexerWarning *items;
} LexerWarningVector;

typedef enum LexerResultType {
	LexerResultType_SUCCESS,
	LexerResultType_ERROR
} LexerResultType;

typedef struct LexerResult {
	LexerResultType type;
	union {
		TokenVector *tokens;
		LexerError error;
	} result;
	LexerWarningVector *warnings;
} LexerResult;

LexerResult *tokenize(string * richtext);

void LexerResult_free(LexerResult * result);

#endif
