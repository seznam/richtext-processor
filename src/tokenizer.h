#ifndef TOKENIZER_HEADER_FILE
#define TOKENIZER_HEADER_FILE 1

#include "string.h"
#include "token_vector.h"

/*
  Note: UTF-8 uses 1-4 bytes to encode a codepoint, but some "characters" may
	consists of multiple codepoints (for example national flags use 2 4-byte
	codepoints, therefore occupying 8 bytes).
	To keep things reasonably simple, this code provides both byte indexes
	and codepoint indexes of tokens and errors, but does not provide
	"character" indexes (this is also due to the fact that new characters
	constructed of multiple codepoints are still being added to Unicode as
	of writing this). See https://en.wikipedia.org/wiki/UTF-8#Encoding for
	more details on UTF-8 encoding.
 */

typedef enum TokenizerErrorCode {
	TokenizerErrorCode_OK,
	TokenizerErrorCode_UNEXPECTED_COMMAND_START,
	TokenizerErrorCode_UNTERMINATED_COMMAND,
	TokenizerErrorCode_OUT_OF_MEMORY_FOR_SUBSTRING,
	TokenizerErrorCode_OUT_OF_MEMORY_FOR_TOKENS,
	TokenizerErrorCode_OUT_OF_MEMORY_FOR_WARNINGS
} TokenizerErrorCode;

typedef struct TokenizerError {
	unsigned long byteIndex;
	unsigned long codepointIndex;
	TokenizerErrorCode code;
} TokenizerError;

typedef enum TokenizerWarningCode {
	TokenizerWarningCode_INVALID_UTF8_CHARACTER,
	TokenizerWarningCode_UNEXPECTED_UTF8_CONTINUATION_BYTE
} TokenizerWarningCode;

typedef struct TokenizerWarning {
	unsigned long byteIndex;
	unsigned long codepointIndex;
	TokenizerWarningCode code;
} TokenizerWarning;

Vector_ofType(TokenizerWarning)
typedef enum TokenizerResultType {
	TokenizerResultType_SUCCESS,
	TokenizerResultType_ERROR
} TokenizerResultType;

typedef struct TokenizerResult {
	TokenizerResultType type;
	union {
		TokenVector *tokens;
		TokenizerError error;
	} result;
	TokenizerWarningVector *warnings;
} TokenizerResult;

TokenizerResult *tokenize(string * richtext);

void TokenizerResult_free(TokenizerResult * result);

#endif
