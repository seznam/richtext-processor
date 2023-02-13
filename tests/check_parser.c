#include <stdio.h>
#include <stdlib.h>
#include "../src/bool.h"
#include "../src/parser.h"
#include "unit.h"

/*
   This file does not bother to free heap-allocated memory because it a suite
	 of unit tests, ergo it's not a big deal.
 */

unsigned int tests_run = 0;
unsigned int tests_failed = 0;

static void all_tests(void);

int main(void);

static ParserResult *parseStringToResult(const char *input,
					 bool caseInsensitiveCommands);

static ASTNodeVector *parseString(const char *input,
				  bool caseInsensitiveCommands);

static char *_assert_node(char *filename, unsigned int line,
			  unsigned int nodeOrdinalNumber, ASTNode * node,
			  unsigned long byteIndex, unsigned long codepointIndex,
			  unsigned long tokenIndex, ASTNodeType type,
			  char *value);

static char *_assert_error(char *fileName, unsigned int line, ParserError error,
			   unsigned long byteIndex,
			   unsigned long codepointIndex,
			   unsigned long tokenIndex, ParserErrorCode code);

#define assert_node(nodeOrdinalNumber, node, byteIndex, codepointIndex, \
		    tokenIndex, type, value)\
do {\
	char *assertError = _assert_node(__FILE__, __LINE__, nodeOrdinalNumber,\
					 node, byteIndex, codepointIndex, \
					 tokenIndex, type, value);\
	if (assertError != NULL) {\
		return assertError;\
	}\
} while (0)

#define assert_error(error, byteIndex, codepointIndex, tokenIndex, code)\
do {\
	char *assertError = _assert_error(__FILE__, __LINE__, error, byteIndex,\
					  codepointIndex, tokenIndex, code);\
	if (assertError != NULL) {\
		return assertError;\
	}\
} while (0)

START_TEST(parse_returnsNullTokensErrorForNullInput)
{
	ParserResult *result = parse(NULL, true);
	ParserError error;
	assert(result != NULL
	       && result->type == ParserResultType_ERROR,
	       "Expected error result for tokens being null");
	error = result->result.error;
	assert(error.byteIndex == 0 && error.codepointIndex == 0
	       && error.tokenIndex == 0,
	       "Expected the byte, codepoint and token indexes to be 0");
	assert(error.code == ParserErrorCode_NULL_TOKENS,
	       "Expected the error code to be NULL_TOKENS");
END_TEST}

START_TEST(parse_processesPlainText)
{
	ASTNodeVector *nodes = parseString("foo  bar baz", false);
	ASTNode **nodePointer;
	assert(nodes != NULL, "Expected the text to be parsed");
	assert(nodes->size.length == 6,
	       "Expected the result to have 6 root nodes");
	nodePointer = nodes->items;
	assert_node(1, *nodePointer, 0, 0, 0, ASTNodeType_TEXT, "foo");
	assert((*nodePointer)->children == NULL,
	       "Expected NULL children for text node");
	assert((*nodePointer)->parent == NULL,
	       "Expected NULL parent pointer for root node");
	assert_node(2, *(nodePointer + 1), 3, 3, 1, ASTNodeType_WHITESPACE,
		    " ");
	assert((*(nodePointer + 1))->children == NULL,
	       "Expected NULL children for whitespace node");
	assert((*(nodePointer + 1))->parent == NULL,
	       "Expected NULL parent pointer for root node");
	assert_node(3, *(nodePointer + 2), 4, 4, 2, ASTNodeType_WHITESPACE,
		    " ");
	assert_node(4, *(nodePointer + 3), 5, 5, 3, ASTNodeType_TEXT, "bar");
	assert_node(5, *(nodePointer + 4), 8, 8, 4, ASTNodeType_WHITESPACE,
		    " ");
	assert_node(6, *(nodePointer + 5), 9, 9, 5, ASTNodeType_TEXT, "baz");
END_TEST}

START_TEST(parse_processesCommandContentAsChildNodes)
{
	ASTNodeVector *nodes = parseString("<foo>bar</foo>", false);
	ASTNode **nodePointer;
	assert(nodes != NULL
	       && nodes->size.length == 1, "Expected 1 root node");
	nodePointer = nodes->items;
	assert_node(1, *nodePointer, 0, 0, 0, ASTNodeType_COMMAND, "foo");
	assert((*nodePointer)->parent == NULL,
	       "Expected the parent pointer to be NULL for root node");
	assert((*nodePointer)->children != NULL
	       && (*nodePointer)->children->size.length == 1,
	       "Expected a single child node");
	assert((*(*nodePointer)->children->items)->parent == *nodePointer,
	       "Expected the child's parent pointer to point to the root node");
	nodePointer = (*nodePointer)->children->items;
	assert_node(2, *nodePointer, 5, 5, 1, ASTNodeType_TEXT, "bar");
END_TEST}

START_TEST(parse_processesNestedCommandsAsChildNodes)
{
	ASTNodeVector *nodes = parseString("<foo><bar>baz</bar></foo>", false);
	ASTNode *root, *child, *leaf;
	assert(nodes != NULL
	       && nodes->size.length == 1, "Expected 1 root node");
	root = *nodes->items;
	assert_node(1, root, 0, 0, 0, ASTNodeType_COMMAND, "foo");
	assert(root->children != NULL
	       && root->children->size.length == 1, "Expected 1 child node");
	child = *root->children->items;
	assert_node(2, child, 5, 5, 1, ASTNodeType_COMMAND, "bar");
	assert(child->children != NULL
	       && child->children->size.length == 1, "Expected 1 leaf node");
	leaf = *child->children->items;
	assert_node(3, leaf, 10, 10, 2, ASTNodeType_TEXT, "baz");
	assert(leaf->children == NULL, "Expected leaf's children to be NULL");
END_TEST}

START_TEST(parse_processesMultipleRootCommandsAsSiblings)
{
	ASTNodeVector *nodes =
	    parseString("<foo>bar</foo>baz<foobar>abc</foobar>", false);
	ASTNode **nodePointer;
	assert(nodes != NULL
	       && nodes->size.length == 3, "Expected 3 root nodes");
	nodePointer = nodes->items;
	assert_node(1, *nodePointer, 0, 0, 0, ASTNodeType_COMMAND, "foo");
	assert_node(2, *(nodePointer + 1), 14, 14, 3, ASTNodeType_TEXT, "baz");
	assert_node(3, *(nodePointer + 2), 17, 17, 4, ASTNodeType_COMMAND,
		    "foobar");
END_TEST}

START_TEST(parse_emptyCommandsHaveEmptyNonNullChildrenVector)
{
	ASTNodeVector *nodes = parseString("<a></a>", false);
	ASTNode *node;
	assert(nodes != NULL
	       && nodes->size.length == 1, "Expected 1 root node");
	node = *nodes->items;
	assert(node->children != NULL
	       && node->children->size.length == 0,
	       "Expected an empty vector of child nodes on the root node");
END_TEST}

START_TEST(parse_matchingBalancingCommandEndsInCaseInsensitiveWay)
{
	LexerResult *lexerResult = tokenize(string_from("<abc></ABC>"));
	ParserResult *result;
	ASTNodeVector *nodes;
	ASTNode *node;
	assert(lexerResult != NULL
	       && lexerResult->type == LexerResultType_SUCCESS,
	       "Expected successful tokenization");
	result = parse(lexerResult->result.tokens, false);
	assert(result != NULL
	       && result->type == ParserResultType_ERROR,
	       "Expected failed parsing");
	assert(result->result.error.code ==
	       ParserErrorCode_IMPROPERLY_BALANCED_COMMAND,
	       "Expected the IMPROPERLY_BALANCED_COMMAND error");
	assert(result->result.error.tokenIndex == 1,
	       "Expected the error token index to be 1");

	result = parse(lexerResult->result.tokens, true);
	assert(result != NULL
	       && result->type == ParserResultType_SUCCESS,
	       "Expected successful parsing");
	nodes = result->result.nodes;
	assert(nodes->size.length == 1, "Expected a single root node");
	node = *nodes->items;
	assert_node(1, node, 0, 0, 0, ASTNodeType_COMMAND, "abc");
END_TEST}

START_TEST(parse_processesNonBalancingCommandsWithoutNesting)
{
	LexerResult *lexerResult = tokenize(string_from("<lt><nl><np>"));
	ParserResult *result;
	ASTNodeVector *nodes;
	ASTNode **nodePointer;
	assert(lexerResult != NULL
	       && lexerResult->type == LexerResultType_SUCCESS,
	       "Expected successful tokenization");
	result = parse(lexerResult->result.tokens, false);
	assert(result != NULL
	       && result->type == ParserResultType_SUCCESS,
	       "Expected successful parsing");
	nodes = result->result.nodes;
	assert(nodes->size.length == 3, "Expected 3 root nodes");
	nodePointer = nodes->items;
	assert_node(1, *nodePointer, 0, 0, 0, ASTNodeType_COMMAND, "lt");
	assert_node(2, *(nodePointer + 1), 4, 4, 1, ASTNodeType_COMMAND, "nl");
	assert_node(3, *(nodePointer + 2), 8, 8, 2, ASTNodeType_COMMAND, "np");

	lexerResult = tokenize(string_from("<Np><lT><NL>"));
	assert(lexerResult != NULL
	       && lexerResult->type == LexerResultType_SUCCESS,
	       "Expected successful tokenization");
	result = parse(lexerResult->result.tokens, false);
	assert(result != NULL
	       && result->type == ParserResultType_ERROR,
	       "Expected parsing error");
	assert(result->result.error.code ==
	       ParserErrorCode_UNTERMINATED_COMMAND,
	       "Expected the UNTERMINATED_COMMAND error");

	result = parse(lexerResult->result.tokens, true);
	assert(result != NULL
	       && result->type == ParserResultType_SUCCESS,
	       "Expected successful parsing");
	assert(result->result.nodes->size.length == 3, "Expected 3 root nodes");
	nodePointer = result->result.nodes->items;
	assert_node(1, *nodePointer, 0, 0, 0, ASTNodeType_COMMAND, "Np");
	assert_node(2, *(nodePointer + 1), 4, 4, 1, ASTNodeType_COMMAND, "lT");
	assert_node(3, *(nodePointer + 2), 8, 8, 2, ASTNodeType_COMMAND, "NL");
END_TEST}

START_TEST(parse_correctlyReflectsTokenCodepointIndexes)
{
	ASTNodeVector *nodes = parseString(" <a>\302\205x</a>", false);
	ASTNode *node;
	assert(nodes != NULL
	       && nodes->size.length == 2, "Expected 2 root nodes");
	node = *((*(nodes->items + 1))->children->items + 1);
	assert_node(4, node, 6, 5, 3, ASTNodeType_TEXT, "x");
END_TEST}

START_TEST(parse_doesNotAllowBalancingEndCommandOfLtNlNp)
{
	char *commands[6];
	ParserResult *result;
	unsigned int i = 0;
	char *inputText = malloc(sizeof(char) * 10);

	commands[0] = "lt";
	commands[1] = "LT";
	commands[2] = "nl";
	commands[3] = "NL";
	commands[4] = "np";
	commands[5] = "NP";

	for (; i < 6; i++) {
		sprintf(inputText, "<%s></%s>", commands[i], commands[i]);
		result = parseStringToResult(inputText, i % 2);
		assert(result != NULL && result->type == ParserResultType_ERROR,
		       "Expected error result");
		assert_error(result->result.error, 4, 4, 1,
			     ParserErrorCode_UNALLOWED_BALANCING_COMMAND_END);
	}
END_TEST}

START_TEST(parse_rejectsUnterminatedCommands)
{				/* except for lt, nl & np */
	ParserResult *result = parseStringToResult("<z>a</z> <ab><cd>xyz</cd>",
						   false);
	assert(result != NULL
	       && result->type == ParserResultType_ERROR,
	       "Expected error result");
	assert_error(result->result.error, 9, 9, 4,
		     ParserErrorCode_UNTERMINATED_COMMAND);
END_TEST}

START_TEST(parse_rejectsUnpairedCommandEnds)
{
	ParserResult *result =
	    parseStringToResult("text <bar>a</bar></foo>0", false);
	assert(result != NULL
	       && result->type == ParserResultType_ERROR,
	       "Expected error result");
	assert_error(result->result.error, 17, 17, 5,
		     ParserErrorCode_UNEXPECTED_COMMAND_END);
END_TEST}

START_TEST(parse_improperlyBalancedCommands)
{
	ParserResult *result =
	    parseStringToResult("<foo><bar>baz</foo></bar>", false);
	assert(result != NULL
	       && result->type == ParserResultType_ERROR,
	       "Expected error result");
	assert_error(result->result.error, 13, 13, 3,
		     ParserErrorCode_IMPROPERLY_BALANCED_COMMAND);
END_TEST}

START_TEST(parse_rejectsInvalidTokenType)
{
	LexerResult *lexerResult = tokenize(string_from("foo"));
	ParserResult *result;
	assert(lexerResult != NULL
	       && lexerResult->type == LexerResultType_SUCCESS,
	       "Expected tokenization to succeed");
	lexerResult->result.tokens->items->type = 12000;
	result = parse(lexerResult->result.tokens, false);
	assert(result != NULL
	       && result->type == ParserResultType_ERROR,
	       "Expected parsing error");
	assert_error(result->result.error, 0, 0, 0,
		     ParserErrorCode_UNSUPPORTED_TOKEN_TYPE);
END_TEST}

START_TEST(ParserResult_free_acceptsNull)
{
	ParserResult_free(NULL);
END_TEST}

START_TEST(ParserResult_free_acceptsSuccessfulParsingResult)
{
	ParserResult *result = parseStringToResult("<foo>bar</foo>", false);
	ParserResult_free(result);
END_TEST}

START_TEST(ParserResult_free_acceptsErrorParsingResult)
{
	ParserResult *result = parseStringToResult("</baz>", false);
	ParserResult_free(result);
END_TEST}

static void all_tests()
{
	runTest(parse_returnsNullTokensErrorForNullInput);
	runTest(parse_processesPlainText);
	runTest(parse_processesCommandContentAsChildNodes);
	runTest(parse_processesNestedCommandsAsChildNodes);
	runTest(parse_processesMultipleRootCommandsAsSiblings);
	runTest(parse_emptyCommandsHaveEmptyNonNullChildrenVector);
	runTest(parse_matchingBalancingCommandEndsInCaseInsensitiveWay);
	runTest(parse_processesNonBalancingCommandsWithoutNesting);
	runTest(parse_correctlyReflectsTokenCodepointIndexes);
	runTest(parse_doesNotAllowBalancingEndCommandOfLtNlNp);
	runTest(parse_rejectsUnterminatedCommands);
	runTest(parse_rejectsUnpairedCommandEnds);
	runTest(parse_improperlyBalancedCommands);
	runTest(parse_rejectsInvalidTokenType);
	runTest(ParserResult_free_acceptsNull);
	runTest(ParserResult_free_acceptsSuccessfulParsingResult);
	runTest(ParserResult_free_acceptsErrorParsingResult);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}

static ParserResult *parseStringToResult(input, caseInsensitiveCommands)
const char *input;
bool caseInsensitiveCommands;
{
	LexerResult *lexerResult = tokenize(string_from(input));
	if (lexerResult == NULL || lexerResult->type == LexerResultType_ERROR) {
		return NULL;
	}
	return parse(lexerResult->result.tokens, caseInsensitiveCommands);
}

static ASTNodeVector *parseString(input, caseInsensitiveCommands)
const char *input;
bool caseInsensitiveCommands;
{
	ParserResult *parserResult =
	    parseStringToResult(input, caseInsensitiveCommands);
	if (parserResult == NULL
	    || parserResult->type == ParserResultType_ERROR) {
		return NULL;
	}
	return parserResult->result.nodes;
}

static char *STRINGIFIED_NODE_TYPE[] = {
	"ASTNodeType_COMMAND",
	"ASTNodeType_TEXT",
	"ASTNodeType_WHITESPACE"
};

static char *_assert_node(filename, line, nodeOrdinalNumber, node, byteIndex,
			  codepointIndex, tokenIndex, type, value)
char *filename;
unsigned int line;
unsigned int nodeOrdinalNumber;
ASTNode *node;
unsigned long byteIndex;
unsigned long codepointIndex;
unsigned long tokenIndex;
ASTNodeType type;
char *value;
{
	char *errorFormat;
	char *errorMessage;
	string *valueString;

	if (node == NULL) {
		return unit_assert(filename, line, node != NULL,
				   "Expected the node to be non-NULL");
	}

	errorFormat =
	    "Expected the %u. node's byteIndex to be %lu, but was %lu";
	errorMessage =
	    malloc(sizeof(char) * (strlen(errorFormat) + 20 + 20 + 20 + 1));
	sprintf(errorMessage, errorFormat, nodeOrdinalNumber, byteIndex,
		node->byteIndex);
	errorMessage =
	    unit_assert(filename, line, node->byteIndex == byteIndex,
			errorMessage);
	if (errorMessage != NULL) {
		return errorMessage;
	}

	errorFormat =
	    "Expected the %u. node's codepointIndex to be %lu, but was %lu";
	errorMessage =
	    malloc(sizeof(char) * (strlen(errorFormat) + 20 + 20 + 20 + 1));
	sprintf(errorMessage, errorFormat, nodeOrdinalNumber, codepointIndex,
		node->codepointIndex);
	errorMessage =
	    unit_assert(filename, line, node->codepointIndex == codepointIndex,
			errorMessage);
	if (errorMessage != NULL) {
		return errorMessage;
	}

	errorFormat =
	    "Expected the %u. node's tokenIndex to be %lu, but was %lu";
	errorMessage =
	    malloc(sizeof(char) * (strlen(errorFormat) + 20 + 20 + 20 + 1));
	sprintf(errorMessage, errorFormat, nodeOrdinalNumber, tokenIndex,
		node->tokenIndex);
	errorMessage =
	    unit_assert(filename, line, node->tokenIndex == tokenIndex,
			errorMessage);
	if (errorMessage != NULL) {
		return errorMessage;
	}

	errorFormat = "Expect the %u. node's type to be %s, but was %s";
	errorMessage =
	    malloc(sizeof(char) * (strlen(errorFormat) + 20 + 47 + 47 + 1));
	sprintf(errorMessage, errorFormat, nodeOrdinalNumber,
		STRINGIFIED_NODE_TYPE[type], STRINGIFIED_NODE_TYPE[node->type]);
	errorMessage =
	    unit_assert(filename, line, node->type == type, errorMessage);
	if (errorMessage != NULL) {
		return errorMessage;
	}

	valueString = string_from(value);
	errorFormat = "Expected the %u. node's value to be %s, but was %s";
	errorMessage =
	    malloc(sizeof(char) *
		   (strlen(errorFormat) + 20 + valueString->length +
		    node->value->length + 1));
	sprintf(errorMessage, errorFormat, nodeOrdinalNumber, value,
		node->value->content);
	errorMessage =
	    unit_assert(filename, line,
			string_compare(node->value, valueString) == 0,
			errorMessage);
	if (errorMessage != NULL) {
		return errorMessage;
	}

	return NULL;
}

static char *STRINGIFIED_ERROR_CODE[] = {
	"ParserErrorCode_OK",
	"ParserErrorCode_NULL_TOKENS",
	"ParserErrorCode_UNALLOWED_BALANCING_COMMAND_END",
	"ParserErrorCode_UNTERMINATED_COMMAND",
	"ParserErrorCode_UNEXPECTED_COMMAND_END",
	"ParserErrorCode_IMPROPERLY_BALANCED_COMMAND",
	"ParserErrorCode_OUT_OF_MEMORY_FOR_STRINGS",
	"ParserErrorCode_OUT_OF_MEMORY_FOR_NODES",
	"ParserErrorCode_UNSUPPORTED_TOKEN_TYPE",
	"ParserErrorCode_INTERNAL_ASSERTION_FAILED"
};

static char *_assert_error(fileName, line, error, byteIndex, codepointIndex,
			   tokenIndex, code)
char *fileName;
unsigned int line;
ParserError error;
unsigned long byteIndex;
unsigned long codepointIndex;
unsigned long tokenIndex;
ParserErrorCode code;
{
	char *assertError;

	assertError =
	    unit_assertUnsignedLongEquals(fileName, line, "error's byteIndex",
					  error.byteIndex, byteIndex);
	if (assertError != NULL) {
		return assertError;
	}
	assertError =
	    unit_assertUnsignedLongEquals(fileName, line,
					  "error's codepointIndex",
					  error.codepointIndex, codepointIndex);
	if (assertError != NULL) {
		return assertError;
	}
	assertError =
	    unit_assertUnsignedLongEquals(fileName, line, "error's tokenIndex",
					  error.tokenIndex, tokenIndex);
	if (assertError != NULL) {
		return assertError;
	}
	return unit_assertCStringEquals(fileName, line, "error's code",
					STRINGIFIED_ERROR_CODE[error.code],
					STRINGIFIED_ERROR_CODE[code]);
}
