#ifndef PARSER_HEADER_FILE
#define PARSER_HEADER_FILE 1

#include "ast_node_pointer_vector.h"
#include "bool.h"
#include "tokenizer.h"
#include "string.h"
#include "token_vector.h"

typedef enum ParserErrorCode {
	ParserErrorCode_OK,
	ParserErrorCode_NULL_TOKENS,
	ParserErrorCode_UNALLOWED_BALANCING_COMMAND_END,
	ParserErrorCode_UNTERMINATED_COMMAND,
	ParserErrorCode_UNEXPECTED_COMMAND_END,
	ParserErrorCode_IMPROPERLY_BALANCED_COMMAND,
	ParserErrorCode_OUT_OF_MEMORY_FOR_STRINGS,
	ParserErrorCode_OUT_OF_MEMORY_FOR_NODES,
	ParserErrorCode_UNSUPPORTED_TOKEN_TYPE,
	ParserErrorCode_INTERNAL_ASSERTION_FAILED
} ParserErrorCode;

typedef struct ParserError {
	unsigned long byteIndex;
	unsigned long codepointIndex;
	unsigned long tokenIndex;
	ParserErrorCode code;
} ParserError;

typedef enum ParserResultType {
	ParserResultType_SUCCESS,
	ParserResultType_ERROR
} ParserResultType;

typedef struct ParserResult {
	ParserResultType type;
	union {
		ASTNodePointerVector *nodes;
		ParserError error;
	} result;
} ParserResult;

ParserResult *parse(TokenVector * tokens, bool caseInsensitiveCommands);

void ParserResult_free(ParserResult * result);

#endif
