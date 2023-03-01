#include <stdlib.h>
#include "bool.h"
#include "lexer.h"
#include "parser.h"
#include "string.h"
#include "typed_pointer_vector.h"
#include "vector.h"

static void freeNodes(ASTNodePointerVector * nodes);

static ParserError createError(unsigned long byteIndex,
			       unsigned long codepointIndex,
			       unsigned long tokenIndex,
			       ParserErrorCode errorCode);

static bool string_equals(string * string1, string * string2,
			  bool caseInsensitive);

ParserResult *parse(tokens, caseInsensitiveCommands)
TokenVector *tokens;
bool caseInsensitiveCommands;
{
	ParserResult *result;
	Token *token;
	unsigned long tokenIndex = 0;
	ASTNodePointerVector *nodes;
	ASTNodePointerVector *grownNodes;
	ASTNode *parent = NULL;
	ASTNode *node = NULL;
	string *value;
	ASTNodePointerVector *siblings;
	ParserErrorCode errorCode = ParserErrorCode_OK;
	string *command_lt, *command_nl, *command_np;

	result = malloc(sizeof(ParserResult));
	if (result == NULL) {
		return NULL;
	}

	if (tokens == NULL) {
		result->type = ParserResultType_ERROR;
		result->result.error =
		    createError(0, 0, 0, ParserErrorCode_NULL_TOKENS);
		return result;
	}

	result->type = ParserResultType_SUCCESS;

	nodes = ASTNodePointerVector_new(0, 0);
	if (nodes == NULL) {
		result->type = ParserResultType_ERROR;
		result->result.error =
		    createError(0, 0, 0,
				ParserErrorCode_OUT_OF_MEMORY_FOR_NODES);
		return result;
	}

	command_lt = string_from("lt");
	command_nl = string_from("nl");
	command_np = string_from("np");
	if (command_lt == NULL || command_nl == NULL || command_np == NULL) {
		errorCode = ParserErrorCode_OUT_OF_MEMORY_FOR_STRINGS;
	}

	for (token = tokens->items, tokenIndex = 0;
	     tokenIndex < tokens->size.length; tokenIndex++, token++) {
		node = malloc(sizeof(ASTNode));
		if (node == NULL) {
			errorCode = ParserErrorCode_OUT_OF_MEMORY_FOR_NODES;
			break;
		}

		value = token->value;
		node->byteIndex = token->byteIndex;
		node->codepointIndex = token->codepointIndex;
		node->tokenIndex = tokenIndex;
		node->parent = parent;
		node->children = NULL;
		node->value = string_substring(value, 0, value->length);
		if (node->value == NULL) {
			errorCode = ParserErrorCode_OUT_OF_MEMORY_FOR_STRINGS;
			break;
		}

		switch (token->type) {
		case TokenType_COMMAND_START:
			node->type = ASTNodeType_COMMAND;
			node->children = ASTNodePointerVector_new(0, 0);
			if (node->children == NULL) {
				errorCode =
				    ParserErrorCode_OUT_OF_MEMORY_FOR_NODES;
				break;
			}

			siblings = parent == NULL ? nodes : parent->children;
			grownNodes =
			    ASTNodePointerVector_append(siblings, &node);
			if (grownNodes == NULL) {
				errorCode =
				    ParserErrorCode_OUT_OF_MEMORY_FOR_NODES;
				break;
			}

			if (parent == NULL) {
				nodes = grownNodes;
			} else {
				parent->children = grownNodes;
			}

			if (!string_equals
			    (node->value, command_lt, caseInsensitiveCommands)
			    && !string_equals(node->value, command_nl,
					      caseInsensitiveCommands)
			    && !string_equals(node->value, command_np,
					      caseInsensitiveCommands)) {
				parent = node;
			}

			/* Prevent possible double-freeing on error */
			node = NULL;
			break;

		case TokenType_COMMAND_END:
			string_free(node->value);
			free(node);
			node = NULL;

			/*
			   The following line would not fit into the line length
			   limit on higher indentation level.
			 */
			errorCode =
			    ParserErrorCode_UNALLOWED_BALANCING_COMMAND_END;
			if (string_equals
			    (token->value, command_lt, caseInsensitiveCommands)
			    || string_equals(token->value, command_nl,
					     caseInsensitiveCommands)
			    || string_equals(token->value, command_np,
					     caseInsensitiveCommands)
			    ) {
				break;
			}
			errorCode = ParserErrorCode_OK;

			if (parent == NULL) {
				errorCode =
				    ParserErrorCode_UNEXPECTED_COMMAND_END;
				break;
			}

			if (!string_equals
			    (token->value, parent->value,
			     caseInsensitiveCommands)) {
				errorCode =
				    ParserErrorCode_IMPROPERLY_BALANCED_COMMAND;
				break;
			}

			parent = parent->parent;
			break;

		case TokenType_TEXT:
		case TokenType_WHITESPACE:
			node->type = token->type == TokenType_TEXT ?
			    ASTNodeType_TEXT : ASTNodeType_WHITESPACE;
			siblings = parent == NULL ? nodes : parent->children;
			grownNodes =
			    ASTNodePointerVector_append(siblings, &node);

			if (grownNodes == NULL) {
				errorCode =
				    ParserErrorCode_OUT_OF_MEMORY_FOR_NODES;
				break;
			}

			if (parent == NULL) {
				nodes = grownNodes;
			} else {
				parent->children = grownNodes;
			}

			/* Prevent possible double-freeing on error */
			node = NULL;
			break;

		default:
			errorCode = ParserErrorCode_UNSUPPORTED_TOKEN_TYPE;
			break;
		}

		if (errorCode != ParserErrorCode_OK) {
			break;
		}
	}

	string_free(command_lt);
	string_free(command_nl);
	string_free(command_np);

	if (parent != NULL && errorCode == ParserErrorCode_OK) {
		errorCode = ParserErrorCode_UNTERMINATED_COMMAND;
	}
	if (errorCode != ParserErrorCode_OK) {
		result->type = ParserResultType_ERROR;
	}

	switch (result->type) {
	case ParserResultType_SUCCESS:
		result->result.nodes = nodes;
		break;

	case ParserResultType_ERROR:
		if (errorCode == ParserErrorCode_OK) {
			errorCode = ParserErrorCode_INTERNAL_ASSERTION_FAILED;
		}

		if (parent != NULL
		    && errorCode == ParserErrorCode_UNTERMINATED_COMMAND) {
			result->result.error =
			    createError(parent->byteIndex,
					parent->codepointIndex,
					parent->tokenIndex, errorCode);
		} else if (tokenIndex < tokens->size.length) {
			result->result.error =
			    createError(token->byteIndex, token->codepointIndex,
					tokenIndex, errorCode);
		} else {
			result->result.error = createError(0, 0, 0, errorCode);
		}

		if (node != NULL) {
			free(node);
		}
		freeNodes(nodes);
		break;

	default:
		result->type = ParserResultType_ERROR;
		errorCode = ParserErrorCode_INTERNAL_ASSERTION_FAILED;
		result->result.error = createError(0, 0, 0, errorCode);
		if (node != NULL) {
			free(node);
		}
		freeNodes(nodes);
		break;
	}

	return result;
}

void ParserResult_free(result)
ParserResult *result;
{
	if (result == NULL) {
		return;
	}

	switch (result->type) {
	case ParserResultType_SUCCESS:
		freeNodes(result->result.nodes);
		break;

	case ParserResultType_ERROR:
		break;

	default:
		break;
	}

	free(result);
}

static void freeNodes(nodes)
ASTNodePointerVector *nodes;
{
	ASTNode **node;
	unsigned long index = 0;
	if (nodes == NULL) {
		return;
	}

	for (node = nodes->items; index < nodes->size.length; index++, node++) {
		freeNodes((*node)->children);
		string_free((*node)->value);
		free(*node);
	}

	ASTNodePointerVector_free(nodes);
}

static ParserError createError(byteIndex, codepointIndex, tokenIndex, errorCode)
unsigned long byteIndex;
unsigned long codepointIndex;
unsigned long tokenIndex;
ParserErrorCode errorCode;
{
	ParserError error;
	error.byteIndex = byteIndex;
	error.codepointIndex = codepointIndex;
	error.tokenIndex = tokenIndex;
	error.code = errorCode;
	return error;
}

static bool string_equals(string1, string2, caseInsensitive)
string *string1;
string *string2;
bool caseInsensitive;
{
	if (caseInsensitive) {
		return string_caseInsensitiveCompare(string1, string2) == 0;
	} else {
		return string_compare(string1, string2) == 0;
	}
}

Vector_ofPointerImplementation(ASTNode)
