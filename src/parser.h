#ifndef PARSER_HEADER_FILE
#define PARSER_HEADER_FILE 1

#include "bool.h"
#include "lexer.h"
#include "string.h"
#include "vector.h"

typedef enum ASTNodeType {
	ASTNodeType_COMMAND,
	ASTNodeType_TEXT,
	ASTNodeType_WHITESPACE
} ASTNodeType;

struct ASTNodeVector;

typedef struct ASTNode {
	unsigned long byteIndex;
	unsigned long codepointIndex;
	unsigned long tokenIndex;
	ASTNodeType type;
	string *value;
	struct ASTNode *parent;
	struct ASTNodeVector *children;
} ASTNode;

typedef struct ASTNodeVector {
	VectorSize size;
	ASTNode **items;
} ASTNodeVector;

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
		ASTNodeVector *nodes;
		ParserError error;
	} result;
} ParserResult;

ParserResult *parse(TokenVector * tokens, bool caseInsensitiveCommands);

void ParserResult_free(ParserResult * result);

#endif
