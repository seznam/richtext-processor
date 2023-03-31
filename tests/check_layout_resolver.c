#include <limits.h>
#include <stdio.h>
#include "../src/bool.h"
#include "../src/layout_resolver.h"
#include "../src/parser.h"
#include "../src/string.h"
#include "unit.h"

/*
   This file does not bother to free heap-allocated memory because it a suite
	 of unit tests, ergo it's not a big deal.
 */

unsigned int tests_run = 0;
unsigned int tests_failed = 0;

static char *STRINGIFIED_WARNING_CODES[2];
static char *STRINGIFIED_ERROR_CODES[30];
static char *STRINGIFIED_AST_NODE_TYPE[3];

static void all_tests(void);

int main(void);

static char *_assert_result(char *fileName,
			    unsigned int lineOfCode,
			    ASTNodePointerVector * nodes,
			    CustomCommandLayoutInterpretation
			    customCommandHook(ASTNode *, bool),
			    bool caseInsensitiveCommands,
			    LayoutResolverResult ** resultPointer);

static char *_assert_error(char *fileName,
			   unsigned int lineOfCode,
			   ASTNodePointerVector * nodes,
			   CustomCommandLayoutInterpretation
			   customCommandHook(ASTNode *, bool),
			   bool caseInsensitiveCommands,
			   LayoutResolverErrorCode errorCode,
			   ASTNode * errorLocation);

static char *stringifyASTNode(ASTNode * node);

static ASTNode *makeNode(unsigned long byteIndex, unsigned long codepointIndex,
			 unsigned long tokenIndex, ASTNodeType type,
			 char *value, ASTNode * parent, ASTNode * childNodes[]);

static ASTNode *newNode(unsigned long byteIndex, unsigned long codepointIndex,
			unsigned long tokenIndex, ASTNodeType type,
			string * value, ASTNode * parent,
			ASTNodePointerVector * children);

#define assert_result(nodes, customCommandHook, caseInsensitiveCommands,\
		      resultPointer)\
do {\
	char *_assert_failure =\
	    _assert_result(__FILE__, __LINE__, nodes, customCommandHook,\
			   caseInsensitiveCommands, resultPointer);\
	if (_assert_failure != NULL) {\
		return _assert_failure;\
	}\
} while (false)

#define assert_error(nodes, customCommandHook, caseInsensitiveCommands,\
		     errorCode, errorLocation)\
do {\
	char *_assert_failure =\
	    _assert_error(__FILE__, __LINE__, nodes, customCommandHook,\
	    caseInsensitiveCommands, errorCode, errorLocation);\
	if (_assert_failure != NULL) {\
		return _assert_failure;\
	}\
} while (false)

START_TEST(resolveLayout_returnsErrorForNullNodes)
{
	assert_error(NULL, NULL, true,
		     LayoutResolverErrorCode_NULL_NODES_PROVIDED, NULL);
END_TEST}

START_TEST(resolveLayout_returnsErrorForUnsupportedNodeType)
{
	ASTNode *children[1];
	ASTNode *node;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	children[0] = NULL;
	node = makeNode(0, 0, 0, 65000, "", NULL, children);
	ASTNodePointerVector_append(nodes, &node);
	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_UNSUPPORTED_NODE_TYPE, node);
END_TEST}

static CustomCommandLayoutInterpretation _customCommandRejector(ASTNode * node,
								bool
								ignoreCase);

static CustomCommandLayoutInterpretation
_invalidCustomCommandInterpreter(ASTNode * node, bool caseInsensitive);

START_TEST(resolveLayout_returnsErrorIfCustomCommandHookRejectsCommand)
{
	ASTNode *children[1];
	ASTNode *node;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	children[0] = NULL;
	node =
	    makeNode(0, 0, 0, ASTNodeType_COMMAND, "x-custom", NULL, children);
	ASTNodePointerVector_append(nodes, &node);
	assert_error(nodes, _customCommandRejector, false,
		     LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND, node);
END_TEST}

START_TEST(resolveLayout_returnsErrorIfCustomCommandHookReturnsInvalidValue)
{
	LayoutResolverErrorCode INVALID_INTERPRETATION_ERROR =
	    LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND_INTERPRETATION;
	ASTNode *children[1];
	ASTNode *node;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	children[0] = NULL;
	node =
	    makeNode(0, 0, 0, ASTNodeType_COMMAND, "x-custom", NULL, children);
	ASTNodePointerVector_append(nodes, &node);
	assert_error(nodes, _invalidCustomCommandInterpreter, false,
		     INVALID_INTERPRETATION_ERROR, node);
END_TEST}

START_TEST(resolveLayout_returnsErrorForTooHighLeftOutdentLevel)
{
	unsigned long requiredNesting = -SHRT_MIN + 1;
	ASTNode *children[2];
	ASTNode *node = NULL;
	ASTNode *lastNode = NULL;
	ASTNode *deepestNode = NULL;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	unsigned long i;
	children[0] = NULL;
	children[1] = NULL;
	for (i = 0; i < requiredNesting; i++) {
		node =
		    makeNode(9 * (requiredNesting - 1 - i),
			     9 * (requiredNesting - 1 - i),
			     (requiredNesting - 1 - i), ASTNodeType_COMMAND,
			     "Outdent", NULL, children);
		if (deepestNode == NULL) {
			deepestNode = node;
		}
		if (lastNode != NULL) {
			lastNode->parent = node;
		}
		lastNode = node;
		children[0] = node;
	}
	ASTNodePointerVector_append(nodes, &node);

	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_LEFT_INDENTATION_LEVEL_UNDERFLOW,
		     deepestNode);
END_TEST}

START_TEST(resolveLayout_returnsErrorForTooHighLeftIndentLevel)
{
	unsigned long requiredNesting = SHRT_MAX + 1;
	ASTNode *children[2];
	ASTNode *node = NULL;
	ASTNode *lastNode = NULL;
	ASTNode *deepestNode = NULL;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	unsigned long i;
	children[0] = NULL;
	children[1] = NULL;
	for (i = 0; i < requiredNesting; i++) {
		node =
		    makeNode(8 * (requiredNesting - 1 - i),
			     8 * (requiredNesting - 1 - i),
			     (requiredNesting - 1 - i), ASTNodeType_COMMAND,
			     "Indent", NULL, children);
		if (deepestNode == NULL) {
			deepestNode = node;
		}
		if (lastNode != NULL) {
			lastNode->parent = node;
		}
		lastNode = node;
		children[0] = node;
	}
	ASTNodePointerVector_append(nodes, &node);

	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_LEFT_INDENTATION_LEVEL_OVERFLOW,
		     deepestNode);
END_TEST}

START_TEST(resolveLayout_returnsErrorForTooHighRightOutdentLevel)
{
	unsigned long requiredNesting = -SHRT_MIN + 1;
	ASTNode *children[2];
	ASTNode *node = NULL;
	ASTNode *lastNode = NULL;
	ASTNode *deepestNode = NULL;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	unsigned long i;
	children[0] = NULL;
	children[1] = NULL;
	for (i = 0; i < requiredNesting; i++) {
		node =
		    makeNode(14 * (requiredNesting - 1 - i),
			     14 * (requiredNesting - 1 - i),
			     (requiredNesting - 1 - i), ASTNodeType_COMMAND,
			     "OutdentRight", NULL, children);
		if (deepestNode == NULL) {
			deepestNode = node;
		}
		if (lastNode != NULL) {
			lastNode->parent = node;
		}
		lastNode = node;
		children[0] = node;
	}
	ASTNodePointerVector_append(nodes, &node);

	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_RIGHT_INDENTATION_LEVEL_UNDERFLOW,
		     deepestNode);
END_TEST}

START_TEST(resolveLayout_returnsErrorForTooHighRightIndentLevel)
{
	unsigned long requiredNesting = SHRT_MAX + 1;
	ASTNode *children[2];
	ASTNode *node = NULL;
	ASTNode *lastNode = NULL;
	ASTNode *deepestNode = NULL;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	unsigned long i;
	children[0] = NULL;
	children[1] = NULL;
	for (i = 0; i < requiredNesting; i++) {
		node =
		    makeNode(13 * (requiredNesting - 1 - i),
			     13 * (requiredNesting - 1 - i),
			     (requiredNesting - 1 - i), ASTNodeType_COMMAND,
			     "IndentRight", NULL, children);
		if (deepestNode == NULL) {
			deepestNode = node;
		}
		if (lastNode != NULL) {
			lastNode->parent = node;
		}
		lastNode = node;
		children[0] = node;
	}
	ASTNodePointerVector_append(nodes, &node);

	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_RIGHT_INDENTATION_LEVEL_OVERFLOW,
		     deepestNode);
END_TEST}

START_TEST(resolveLayout_returnsErrorForTooSmallFontSize)
{
	unsigned long requiredNesting = -SHRT_MIN + 1;
	ASTNode *children[2];
	ASTNode *node = NULL;
	ASTNode *lastNode = NULL;
	ASTNode *deepestNode = NULL;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	unsigned long i;
	children[0] = NULL;
	children[1] = NULL;
	for (i = 0; i < requiredNesting; i++) {
		node =
		    makeNode(9 * (requiredNesting - 1 - i),
			     9 * (requiredNesting - 1 - i),
			     (requiredNesting - 1 - i), ASTNodeType_COMMAND,
			     "Smaller", NULL, children);
		if (deepestNode == NULL) {
			deepestNode = node;
		}
		if (lastNode != NULL) {
			lastNode->parent = node;
		}
		lastNode = node;
		children[0] = node;
	}
	ASTNodePointerVector_append(nodes, &node);

	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_FONT_SIZE_CHANGE_UNDERFLOW,
		     deepestNode);
END_TEST}

START_TEST(resolveLayout_returnsErrorForTooLargeFontSize)
{
	unsigned long requiredNesting = SHRT_MAX + 1;
	ASTNode *children[2];
	ASTNode *node = NULL;
	ASTNode *lastNode = NULL;
	ASTNode *deepestNode = NULL;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	unsigned long i;
	children[0] = NULL;
	children[1] = NULL;
	for (i = 0; i < requiredNesting; i++) {
		node =
		    makeNode(8 * (requiredNesting - 1 - i),
			     8 * (requiredNesting - 1 - i),
			     (requiredNesting - 1 - i), ASTNodeType_COMMAND,
			     "Bigger", NULL, children);
		if (deepestNode == NULL) {
			deepestNode = node;
		}
		if (lastNode != NULL) {
			lastNode->parent = node;
		}
		lastNode = node;
		children[0] = node;
	}
	ASTNodePointerVector_append(nodes, &node);

	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_FONT_SIZE_CHANGE_OVERFLOW,
		     deepestNode);
END_TEST}

START_TEST(resolveLayout_returnsErrorForTooManyNestedBoldCommands)
{
	unsigned long requiredNesting = USHRT_MAX + 1;
	ASTNode *children[2];
	ASTNode *node = NULL;
	ASTNode *lastNode = NULL;
	ASTNode *deepestNode = NULL;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	unsigned long i;
	children[0] = NULL;
	children[1] = NULL;
	for (i = 0; i < requiredNesting; i++) {
		node =
		    makeNode(6 * (requiredNesting - 1 - i),
			     6 * (requiredNesting - 1 - i),
			     (requiredNesting - 1 - i), ASTNodeType_COMMAND,
			     "Bold", NULL, children);
		if (deepestNode == NULL) {
			deepestNode = node;
		}
		if (lastNode != NULL) {
			lastNode->parent = node;
		}
		lastNode = node;
		children[0] = node;
	}
	ASTNodePointerVector_append(nodes, &node);

	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_FONT_BOLD_LEVEL_OVERFLOW,
		     deepestNode);
END_TEST}

START_TEST(resolveLayout_returnsErrorForTooManyNestedItalicCommands)
{
	unsigned long requiredNesting = USHRT_MAX + 1;
	ASTNode *children[2];
	ASTNode *node = NULL;
	ASTNode *lastNode = NULL;
	ASTNode *deepestNode = NULL;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	unsigned long i;
	children[0] = NULL;
	children[1] = NULL;
	for (i = 0; i < requiredNesting; i++) {
		node =
		    makeNode(8 * (requiredNesting - 1 - i),
			     8 * (requiredNesting - 1 - i),
			     (requiredNesting - 1 - i), ASTNodeType_COMMAND,
			     "Italic", NULL, children);
		if (deepestNode == NULL) {
			deepestNode = node;
		}
		if (lastNode != NULL) {
			lastNode->parent = node;
		}
		lastNode = node;
		children[0] = node;
	}
	ASTNodePointerVector_append(nodes, &node);

	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_FONT_ITALIC_LEVEL_OVERFLOW,
		     deepestNode);
END_TEST}

START_TEST(resolveLayout_returnsErrorForTooManyNestedUnderlineCommands)
{
	unsigned long requiredNesting = USHRT_MAX + 1;
	ASTNode *children[2];
	ASTNode *node = NULL;
	ASTNode *lastNode = NULL;
	ASTNode *deepestNode = NULL;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	unsigned long i;
	children[0] = NULL;
	children[1] = NULL;
	for (i = 0; i < requiredNesting; i++) {
		node =
		    makeNode(9 * (requiredNesting - 1 - i),
			     9 * (requiredNesting - 1 - i),
			     (requiredNesting - 1 - i), ASTNodeType_COMMAND,
			     "Underline", NULL, children);
		if (deepestNode == NULL) {
			deepestNode = node;
		}
		if (lastNode != NULL) {
			lastNode->parent = node;
		}
		lastNode = node;
		children[0] = node;
	}
	ASTNodePointerVector_append(nodes, &node);

	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_FONT_UNDERLINED_LEVEL_OVERFLOW,
		     deepestNode);
END_TEST}

START_TEST(resolveLayout_returnsErrorForTooManyNestedFixedCommands)
{
	unsigned long requiredNesting = USHRT_MAX + 1;
	ASTNode *children[2];
	ASTNode *node = NULL;
	ASTNode *lastNode = NULL;
	ASTNode *deepestNode = NULL;
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	unsigned long i;
	children[0] = NULL;
	children[1] = NULL;
	for (i = 0; i < requiredNesting; i++) {
		node =
		    makeNode(7 * (requiredNesting - 1 - i),
			     7 * (requiredNesting - 1 - i),
			     (requiredNesting - 1 - i), ASTNodeType_COMMAND,
			     "Fixed", NULL, children);
		if (deepestNode == NULL) {
			deepestNode = node;
		}
		if (lastNode != NULL) {
			lastNode->parent = node;
		}
		lastNode = node;
		children[0] = node;
	}
	ASTNodePointerVector_append(nodes, &node);

	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_FONT_FIXED_LEVEL_OVERFLOW,
		     deepestNode);
END_TEST}

START_TEST(resolveLayout_returnsErrorForNodeChildrenContainingNULL)
{
	ASTNodePointerVector *nodes = ASTNodePointerVector_new(0, 1);
	ASTNode *node = NULL;
	ASTNodePointerVector_append(nodes, &node);
	node =
	    newNode(0, 0, 0, ASTNodeType_COMMAND, string_from("Paragraph"),
		    NULL, nodes);
	nodes = ASTNodePointerVector_new(0, 1);
	ASTNodePointerVector_append(nodes, &node);
	assert_error(nodes, NULL, false,
		     LayoutResolverErrorCode_NULL_NODES_PROVIDED, node);
END_TEST}

static void all_tests()
{
	runTest(resolveLayout_returnsErrorForNullNodes);
	runTest(resolveLayout_returnsErrorForUnsupportedNodeType);
	runTest(resolveLayout_returnsErrorIfCustomCommandHookRejectsCommand);
	runTest
	    (resolveLayout_returnsErrorIfCustomCommandHookReturnsInvalidValue);
	runTest(resolveLayout_returnsErrorForTooHighLeftOutdentLevel);
	runTest(resolveLayout_returnsErrorForTooHighLeftIndentLevel);
	runTest(resolveLayout_returnsErrorForTooHighRightOutdentLevel);
	runTest(resolveLayout_returnsErrorForTooHighRightIndentLevel);
	runTest(resolveLayout_returnsErrorForTooSmallFontSize);
	runTest(resolveLayout_returnsErrorForTooLargeFontSize);
	runTest(resolveLayout_returnsErrorForTooManyNestedBoldCommands);
	runTest(resolveLayout_returnsErrorForTooManyNestedItalicCommands);
	runTest(resolveLayout_returnsErrorForTooManyNestedUnderlineCommands);
	runTest(resolveLayout_returnsErrorForTooManyNestedFixedCommands);
	runTest(resolveLayout_returnsErrorForNodeChildrenContainingNULL);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}

static char *_assert_result(fileName, lineOfCode, nodes, customCommandHook,
			    caseInsensitiveCommands, resultPointer)
char *fileName;
unsigned int lineOfCode;
ASTNodePointerVector *nodes;
CustomCommandLayoutInterpretation customCommandHook(ASTNode *, bool);
bool caseInsensitiveCommands;
LayoutResolverResult **resultPointer;
{
	char *failure;
	*resultPointer =
	    resolveLayout(nodes, customCommandHook, caseInsensitiveCommands);
	failure =
	    unit_assert(fileName, lineOfCode, *resultPointer != NULL,
			"Expected a non-NULL result");
	if (failure != NULL) {
		return failure;
	}

	return unit_assert(fileName, lineOfCode,
			   (*resultPointer)->warnings != NULL,
			   "Expected result's warnings vector to not be NULL");
}

static char *_assert_error(fileName, lineOfCode, nodes, customCommandHook,
			   caseInsensitiveCommands, errorCode, errorLocation)
char *fileName;
unsigned int lineOfCode;
ASTNodePointerVector *nodes;
CustomCommandLayoutInterpretation customCommandHook(ASTNode *, bool);
bool caseInsensitiveCommands;
LayoutResolverErrorCode errorCode;
ASTNode *errorLocation;
{
	LayoutResolverResult *result;
	char *stringifiedExpectedErrorCode =
	    errorCode < 30 ? STRINGIFIED_ERROR_CODES[errorCode] : "<UNKNOWN>";
	char *stringifiedObtainedErrorCode = NULL;
	char *errorCodeAssertFailureFormat =
	    "Expected the error code to be %s, but was %s";
	char *errorCodeAssertFailure = NULL;
	char *stringifiedExpectedErrorLocation =
	    stringifyASTNode(errorLocation);
	char *stringifiedObtainedErrorLocation;
	char *errorLocationAssertFailureFormat =
	    "Expected the error location to be %s, but was %s";
	char *errorLocationAssertFailure = NULL;
	char *failure;

	failure =
	    _assert_result(fileName, lineOfCode, nodes, customCommandHook,
			   caseInsensitiveCommands, &result);
	if (failure != NULL) {
		return failure;
	}

	failure =
	    unit_assert(fileName, lineOfCode,
			result->type == LayoutResolverResultType_ERROR,
			"Expected an error result");
	if (failure != NULL) {
		return failure;
	}

	stringifiedObtainedErrorCode = result->result.error.code < 30 ?
	    STRINGIFIED_ERROR_CODES[result->result.error.code] : "<UNKNOWN>";
	errorCodeAssertFailure =
	    malloc(sizeof(char) *
		   (strlen(errorCodeAssertFailureFormat) +
		    strlen(stringifiedObtainedErrorCode) +
		    strlen(stringifiedExpectedErrorCode) + 1));
	sprintf(errorCodeAssertFailure, errorCodeAssertFailureFormat,
		stringifiedExpectedErrorCode, stringifiedObtainedErrorCode);
	failure =
	    unit_assert(fileName, lineOfCode,
			result->result.error.code == errorCode,
			errorCodeAssertFailure);
	if (failure != NULL) {
		return failure;
	}

	stringifiedObtainedErrorLocation =
	    stringifyASTNode(result->result.error.location);
	errorLocationAssertFailure =
	    malloc(sizeof(char) *
		   (strlen(errorLocationAssertFailureFormat) +
		    strlen(stringifiedExpectedErrorLocation) +
		    strlen(stringifiedObtainedErrorLocation) + 1));
	sprintf(errorLocationAssertFailure, errorLocationAssertFailureFormat,
		stringifiedExpectedErrorLocation,
		stringifiedObtainedErrorLocation);
	failure =
	    unit_assert(fileName, lineOfCode,
			result->result.error.location == errorLocation,
			errorLocationAssertFailure);
	if (failure != NULL) {
		return failure;
	}

	return NULL;
}

static char *stringifyASTNode(node)
ASTNode *node;
{
	char *format;
	char *stringifiedType;
	char *stringifiedParent;
	char *stringifiedChildrenFormat;
	char *stringifiedChildren;
	char *result;

	if (node == NULL) {
		return "NULL";
	}

	format =
	    "ASTNode{byteIndex: %lu, codepointIndex: %lu, tokenIndex: %lu, type: %s, value: \"%s\", parent: %s, children: %s}";
	stringifiedType =
	    node->type <
	    3 ? STRINGIFIED_AST_NODE_TYPE[node->type] : "<UNKNOWN>";
	stringifiedParent = node->parent == NULL ? "NULL" : "ASTNode{...}";
	stringifiedChildrenFormat = "ASTNodePointerVector[%lu]{...}";
	stringifiedChildren = "NULL";
	result = NULL;

	if (node->children != NULL) {
		stringifiedChildren =
		    malloc(sizeof(char) *
			   (strlen(stringifiedChildrenFormat) + 20 + 1));
		sprintf(stringifiedChildren, stringifiedChildrenFormat,
			node->children->size.length);
	}

	result =
	    malloc(sizeof(char) *
		   (strlen(format) + 20 + 20 + 20 + strlen(stringifiedType) +
		    node->value->length + strlen(stringifiedParent) +
		    strlen(stringifiedChildren) + 1));
	sprintf(result, format, node->byteIndex, node->codepointIndex,
		node->tokenIndex, stringifiedType, node->value->content,
		stringifiedParent, stringifiedChildren);
	return result;
}

static char *STRINGIFIED_WARNING_CODES[] = {
	"LayoutResolverWarningCode_NEW_PAGE_INSIDE_SAME_PAGE",
	"LayoutResolverWarningCode_NESTED_SAME_PAGE"
};

static char *STRINGIFIED_ERROR_CODES[] = {
	"LayoutResolverErrorCode_OK",
	"LayoutResolverErrorCode_NULL_NODES_PROVIDED",
	"LayoutResolverErrorCode_UNSUPPORTED_NODE_TYPE",
	"LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND",
	"LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND_INTERPRETATION",
	"LayoutResolverErrorCode_UNSUPPORTED_LAYOUT_INTERPRETATION",
	"LayoutResolverErrorCode_UNTRANSLATED_CUSTOM_LAYOUT_INTERPRETATION",
	"LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINE_SEGMENTS",
	"LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINES",
	"LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_PARAGRAPHS",
	"LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_BLOCKS",
	"LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_WARNINGS",
	"LayoutResolverErrorCode_BLOCK_TYPE_STACK_UNDERFLOW",
	"LayoutResolverErrorCode_CONTENT_ALIGNMENT_STACK_UNDERFLOW",
	"LayoutResolverErrorCode_OTHER_SEGMENT_MARKER_STACK_UNDERFLOW",
	"LayoutResolverErrorCode_OTHER_SEGMENT_MARKER_STACK_INCONSISTENCY",
	"LayoutResolverErrorCode_LEFT_INDENTATION_LEVEL_UNDERFLOW",
	"LayoutResolverErrorCode_LEFT_INDENTATION_LEVEL_OVERFLOW",
	"LayoutResolverErrorCode_RIGHT_INDENTATION_LEVEL_UNDERFLOW",
	"LayoutResolverErrorCode_RIGHT_INDENTATION_LEVEL_OVERFLOW",
	"LayoutResolverErrorCode_FONT_SIZE_CHANGE_UNDERFLOW",
	"LayoutResolverErrorCode_FONT_SIZE_CHANGE_OVERFLOW",
	"LayoutResolverErrorCode_FONT_BOLD_LEVEL_UNDERFLOW",
	"LayoutResolverErrorCode_FONT_BOLD_LEVEL_OVERFLOW",
	"LayoutResolverErrorCode_FONT_ITALIC_LEVEL_UNDERFLOW",
	"LayoutResolverErrorCode_FONT_ITALIC_LEVEL_OVERFLOW",
	"LayoutResolverErrorCode_FONT_UNDERLINED_LEVEL_UNDERFLOW",
	"LayoutResolverErrorCode_FONT_UNDERLINED_LEVEL_OVERFLOW",
	"LayoutResolverErrorCode_FONT_FIXED_LEVEL_UNDERFLOW",
	"LayoutResolverErrorCode_FONT_FIXED_LEVEL_OVERFLOW"
};

static char *STRINGIFIED_AST_NODE_TYPE[] = {
	"ASTNodeType_COMMAND",
	"ASTNodeType_TEXT",
	"ASTNodeType_WHITESPACE"
};

static ASTNode *makeNode(byteIndex, codepointIndex,
			 tokenIndex, type, value, parent, childNodes)
unsigned long byteIndex;
unsigned long codepointIndex;
unsigned long tokenIndex;
ASTNodeType type;
char *value;
ASTNode *parent;
ASTNode *childNodes[];
{
	ASTNodePointerVector *children = ASTNodePointerVector_new(0, 0);
	ASTNode **child = childNodes;

	for (; *child != NULL; child++) {
		ASTNodePointerVector_append(children, child);
	}

	return newNode(byteIndex, codepointIndex, tokenIndex, type,
		       string_from(value), parent, children);
}

static ASTNode *newNode(byteIndex, codepointIndex, tokenIndex, type, value,
			parent, children)
unsigned long byteIndex;
unsigned long codepointIndex;
unsigned long tokenIndex;
ASTNodeType type;
string *value;
ASTNode *parent;
ASTNodePointerVector *children;
{
	ASTNode *node = malloc(sizeof(ASTNode));
	node->byteIndex = byteIndex;
	node->codepointIndex = codepointIndex;
	node->tokenIndex = tokenIndex;
	node->type = type;
	node->value = value;
	node->parent = parent;
	node->children = children;
	return node;
}

static CustomCommandLayoutInterpretation _customCommandRejector(node,
								ignoreCase)
ASTNode *node;
bool ignoreCase;
{
	/* silence the warnings about unused parameters */
	if (node || ignoreCase) {
		return CustomCommandLayoutInterpretation_INVALID_COMMAND;
	}

	return CustomCommandLayoutInterpretation_INVALID_COMMAND;
}

static CustomCommandLayoutInterpretation
_invalidCustomCommandInterpreter(node, caseInsensitive)
ASTNode *node;
bool caseInsensitive;
{
	/* silence the warnings about unused parameters */
	return node || caseInsensitive ? 65000 : 65000;
}
