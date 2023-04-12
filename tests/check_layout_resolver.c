#include <limits.h>
#include <stdio.h>
#include "../src/bool.h"
#include "../src/layout_resolver.h"
#include "../src/lexer.h"
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
static char *STRINGIFIED_ERROR_CODES[31];
static char *STRINGIFIED_AST_NODE_TYPE[3];
static char *STRINGIFIED_LAYOUT_CONTENT_ALIGNMENT[4];
static char *STRINGIFIED_LAYOUT_PARAGRAPH_TYPE[2];
static char *STRINGIFIED_LAYOUT_BLOCK_TYPE[7];

static void all_tests(void);

int main(void);

static char *_assert_result(char *fileName,
			    unsigned int lineOfCode,
			    ASTNodePointerVector * nodes,
			    CustomCommandLayoutInterpretation
			    customCommandHook(ASTNode *, bool),
			    bool caseInsensitiveCommands,
			    LayoutResolverResult ** resultPointer);

static char *_assert_success(char *fileName, unsigned int lineOfCode,
			     LayoutResolverResult * result,
			     unsigned long expectedWarnings,
			     unsigned long expectedBlocks);

static char *_assert_error(char *fileName,
			   unsigned int lineOfCode,
			   ASTNodePointerVector * nodes,
			   CustomCommandLayoutInterpretation
			   customCommandHook(ASTNode *, bool),
			   bool caseInsensitiveCommands,
			   LayoutResolverErrorCode errorCode,
			   ASTNode * errorLocation);

static LayoutBlock LayoutBlock_make(LayoutBlockType type,
				    ASTNode * causingCommand,
				    LayoutParagraphVector * paragraphs);

static char *LayoutBlockVector_assertEqual(char *fileName,
					   unsigned int lineOfCode,
					   LayoutBlockVector * blocks1,
					   LayoutBlockVector * blocks2);

static char *LayoutBlock_assertEqual(char *fileName, unsigned int lineOfCode,
				     LayoutBlock block1, LayoutBlock block2);

static LayoutParagraph LayoutParagraph_make(ASTNode * causingCommand,
					    LayoutParagraphType type,
					    LayoutLineVector * lines);

static char *LayoutParagraphVector_assertEqual(char *fileName,
					       unsigned int lineOfCode,
					       LayoutParagraphVector *
					       paragraphs1,
					       LayoutParagraphVector *
					       paragraphs2);

static char *LayoutParagraph_assertEqual(char *fileName,
					 unsigned int lineOfCode,
					 LayoutParagraph paragraph1,
					 LayoutParagraph paragraph2);

static LayoutLine LayoutLine_make(ASTNode * causingCommand,
				  LayoutLineSegmentVector * segments);

static char *LayoutLineVector_assertEqual(char *fileName,
					  unsigned int lineOfCode,
					  LayoutLineVector * lines1,
					  LayoutLineVector * lines2);

static char *LayoutLine_assertEqual(char *fileName, unsigned int lineOfCode,
				    LayoutLine line1, LayoutLine line2);

static LayoutLineSegment
LayoutLineSegment_make(ASTNode * causingCommand,
		       LayoutContentAlignment contentAlignment,
		       signed int leftIndentationLevel,
		       signed int rightIndentationLevel,
		       signed int fontSizeChange,
		       unsigned int fontBoldLevel,
		       unsigned int fontItalicLevel,
		       unsigned int fontUnderlinedLevel,
		       unsigned int fontFixedLevel,
		       ASTNodePointerVector * otherSegmentMarkers,
		       ASTNodePointerVector * content);

static char *LayoutLineSegmentVector_assertEqual(char *fileName,
						 unsigned int lineOfCode,
						 LayoutLineSegmentVector *
						 segments1,
						 LayoutLineSegmentVector *
						 segments2);

static char *LayoutLineSegment_assertEqual(char *fileName,
					   unsigned int lineOfCode,
					   LayoutLineSegment segment1,
					   LayoutLineSegment segment2);

static char *ASTNodePointerVector_assertEqual(ASTNodePointerVector * nodes1,
					      ASTNodePointerVector * nodes2);

static ASTNode ASTNode_make(unsigned long byteIndex,
			    unsigned long codepointIndex,
			    unsigned long tokenIndex, ASTNodeType type,
			    string * value, ASTNode * parent,
			    ASTNodePointerVector * children);

static bool ASTNode_equalsShallow(ASTNode node1, ASTNode node2);

static char *stringifyASTNode(ASTNode * node);

static LayoutResolverResult *process(const char *richtext,
				     CustomCommandLayoutInterpretation
				     customCommandHook(ASTNode *, bool),
				     bool caseInsensitiveCommands);

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

#define assert_success(result, expectedWarnings, expectedBlocks)\
do {\
	char *_assert_failure =\
	    _assert_success(__FILE__, __LINE__, result, expectedWarnings,\
			    expectedBlocks);\
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

START_TEST(resolveLayout_parsesSingleTextToSingleSegmentResult)
{
	LayoutResolverResult *result = process("hogoblog", NULL, false);
	LayoutParagraph *paragraph;
	LayoutLineSegment *segment;
	assert_success(result, 0, 1);
	assert(result->result.blocks->items->type ==
	       LayoutBlockType_MAIN_CONTENT,
	       "Expected the block to have the LayoutBlockType_MAIN_CONTENT type");
	assert(result->result.blocks->items->paragraphs->size.length == 1,
	       "Expected 1 paragraph");
	paragraph = result->result.blocks->items->paragraphs->items;
	assert(paragraph->type == LayoutParagraphType_IMPLICIT,
	       "Expected the paragraph to have the LayoutParagraphType_IMPLICIT type");
	assert(paragraph->lines->size.length == 1, "Expected 1 line");
	assert(paragraph->lines->items->segments->size.length == 1,
	       "Expected 1 segment");
	segment = paragraph->lines->items->segments->items;
	assert(segment->contentAlignment == LayoutContentAlignment_DEFAULT,
	       "Expected the segment to have LayoutContentAlignment_DEFAULT content alignment");
	assert(segment->leftIndentationLevel == 0,
	       "Expected the segment to have 0 left indentation level");
	assert(segment->rightIndentationLevel == 0,
	       "Expected the segment to have 0 right indentation level");
	assert(segment->fontSizeChange == 0,
	       "Expected the segment to have 0 font size change");
	assert(segment->fontBoldLevel == 0,
	       "Expected the segment to have 0 font bold level");
	assert(segment->fontItalicLevel == 0,
	       "Expected the segment to have 0 font italic level");
	assert(segment->fontUnderlinedLevel == 0,
	       "Expected the segment to have 0 font underline level");
	assert(segment->fontFixedLevel == 0,
	       "Expected the segment to have 0 font fixed level");
	assert(segment->otherSegmentMarkers->size.length == 0,
	       "Expected the segment's other segment markers vector to be empty");
	assert(segment->content->size.length == 1,
	       "Expected a 1 node in the segment's content");
	assert((*segment->content->items)->type == ASTNodeType_TEXT,
	       "Expected the segment's content node to have the ASTNodeType_TEXT type");
	assert(string_compare
	       ((*segment->content->items)->value,
		string_from("hogoblog")) == 0,
	       "Expected the content node to have the 'hogoblog' value");
END_TEST}

#define make_node(byteIndex, codepointIndex, tokenIndex, type, value)\
ASTNode_make(byteIndex, codepointIndex, tokenIndex, ASTNodeType_##type,\
	     string_from(value), NULL, NULL)

#define make_nodes0()\
ASTNodePointerVector_new(0, 0)

#define make_nodes1(node1)\
ASTNodePointerVector_of1(&node1)

#define make_segment(causingCommand, contentAlignment, leftIndentationLevel,\
		rightIndentationLevel, fontSizeChange, fontBoldLevel,\
		fontItalicLevel, fontUnderlinedLevel, fontFixedLevel,\
		otherSegmentMarkers, content)\
LayoutLineSegment_make(&causingCommand,\
		       LayoutContentAlignment_##contentAlignment,\
		       leftIndentationLevel, rightIndentationLevel,\
		       fontSizeChange, fontBoldLevel, fontItalicLevel,\
		       fontUnderlinedLevel, fontFixedLevel,\
		       otherSegmentMarkers, content)

#define make_segments1(segment1)\
LayoutLineSegmentVector_of1(segment1)

#define make_line(causingCommand, segments)\
LayoutLine_make(&causingCommand, segments)

#define make_lines1(line1)\
LayoutLineVector_of1(line1)

#define make_paragraph(causingCommand, type, lines)\
LayoutParagraph_make(&causingCommand, LayoutParagraphType_##type, lines)

#define make_paragraphs1(paragraph1)\
LayoutParagraphVector_of1(paragraph1)

#define make_block(type, causingCommand, paragraphs)\
LayoutBlock_make(LayoutBlockType_##type, &causingCommand, paragraphs)

#define assert_blocks_match(blocks1, blocks2)\
do {\
	char *_assert_failure =\
	    LayoutBlockVector_assertEqual(__FILE__, __LINE__, blocks1,\
					  blocks2);\
	if (_assert_failure != NULL) {\
		return _assert_failure;\
	}\
} while (false)

START_TEST(resolveLayout_processesMainContentAndFootingInsideHeading)
{
	LayoutResolverResult *result =
	    process("<Heading>text1<Footing>text2</Footing></Heading>text3",
		    NULL, false);
	LayoutBlockVector *expectedResult;
	ASTNode nodes[5];
	LayoutLineSegment segments[3];
	LayoutLine lines[3];
	LayoutParagraph paragraphs[3];
	LayoutBlock blocks[3];

	nodes[0] = make_node(0, 0, 0, COMMAND, "Heading");
	nodes[1] = make_node(9, 9, 1, TEXT, "text1");
	nodes[2] = make_node(14, 14, 2, COMMAND, "Footing");
	nodes[3] = make_node(23, 23, 3, TEXT, "text2");
	nodes[4] = make_node(48, 48, 6, TEXT, "text3");

	segments[0] =
	    make_segment(nodes[0], DEFAULT, 0, 0, 0, 0, 0, 0, 0, make_nodes0(),
			 make_nodes1(nodes[1]));
	segments[1] =
	    make_segment(nodes[2], DEFAULT, 0, 0, 0, 0, 0, 0, 0, make_nodes0(),
			 make_nodes1(nodes[3]));
	segments[2] =
	    make_segment(nodes[4], DEFAULT, 0, 0, 0, 0, 0, 0, 0, make_nodes0(),
			 make_nodes1(nodes[4]));

	lines[0] = make_line(nodes[0], make_segments1(segments[0]));
	lines[1] = make_line(nodes[2], make_segments1(segments[1]));
	lines[2] = make_line(nodes[4], make_segments1(segments[2]));

	paragraphs[0] =
	    make_paragraph(nodes[0], IMPLICIT, make_lines1(lines[0]));
	paragraphs[1] =
	    make_paragraph(nodes[2], IMPLICIT, make_lines1(lines[1]));
	paragraphs[2] =
	    make_paragraph(nodes[4], IMPLICIT, make_lines1(lines[2]));

	blocks[0] =
	    make_block(HEADING, nodes[0], make_paragraphs1(paragraphs[0]));
	blocks[1] =
	    make_block(FOOTING, nodes[2], make_paragraphs1(paragraphs[1]));
	blocks[2] =
	    make_block(MAIN_CONTENT, nodes[4], make_paragraphs1(paragraphs[2]));

	expectedResult = LayoutBlockVector_of3(blocks[0], blocks[1], blocks[2]);

	assert_success(result, 0, 3);
	assert_blocks_match(expectedResult, result->result.blocks);
END_TEST}

#undef assert_blocks_match
#undef make_block
#undef make_paragraphs1
#undef make_paragraph
#undef make_lines1
#undef make_line
#undef make_segments1
#undef make_segment
#undef make_nodes1
#undef make_nodes0
#undef make_node

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
	runTest(resolveLayout_parsesSingleTextToSingleSegmentResult);
	runTest(resolveLayout_processesMainContentAndFootingInsideHeading);
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

static char *_assert_success(fileName, lineOfCode, result, expectedWarnings,
			     expectedBlocks)
char *fileName;
unsigned int lineOfCode;
LayoutResolverResult *result;
unsigned long expectedWarnings;
unsigned long expectedBlocks;
{
	char *failure;
	char *warningErrorFormat =
	    "Expected %lu warnings, but the result had %lu warnings";
	char *warningError;
	char *blocksErrorFormat =
	    "Expected %lu blocks, but the result had %lu blocks";
	char *blocksError;

	failure =
	    unit_assert(fileName, lineOfCode, result != NULL,
			"Expected non-NULL result");
	if (failure != NULL) {
		return failure;
	}

	failure =
	    unit_assert(fileName, lineOfCode,
			result->type == LayoutResolverResultType_SUCCESS,
			"Expected successful result");
	if (failure != NULL) {
		return failure;
	}

	failure =
	    unit_assert(fileName, lineOfCode, result->warnings != NULL,
			"Expected the result to have non-NULL warnings vector");
	if (failure != NULL) {
		return failure;
	}

	warningError =
	    malloc(sizeof(char) + (strlen(warningErrorFormat) + 20 + 20 + 1));
	sprintf(warningError, warningErrorFormat, expectedWarnings,
		result->warnings->size.length);
	failure =
	    unit_assert(fileName, lineOfCode,
			result->warnings->size.length == expectedWarnings,
			warningError);
	if (failure != NULL) {
		return failure;
	}

	blocksError =
	    malloc(sizeof(char) * (strlen(blocksErrorFormat) + 20 + 20 + 1));
	sprintf(blocksError, blocksErrorFormat, expectedBlocks,
		result->result.blocks->size.length);
	return unit_assert(fileName, lineOfCode,
			   result->result.blocks->size.length == expectedBlocks,
			   blocksError);
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

static LayoutBlock LayoutBlock_make(type, causingCommand, paragraphs)
LayoutBlockType type;
ASTNode *causingCommand;
LayoutParagraphVector *paragraphs;
{
	LayoutBlock block;
	block.type = type;
	block.causingCommand = causingCommand;
	block.paragraphs = paragraphs;
	return block;
}

static char *LayoutBlockVector_assertEqual(fileName, lineOfCode, blocks1,
					   blocks2)
char *fileName;
unsigned int lineOfCode;
LayoutBlockVector *blocks1;
LayoutBlockVector *blocks2;
{
	char *lengthErrorFormat =
	    "The 1st block vector has size of %lu, but the 2nd block vector has size of %lu";
	char *lengthError =
	    malloc(sizeof(char) * (strlen(lengthErrorFormat) + 20 + 20 + 1));
	char *itemErrorFormat = "The %lu. block did not match: %s";
	char *itemError;
	char *failure;
	unsigned long i;
	LayoutBlock *block1, *block2;

	sprintf(lengthError, lengthErrorFormat, blocks1->size.length,
		blocks2->size.length);
	failure =
	    unit_assert(fileName, lineOfCode,
			blocks1->size.length == blocks2->size.length,
			lengthError);
	if (failure != NULL) {
		return failure;
	}

	for (i = 0, block1 = blocks1->items, block2 = blocks2->items;
	     i < blocks1->size.length; i++, block1++, block2++) {
		failure = LayoutBlock_assertEqual(NULL, 0, *block1, *block2);
		if (failure != NULL) {
			itemError =
			    malloc(sizeof(char) *
				   (strlen(itemErrorFormat) + 20 +
				    strlen(failure) + 1));
			sprintf(itemError, itemErrorFormat, i, failure);
			return unit_assert(fileName, lineOfCode, false,
					   itemError);
		}
	}

	return NULL;
}

static char *LayoutBlock_assertEqual(fileName, lineOfCode, block1, block2)
char *fileName;
unsigned int lineOfCode;
LayoutBlock block1;
LayoutBlock block2;
{
	char *stringifiedNode1 = stringifyASTNode(block1.causingCommand);
	char *stringifiedNode2 = stringifyASTNode(block2.causingCommand);
	char *stringifiedType1 = block1.type >= 0 && block1.type < 7 ?
	    STRINGIFIED_LAYOUT_BLOCK_TYPE[block1.type] : "<UNKNOWN>";
	char *stringifiedType2 = block2.type >= 0 && block2.type < 7 ?
	    STRINGIFIED_LAYOUT_BLOCK_TYPE[block2.type] : "<UNKNOWN>";
	char *causingCommandErrorFormat =
	    "The causing command is not equal, it was %s in the 1st block, but %s in the 2nd block";
	char *causingCommandError =
	    malloc(sizeof(char) *
		   (strlen(causingCommandErrorFormat) +
		    strlen(stringifiedNode1) + strlen(stringifiedNode2) + 1));
	char *typeErrorFormat =
	    "The type is not equal, it was %s in the 1st block but %s in the 2nd block";
	char *typeError =
	    malloc(sizeof(char) *
		   (strlen(typeErrorFormat) + strlen(stringifiedType1) +
		    strlen(stringifiedType2) + 1));
	char *linesErrorFormat = "The paragraphs are not equal: %s";
	char *linesError;
	char *failure;

	sprintf(causingCommandError, causingCommandErrorFormat,
		stringifiedNode1, stringifiedNode2);
	failure =
	    unit_assert(fileName, lineOfCode,
			ASTNode_equalsShallow(*block1.causingCommand,
					      *block2.causingCommand),
			causingCommandError);
	if (failure != NULL) {
		return failure;
	}

	sprintf(typeError, typeErrorFormat, stringifiedType1, stringifiedType2);
	failure =
	    unit_assert(fileName, lineOfCode, block1.type == block2.type,
			typeError);
	if (failure != NULL) {
		return failure;
	}

	failure =
	    LayoutParagraphVector_assertEqual(fileName, lineOfCode,
					      block1.paragraphs,
					      block2.paragraphs);
	if (failure != NULL) {
		linesError =
		    malloc(sizeof(char) *
			   (strlen(linesErrorFormat) + strlen(failure) + 1));
		sprintf(linesError, linesErrorFormat, failure);
		return unit_assert(fileName, lineOfCode, false, linesError);
	}

	return NULL;
}

static LayoutParagraph LayoutParagraph_make(causingCommand, type, lines)
ASTNode *causingCommand;
LayoutParagraphType type;
LayoutLineVector *lines;
{
	LayoutParagraph paragraph;
	paragraph.causingCommand = causingCommand;
	paragraph.type = type;
	paragraph.lines = lines;
	return paragraph;
}

static char *LayoutParagraphVector_assertEqual(fileName, lineOfCode,
					       paragraphs1, paragraphs2)
char *fileName;
unsigned int lineOfCode;
LayoutParagraphVector *paragraphs1;
LayoutParagraphVector *paragraphs2;
{
	char *lengthErrorFormat =
	    "The 1st paragraph vector has size of %lu, but the 2nd paragraph vector has size of %lu";
	char *lengthError =
	    malloc(sizeof(char) * (strlen(lengthErrorFormat) + 20 + 20 + 1));
	char *itemErrorFormat = "The %lu. paragraph did not match: %s";
	char *itemError;
	char *failure;
	unsigned long i;
	LayoutParagraph *paragraph1, *paragraph2;

	sprintf(lengthError, lengthErrorFormat, paragraphs1->size.length,
		paragraphs2->size.length);
	failure =
	    unit_assert(fileName, lineOfCode,
			paragraphs1->size.length == paragraphs2->size.length,
			lengthError);
	if (failure != NULL) {
		return failure;
	}

	for (i = 0, paragraph1 = paragraphs1->items, paragraph2 =
	     paragraphs2->items; i < paragraphs1->size.length;
	     i++, paragraph1++, paragraph2++) {
		failure =
		    LayoutParagraph_assertEqual(fileName, lineOfCode,
						*paragraph1, *paragraph2);
		if (failure != NULL) {
			itemError =
			    malloc(sizeof(char) *
				   (strlen(itemErrorFormat) + 20 +
				    strlen(failure) + 1));
			sprintf(itemError, itemErrorFormat, i, failure);
			return unit_assert(fileName, lineOfCode, false,
					   itemError);
		}
	}

	return NULL;
}

static char *LayoutParagraph_assertEqual(fileName, lineOfCode, paragraph1,
					 paragraph2)
char *fileName;
unsigned int lineOfCode;
LayoutParagraph paragraph1;
LayoutParagraph paragraph2;
{
	char *stringifiedNode1 = stringifyASTNode(paragraph1.causingCommand);
	char *stringifiedNode2 = stringifyASTNode(paragraph2.causingCommand);
	char *stringifiedType1 = paragraph1.type >= 0 && paragraph1.type < 2 ?
	    STRINGIFIED_LAYOUT_PARAGRAPH_TYPE[paragraph1.type] : "<UNKNOWN>";
	char *stringifiedType2 = paragraph2.type >= 0 && paragraph2.type < 2 ?
	    STRINGIFIED_LAYOUT_PARAGRAPH_TYPE[paragraph2.type] : "<UNKNOWN>";
	char *causingCommandErrorFormat =
	    "The causing command is not equal, it was %s in the 1st line, but %s in the 2nd line";
	char *causingCommandError =
	    malloc(sizeof(char) *
		   (strlen(causingCommandErrorFormat) +
		    strlen(stringifiedNode1) + strlen(stringifiedNode2) + 1));
	char *typeErrorFormat =
	    "The type is not equal, it was %s in the 1st paragraph but %s in the 2nd paragraph";
	char *typeError =
	    malloc(sizeof(char) *
		   (strlen(typeErrorFormat) + strlen(stringifiedType1) +
		    strlen(stringifiedType2) + 1));
	char *linesErrorFormat = "The lines are not equal: %s";
	char *linesError;
	char *failure;

	sprintf(causingCommandError, causingCommandErrorFormat,
		stringifiedNode1, stringifiedNode2);
	failure =
	    unit_assert(fileName, lineOfCode,
			ASTNode_equalsShallow(*paragraph1.causingCommand,
					      *paragraph2.causingCommand),
			causingCommandError);
	if (failure != NULL) {
		return failure;
	}

	sprintf(typeError, typeErrorFormat, stringifiedType1, stringifiedType2);
	failure =
	    unit_assert(fileName, lineOfCode,
			paragraph1.type == paragraph2.type, typeError);
	if (failure != NULL) {
		return failure;
	}

	failure =
	    LayoutLineVector_assertEqual(fileName, lineOfCode, paragraph1.lines,
					 paragraph2.lines);
	if (failure != NULL) {
		linesError =
		    malloc(sizeof(char) *
			   (strlen(linesErrorFormat) + strlen(failure) + 1));
		sprintf(linesError, linesErrorFormat, failure);
		return unit_assert(fileName, lineOfCode, false, linesError);
	}

	return NULL;
}

static LayoutLine LayoutLine_make(causingCommand, segments)
ASTNode *causingCommand;
LayoutLineSegmentVector *segments;
{
	LayoutLine line;
	line.causingCommand = causingCommand;
	line.segments = segments;
	return line;
}

static char *LayoutLineVector_assertEqual(fileName, lineOfCode, lines1, lines2)
char *fileName;
unsigned int lineOfCode;
LayoutLineVector *lines1;
LayoutLineVector *lines2;
{
	char *lengthErrorFormat =
	    "The 1st line vector has size of %lu, but the 2nd line vector has size of %lu";
	char *lengthError =
	    malloc(sizeof(char) * (strlen(lengthErrorFormat) + 20 + 20 + 1));
	char *itemErrorFormat = "The %lu. line did not match: %s";
	char *itemError;
	char *failure;
	unsigned long i;
	LayoutLine *line1, *line2;

	sprintf(lengthError, lengthErrorFormat, lines1->size.length,
		lines2->size.length);
	failure =
	    unit_assert(fileName, lineOfCode,
			lines1->size.length == lines2->size.length,
			lengthError);
	if (failure != NULL) {
		return failure;
	}

	for (i = 0, line1 = lines1->items, line2 = lines2->items;
	     i < lines1->size.length; i++, line1++, line2++) {
		failure =
		    LayoutLine_assertEqual(fileName, lineOfCode, *line1,
					   *line2);
		if (failure != NULL) {
			itemError =
			    malloc(sizeof(char) *
				   (strlen(itemErrorFormat) + 20 +
				    strlen(failure) + 1));
			sprintf(itemError, itemErrorFormat, i, failure);
			return unit_assert(fileName, lineOfCode, false,
					   itemError);
		}
	}

	return NULL;
}

static char *LayoutLine_assertEqual(fileName, lineOfCode, line1, line2)
char *fileName;
unsigned int lineOfCode;
LayoutLine line1;
LayoutLine line2;
{
	char *causingCommandErrorFormat =
	    "The causing command is not equal, it was %s in the 1st line, but %s in the 2nd line";
	char *stringifiedCommand1 = stringifyASTNode(line1.causingCommand);
	char *stringifiedCommand2 = stringifyASTNode(line2.causingCommand);
	char *causingCommandError =
	    malloc(sizeof(char) *
		   (strlen(causingCommandErrorFormat) +
		    strlen(stringifiedCommand1) + strlen(stringifiedCommand2) +
		    1));
	char *failure;

	sprintf(causingCommandError, causingCommandErrorFormat,
		stringifiedCommand1, stringifiedCommand2);
	failure =
	    unit_assert(fileName, lineOfCode,
			ASTNode_equalsShallow(*line1.causingCommand,
					      *line2.causingCommand),
			causingCommandError);
	if (failure != NULL) {
		return failure;
	}

	failure =
	    LayoutLineSegmentVector_assertEqual(fileName, lineOfCode,
						line1.segments, line2.segments);
	if (failure != NULL) {
		return unit_assert(fileName, lineOfCode, false, failure);
	}

	return NULL;
}

static LayoutLineSegment
LayoutLineSegment_make(causingCommand, contentAlignment, leftIndentationLevel,
		       rightIndentationLevel, fontSizeChange, fontBoldLevel,
		       fontItalicLevel, fontUnderlinedLevel, fontFixedLevel,
		       otherSegmentMarkers, content)
ASTNode *causingCommand;
LayoutContentAlignment contentAlignment;
signed int leftIndentationLevel;
signed int rightIndentationLevel;
signed int fontSizeChange;
unsigned int fontBoldLevel;
unsigned int fontItalicLevel;
unsigned int fontUnderlinedLevel;
unsigned int fontFixedLevel;
ASTNodePointerVector *otherSegmentMarkers;
ASTNodePointerVector *content;
{
	LayoutLineSegment segment;
	segment.causingCommand = causingCommand;
	segment.contentAlignment = contentAlignment;
	segment.leftIndentationLevel = leftIndentationLevel;
	segment.rightIndentationLevel = rightIndentationLevel;
	segment.fontSizeChange = fontSizeChange;
	segment.fontBoldLevel = fontBoldLevel;
	segment.fontItalicLevel = fontItalicLevel;
	segment.fontUnderlinedLevel = fontUnderlinedLevel;
	segment.fontFixedLevel = fontFixedLevel;
	segment.otherSegmentMarkers = otherSegmentMarkers;
	segment.content = content;
	return segment;
}

static char *LayoutLineSegmentVector_assertEqual(fileName, lineOfCode,
						 segments1, segments2)
char *fileName;
unsigned int lineOfCode;
LayoutLineSegmentVector *segments1;
LayoutLineSegmentVector *segments2;
{
	char *lengthErrorFormat =
	    "The 1st layout line segment vector has size of %lu, but the 2nd layout line segment vector has size of %lu";
	char *lengthError =
	    malloc(sizeof(char) * (strlen(lengthErrorFormat) + 20 + 20 + 1));
	char *itemErrorFormat = "The %lu. line segment did not match: %s";
	char *itemError;
	LayoutLineSegment *segment1, *segment2;
	unsigned long i;
	char *failure;

	sprintf(lengthError, lengthErrorFormat, segments1->size.length,
		segments2->size.length);
	failure =
	    unit_assert(fileName, lineOfCode,
			segments1->size.length == segments2->size.length,
			lengthError);
	if (failure != NULL) {
		return failure;
	}

	for (i = 0, segment1 = segments1->items, segment2 = segments2->items;
	     i < segments1->size.length; i++, segment1++, segment2++) {
		failure =
		    LayoutLineSegment_assertEqual(fileName, lineOfCode,
						  *segment1, *segment2);
		if (failure != NULL) {
			itemError =
			    malloc(sizeof(char) *
				   (strlen(itemErrorFormat) + 20 +
				    strlen(failure) + 1));
			sprintf(itemError, itemErrorFormat, i, failure);
			return unit_assert(fileName, lineOfCode, false,
					   itemError);
		}
	}

	return NULL;
}

static char *LayoutLineSegment_assertEqual(fileName, lineOfCode, segment1,
					   segment2)
char *fileName;
unsigned int lineOfCode;
LayoutLineSegment segment1;
LayoutLineSegment segment2;
{
	char *failure;
	char *stringifiedCausingCommand1 =
	    stringifyASTNode(segment1.causingCommand);
	char *stringifiedCausingCommand2 =
	    stringifyASTNode(segment2.causingCommand);
	char *causingCommandErrorFormat =
	    "The causingCommand property is not equal. It is set to %s in 1st segment, but set to %s in 2nd segment";
	char *causingCommandError =
	    malloc(sizeof(char) *
		   (strlen(causingCommandErrorFormat) +
		    strlen(stringifiedCausingCommand1) +
		    strlen(stringifiedCausingCommand2) + 1));
	char *stringifiedContentAlignment1 = segment1.contentAlignment >= 0
	    && segment1.contentAlignment <
	    4 ? STRINGIFIED_LAYOUT_CONTENT_ALIGNMENT[segment1.contentAlignment]
	    : "<UNKNOWN>";
	char *stringifiedContentAlignment2 = segment2.contentAlignment >= 0
	    && segment2.contentAlignment <
	    4 ? STRINGIFIED_LAYOUT_CONTENT_ALIGNMENT[segment2.contentAlignment]
	    : "<UNKNOWN>";
	char *contentAlignmentErrorFormat =
	    "The contentAlignment property is not equal: It is set to %s in 1st segment, but set to %s in 2nd segment";
	char *contentAlignmentError =
	    malloc(sizeof(char) *
		   (strlen(contentAlignmentErrorFormat) +
		    strlen(stringifiedContentAlignment1) +
		    strlen(stringifiedContentAlignment2) + 1));
	char *leftIndentationLevelErrorFormat =
	    "The leftIndentationLevel property is not equal: It is set to %hi in 1st segment, but set to %hi in 2nd segment";
	char *leftIndentationLevelError =
	    malloc(sizeof(char) *
		   (strlen(leftIndentationLevelErrorFormat) + 6 + 6 + 1));
	char *rightIndentationLevelErrorFormat =
	    "The rightIndentationLevel property is not equal: It is set to %hi in 1st segment, but set to %hi in 2nd segment";
	char *rightIndentationLevelError =
	    malloc(sizeof(char) *
		   (strlen(rightIndentationLevelErrorFormat) + 6 + 6 + 1));
	char *fontSizeChangeErrorFormat =
	    "The fontSizeChange property is not equal: It is set to %hi in 1st segment, but set to %hi in 2nd segment";
	char *fontSizeChangeError =
	    malloc(sizeof(char) *
		   (strlen(fontSizeChangeErrorFormat) + 6 + 6 + 1));
	char *fontBoldLevelErrorFormat =
	    "The fontBoldLevel property is not equal: It is set to %hu in 1st segment, but set to %hu in 2nd segment";
	char *fontBoldLevelError =
	    malloc(sizeof(char) *
		   (strlen(fontBoldLevelErrorFormat) + 5 + 5 + 1));
	char *fontItalicLevelErrorFormat =
	    "The fontItalicLevel property is not equal: It is set to %hu in 1st segment, but set to %hu in 2nd segment";
	char *fontItalicLevelError =
	    malloc(sizeof(char) *
		   (strlen(fontItalicLevelErrorFormat) + 5 + 5 + 1));
	char *fontUnderlinedLevelErrorFormat =
	    "The fontUnderlinedLevel property is not equal: It is set to %hu in 1st segment, but set to %hu in 2nd segment";
	char *fontUnderlinedLevelError =
	    malloc(sizeof(char) *
		   (strlen(fontUnderlinedLevelErrorFormat) + 5 + 5 + 1));
	char *fontFixedLevelErrorFormat =
	    "The fontFixedLevel property is not equal: It is set to %hu in 1st segment, but set to %hu";
	char *fontFixedLevelError =
	    malloc(sizeof(char) *
		   (strlen(fontFixedLevelErrorFormat) + 5 + 5 + 1));
	char *otherSegmentMarkersErrorFormat =
	    "The otherSegmentMarkers property are not equal: %s";
	char *otherSegmentMarkersError;
	char *contentErrorFormat = "The content property is not equal: %s";
	char *contentError;

	sprintf(causingCommandError, causingCommandErrorFormat,
		stringifiedCausingCommand1, stringifiedCausingCommand2);
	failure =
	    unit_assert(fileName, lineOfCode,
			ASTNode_equalsShallow(*segment1.causingCommand,
					      *segment2.causingCommand),
			causingCommandError);
	if (failure != NULL) {
		return failure;
	}

	sprintf(contentAlignmentError, contentAlignmentErrorFormat,
		stringifiedContentAlignment1, stringifiedContentAlignment2);
	failure =
	    unit_assert(fileName, lineOfCode,
			segment1.contentAlignment == segment2.contentAlignment,
			contentAlignmentError);
	if (failure != NULL) {
		return failure;
	}

	sprintf(leftIndentationLevelError, leftIndentationLevelErrorFormat,
		segment1.leftIndentationLevel, segment2.leftIndentationLevel);
	failure =
	    unit_assert(fileName, lineOfCode,
			segment1.leftIndentationLevel ==
			segment2.leftIndentationLevel,
			leftIndentationLevelError);
	if (failure != NULL) {
		return failure;
	}

	sprintf(rightIndentationLevelError, rightIndentationLevelErrorFormat,
		segment1.rightIndentationLevel, segment2.rightIndentationLevel);
	failure =
	    unit_assert(fileName, lineOfCode,
			segment1.rightIndentationLevel ==
			segment2.rightIndentationLevel,
			rightIndentationLevelError);
	if (failure != NULL) {
		return failure;
	}

	sprintf(fontSizeChangeError, fontSizeChangeErrorFormat,
		segment1.fontSizeChange, segment2.fontSizeChange);
	failure =
	    unit_assert(fileName, lineOfCode,
			segment1.fontSizeChange == segment2.fontSizeChange,
			fontSizeChangeError);
	if (failure != NULL) {
		return failure;
	}

	sprintf(fontBoldLevelError, fontBoldLevelErrorFormat,
		segment1.fontBoldLevel, segment2.fontBoldLevel);
	failure =
	    unit_assert(fileName, lineOfCode,
			segment1.fontBoldLevel == segment2.fontBoldLevel,
			fontBoldLevelError);
	if (failure != NULL) {
		return failure;
	}

	sprintf(fontItalicLevelError, fontItalicLevelErrorFormat,
		segment1.fontItalicLevel, segment2.fontItalicLevel);
	failure =
	    unit_assert(fileName, lineOfCode,
			segment1.fontItalicLevel == segment2.fontItalicLevel,
			fontItalicLevelError);
	if (failure != NULL) {
		return failure;
	}

	sprintf(fontUnderlinedLevelError, fontUnderlinedLevelErrorFormat,
		segment1.fontUnderlinedLevel, segment2.fontUnderlinedLevel);
	failure =
	    unit_assert(fileName, lineOfCode,
			segment1.fontUnderlinedLevel ==
			segment2.fontUnderlinedLevel, fontUnderlinedLevelError);
	if (failure != NULL) {
		return failure;
	}

	failure =
	    unit_assert(fileName, lineOfCode,
			segment1.fontFixedLevel == segment2.fontFixedLevel,
			fontFixedLevelError);
	if (failure != NULL) {
		return failure;
	}

	failure =
	    ASTNodePointerVector_assertEqual(segment1.otherSegmentMarkers,
					     segment2.otherSegmentMarkers);
	if (failure != NULL) {
		otherSegmentMarkersError =
		    malloc(sizeof(char) *
			   (strlen(otherSegmentMarkersErrorFormat) +
			    strlen(failure) + 1));
		sprintf(otherSegmentMarkersError,
			otherSegmentMarkersErrorFormat, failure);
		return unit_assert(fileName, lineOfCode, false,
				   otherSegmentMarkersError);
	}

	failure =
	    ASTNodePointerVector_assertEqual(segment1.content,
					     segment2.content);
	if (failure != NULL) {
		contentError =
		    malloc(sizeof(char) *
			   (strlen(contentErrorFormat) + strlen(failure) + 1));
		sprintf(contentError, contentErrorFormat, failure);
		return unit_assert(fileName, lineOfCode, false, contentError);
	}

	return NULL;
}

static char *ASTNodePointerVector_assertEqual(nodes1, nodes2)
ASTNodePointerVector *nodes1;
ASTNodePointerVector *nodes2;
{
	char *lengthErrorFormat =
	    "The 1st vector has size of %lu, but the 2nd vector has size of %lu";
	char *lengthError =
	    malloc(sizeof(char) * (strlen(lengthErrorFormat) + 20 + 20 + 1));
	char *itemUnequalErrorFormat =
	    "The %lu. item is not equal. The item is %s in 1st vector, but %s in 2nd vector";
	char *itemUnequalError;
	char *stringifiedNode1, *stringifiedNode2;
	unsigned long i;
	ASTNode **node1Pointer, **node2Pointer;

	sprintf(lengthError, lengthErrorFormat, nodes1->size.length,
		nodes2->size.length);
	if (nodes1->size.length != nodes2->size.length) {
		return lengthError;
	}

	for (i = 0, node1Pointer = nodes1->items, node2Pointer = nodes2->items;
	     i < nodes1->size.length; i++, node1Pointer++, node2Pointer++) {
		if (!ASTNode_equalsShallow(**node1Pointer, **node2Pointer)) {
			stringifiedNode1 = stringifyASTNode(*node1Pointer);
			stringifiedNode2 = stringifyASTNode(*node2Pointer);
			itemUnequalError =
			    malloc(sizeof(char) *
				   (strlen(itemUnequalErrorFormat) + 20 +
				    strlen(stringifiedNode1) +
				    strlen(stringifiedNode2) + 1));
			sprintf(itemUnequalError, itemUnequalErrorFormat, i,
				stringifiedNode1, stringifiedNode2);
			return itemUnequalError;
		}
	}

	return NULL;
}

static ASTNode ASTNode_make(byteIndex, codepointIndex, tokenIndex, type, value,
			    parent, children)
unsigned long byteIndex;
unsigned long codepointIndex;
unsigned long tokenIndex;
ASTNodeType type;
string *value;
ASTNode *parent;
ASTNodePointerVector *children;
{
	ASTNode node;
	node.byteIndex = byteIndex;
	node.codepointIndex = codepointIndex;
	node.tokenIndex = tokenIndex;
	node.type = type;
	node.value = value;
	node.parent = parent;
	node.children = children;
	return node;
}

static bool ASTNode_equalsShallow(node1, node2)
ASTNode node1;
ASTNode node2;
{
	if (node1.byteIndex != node2.byteIndex
	    || node1.codepointIndex != node2.codepointIndex
	    || node1.tokenIndex != node2.tokenIndex) {
		return false;
	}
	if (node1.type != node2.type) {
		return false;
	}
	if (string_compare(node1.value, node2.value) != 0) {
		return false;
	}

	return true;
}

static char *stringifyASTNode(node)
ASTNode *node;
{
	char *format;
	char *stringifiedType;
	char *stringifiedValue;
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
	stringifiedValue = calloc(sizeof(char), node->value->length + 1);
	memcpy(stringifiedValue, node->value->content, node->value->length);
	sprintf(result, format, node->byteIndex, node->codepointIndex,
		node->tokenIndex, stringifiedType, stringifiedValue,
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

static char *STRINGIFIED_LAYOUT_CONTENT_ALIGNMENT[] = {
	"LayoutContentAlignment_DEFAULT",
	"LayoutContentAlignment_JUSTIFY_LEFT",
	"LayoutContentAlignment_JUSTIFY_RIGHT",
	"LayoutContentAlignment_CENTER"
};

static char *STRINGIFIED_LAYOUT_PARAGRAPH_TYPE[] = {
	"LayoutParagraphType_EXPLICIT",
	"LayoutParagraphType_IMPLICIT"
};

static char *STRINGIFIED_LAYOUT_BLOCK_TYPE[] = {
	"LayoutBlockType_HEADING",
	"LayoutBlockType_FOOTING",
	"LayoutBlockType_MAIN_CONTENT",
	"LayoutBlockType_PAGE_BREAK",
	"LayoutBlockType_SAME_PAGE_START",
	"LayoutBlockType_SAME_PAGE_END",
	"LayoutBlockType_CUSTOM"
};

static LayoutResolverResult *process(richtext, customCommandHook,
				     caseInsensitiveCommands)
const char *richtext;
CustomCommandLayoutInterpretation customCommandHook(ASTNode *, bool);
bool caseInsensitiveCommands;
{
	ParserResult *parserResult;
	LexerResult *tokens = tokenize(string_from(richtext));
	if (tokens == NULL || tokens->type != LexerResultType_SUCCESS) {
		return NULL;
	}

	parserResult = parse(tokens->result.tokens, caseInsensitiveCommands);
	if (parserResult == NULL
	    || parserResult->type != ParserResultType_SUCCESS) {
		return NULL;
	}

	return resolveLayout(parserResult->result.nodes, customCommandHook,
			     caseInsensitiveCommands);
}

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
