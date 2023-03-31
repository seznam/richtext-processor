#include <limits.h>
#include <stdlib.h>
#include "bool.h"
#include "layout_resolver.h"
#include "parser.h"
#include "string.h"
#include "vector.h"

typedef enum CommandLayoutInterpretation {
	/*
	 * The contents of the command will be interpreted as page heading
	 * content.
	 */
	CommandLayoutInterpretation_HEADING_BLOCK,

	/*
	 * The contents of the command will be interpreted as page footing
	 * content.
	 */
	CommandLayoutInterpretation_FOOTING_BLOCK,

	/*
	 * Can be used for implementing additional content areas, for example an
	 * aside column.
	 */
	CommandLayoutInterpretation_NEW_BLOCK,

	/*
	 * Move the subsequent content to a new page.
	 */
	CommandLayoutInterpretation_NEW_PAGE,

	/*
	 * The content of the command should be, if possible, layed out on a
	 * single page.
	 */
	CommandLayoutInterpretation_SAME_PAGE,

	/*
	 * Starts a new implicit paragraph that continues after the causing
	 * command's end.
	 */
	CommandLayoutInterpretation_NEW_PARAGRAPH,

	/*
	 * Starts a new implicit paragraph that ends after the causing command's
	 * end.
	 */
	CommandLayoutInterpretation_NEW_ISOLATED_PARAGRAPH,

	/*
	 * Starts a new line of content.
	 */
	CommandLayoutInterpretation_NEW_LINE,

	/*
	 * Start a new line that ends afters the causing command's end.
	 */
	CommandLayoutInterpretation_NEW_ISOLATED_LINE,

	/*
	 * Starts a new line segment that ends after the causing command's end.
	 */
	CommandLayoutInterpretation_NEW_LINE_SEGMENT,

	/*
	 * The command will be appended to the current line's content.
	 */
	CommandLayoutInterpretation_INLINE_CONTENT,

	/*
	 * Same as INLINE_CONTENT, except any children of the command will just
	 * be added to the current line's content with the command itself, and
	 * the children will not have any effect on the computed layout blocks.
	 */
	CommandLayoutInterpretation_COMMENT,

	/*
	 * The command has no effect, layout, formatting or otherwise, and will
	 * be replaced by its children. This is the default behavior for custom
	 * commands if no custom command hook is provided.
	 */
	CommandLayoutInterpretation_NO_OP,

	/*
	 * The command is not a standard richtext formatting command but a
	 * custom one. Its layout interpretation will be determined by a custom
	 * command hook function, if provided, otherwise it will be treated as a
	 * No-op command.
	 */
	CommandLayoutInterpretation_CUSTOM,

	/*
	 * Used when the custom command layout interpreter returns the
	 * CustomCommandLayoutInterpretation_INVALID_COMMAND result. Will cause
	 * the layout resolver to return the
	 * LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND error.
	 */
	CommandLayoutInterpretation_INVALID_CUSTOM_COMMAND,

	/*
	 * Used when the custom command layout interpreter returns an
	 * unrecognized value. Will cause the layout resolver to return the
	 * LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND_INTERPRETATION error.
	 */
	CommandLayoutInterpretation_INVALID_CUSTOM_INTERPRETATION
} CommandLayoutInterpretation;

Vector_ofType(LayoutContentAlignment)
    Vector_ofType(LayoutBlockType)
typedef struct LayoutResolverState {
	CustomCommandLayoutInterpretation(*customCommandHook) (ASTNode *, bool);
	bool caseInsensitiveCommands;
	LayoutBlockTypeVector *blockTypeStack;
	LayoutContentAlignmentVector *contentAlignmentStack;
	LayoutBlockVector *blocks;
	LayoutParagraphVector *paragraphs;
	LayoutLineVector *lines;
	LayoutLineSegmentVector *segments;
	LayoutResolverWarningVector *warnings;
	LayoutBlock *block;
	LayoutParagraph *paragraph;
	LayoutLine *line;
	LayoutLineSegment *segment;
	ASTNode *errorLocation;
} LayoutResolverState;

static void freeLayoutBlocks(LayoutBlockVector * blocks);

static LayoutResolverErrorCode processCommands(LayoutResolverState * state,
					       ASTNode * parent,
					       ASTNodePointerVector * nodes);

static LayoutResolverErrorCode
processCommandStart(LayoutResolverState * state, ASTNode * node,
		    CommandLayoutInterpretation layout);

static LayoutResolverErrorCode
processCommandEnd(LayoutResolverState * state, ASTNode * node,
		  CommandLayoutInterpretation layout);

static LayoutResolverErrorCode addSegmentContent(LayoutResolverState * state,
						 ASTNode * nodePointer);

static LayoutResolverErrorCode newBlock(LayoutResolverState * state,
					ASTNode * node,
					CommandLayoutInterpretation layout,
					bool isCommandStart);

static LayoutResolverErrorCode newParagraph(LayoutResolverState * state,
					    ASTNode * node,
					    CommandLayoutInterpretation layout);

static LayoutResolverErrorCode newLine(LayoutResolverState * state,
				       ASTNode * node);

static LayoutResolverErrorCode newSegment(LayoutResolverState * state,
					  ASTNode * node);

static LayoutResolverErrorCode updateSegmentOnCommandStart(LayoutResolverState *
							   state,
							   ASTNode * command);

static LayoutResolverErrorCode updateSegmentOnCommandEnd(LayoutResolverState *
							 state,
							 ASTNode * commandNode);

static bool nodeHasParentOfType(ASTNode * node, string * type,
				bool caseInsensitive);

static CommandLayoutInterpretation
getCommandLayoutInterpretation(ASTNode * command,
			       CustomCommandLayoutInterpretation
			       customCommandHook(ASTNode *, bool),
			       bool caseInsensitive);

static CommandLayoutInterpretation getLayoutInterpretation(LayoutResolverState *
							   state,
							   ASTNode *
							   commandNode);

static CommandLayoutInterpretation
getStandardCommandLayoutInterpretation(string * command, bool caseInsensitive);

static bool string_equals(string * string1, string * string2,
			  bool caseInsensitive);

static string *COMMAND_Comment = NULL;	/* The rest is declared bellow */

LayoutResolverResult *resolveLayout(nodes, customCommandHook,
				    caseInsensitiveCommands)
ASTNodePointerVector *nodes;
CustomCommandLayoutInterpretation customCommandHook(ASTNode *, bool);
bool caseInsensitiveCommands;
{
	LayoutResolverState state;
	LayoutResolverResult *result;
	LayoutContentAlignmentVector *contentAlignmentStack = NULL;
	LayoutBlockTypeVector *blockTypeStack = NULL;
	LayoutBlockVector *blocks = NULL;
	LayoutParagraphVector *paragraphs = NULL;
	LayoutLineVector *lines = NULL;
	LayoutLineSegmentVector *segments = NULL;
	LayoutResolverWarningVector *warnings = NULL;
	LayoutBlock block;
	LayoutParagraph paragraph;
	LayoutLine line;
	LayoutLineSegment segment;
	LayoutResolverErrorCode errorCode = LayoutResolverErrorCode_OK;

	result = malloc(sizeof(LayoutResolverResult));
	if (result == NULL) {
		return NULL;
	}
	result->type = LayoutResolverResultType_SUCCESS;

	block.causingCommand = NULL;
	block.paragraphs = NULL;
	block.type = LayoutBlockType_MAIN_CONTENT;

	paragraph.causingCommand = NULL;
	paragraph.lines = NULL;
	paragraph.type = LayoutParagraphType_IMPLICIT;

	line.causingCommand = NULL;
	line.segments = NULL;

	segment.causingCommand = NULL;
	segment.contentAlignment = LayoutContentAlignment_DEFAULT;
	segment.leftIndentationLevel = 0;
	segment.rightIndentationLevel = 0;
	segment.fontSizeChange = 0;
	segment.fontBoldLevel = 0;
	segment.fontItalicLevel = 0;
	segment.fontUnderlinedLevel = 0;
	segment.fontFixedLevel = 0;
	segment.otherSegmentMarkers = NULL;
	segment.content = NULL;

	do {
		warnings = LayoutResolverWarningVector_new(0, 0);
		if (warnings == NULL) {
			errorCode =
			    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_WARNINGS;
			break;
		}

		if (nodes == NULL) {
			errorCode = LayoutResolverErrorCode_NULL_NODES_PROVIDED;
			break;
		}

		blocks = LayoutBlockVector_new(0, 0);
		result->result.blocks = blocks;
		if (blocks == NULL) {
			errorCode =
			    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_BLOCKS;
			break;
		}

		paragraphs = LayoutParagraphVector_new(0, 0);
		errorCode =
		    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_PARAGRAPHS;
		if (paragraphs == NULL) {
			break;
		}
		errorCode = LayoutResolverErrorCode_OK;

		lines = LayoutLineVector_new(0, 0);
		if (lines == NULL) {
			errorCode =
			    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINES;
			break;
		}

		segments = LayoutLineSegmentVector_new(0, 0);
		errorCode =
		    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINE_SEGMENTS;
		if (segments == NULL) {
			break;
		}
		contentAlignmentStack = LayoutContentAlignmentVector_new(0, 0);
		if (contentAlignmentStack == NULL) {
			break;
		}
		errorCode = LayoutResolverErrorCode_OK;

		blockTypeStack = LayoutBlockTypeVector_new(0, 0);
		if (blockTypeStack == NULL) {
			errorCode =
			    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_BLOCKS;
			break;
		}

		block.paragraphs = LayoutParagraphVector_new(0, 0);
		if (block.paragraphs == NULL) {
			errorCode =
			    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_BLOCKS;
			break;
		}

		paragraph.lines = LayoutLineVector_new(0, 0);
		errorCode =
		    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_PARAGRAPHS;
		if (paragraph.lines == NULL) {
			break;
		}
		errorCode = LayoutResolverErrorCode_OK;

		line.segments = LayoutLineSegmentVector_new(0, 0);
		if (line.segments == NULL) {
			errorCode =
			    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINES;
			break;
		}

		segment.contentAlignment = LayoutContentAlignment_DEFAULT;
		segment.leftIndentationLevel = 0;
		segment.rightIndentationLevel = 0;
		segment.fontSizeChange = 0;
		segment.fontBoldLevel = 0;
		segment.fontItalicLevel = 0;
		segment.fontUnderlinedLevel = 0;
		segment.fontFixedLevel = 0;
		segment.otherSegmentMarkers = ASTNodePointerVector_new(0, 0);
		errorCode =
		    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINE_SEGMENTS;
		if (segment.otherSegmentMarkers == NULL) {
			break;
		}
		segment.content = ASTNodePointerVector_new(0, 0);
		if (segment.content == NULL) {
			break;
		}
		errorCode = LayoutResolverErrorCode_OK;
	} while (false);

	if (errorCode != LayoutResolverErrorCode_OK) {
		result->type = LayoutResolverResultType_ERROR;
		result->result.error.code = errorCode;
		result->result.error.location = NULL;
		result->warnings = warnings;
		LayoutBlockTypeVector_free(blockTypeStack);
		LayoutContentAlignmentVector_free(contentAlignmentStack);
		freeLayoutBlocks(blocks);
		LayoutParagraphVector_free(paragraphs);
		LayoutLineVector_free(lines);
		LayoutLineSegmentVector_free(segments);
		LayoutParagraphVector_free(block.paragraphs);
		LayoutLineVector_free(paragraph.lines);
		LayoutLineSegmentVector_free(line.segments);
		ASTNodePointerVector_free(segment.otherSegmentMarkers);
		ASTNodePointerVector_free(segment.content);
		return result;
	}

	state.customCommandHook = customCommandHook;
	state.caseInsensitiveCommands = caseInsensitiveCommands;
	state.blockTypeStack = blockTypeStack;
	state.contentAlignmentStack = contentAlignmentStack;
	state.blocks = blocks;
	state.paragraphs = paragraphs;
	state.lines = lines;
	state.segments = segments;
	state.warnings = warnings;
	state.block = &block;
	state.paragraph = &paragraph;
	state.line = &line;
	state.segment = &segment;
	state.errorLocation = NULL;

	errorCode = processCommands(&state, NULL, nodes);

	if (errorCode == LayoutResolverErrorCode_OK) {
		/* Add the current remaining content to completed blocks */
		ASTNode node;
		node.byteIndex = 0;
		node.codepointIndex = 0;
		node.tokenIndex = 0;
		node.parent = NULL;
		node.type = ASTNodeType_COMMAND;
		node.value = COMMAND_Comment;
		node.children = NULL;
		newBlock(&state, &node, CommandLayoutInterpretation_COMMENT,
			 true);
		result->result.blocks = state.blocks;
	} else {
		result->type = LayoutResolverResultType_ERROR;
		result->result.error.code = errorCode;
		result->result.error.location = state.errorLocation;
		freeLayoutBlocks(state.blocks);
	}
	result->warnings = state.warnings;

	LayoutBlockTypeVector_free(blockTypeStack);
	LayoutContentAlignmentVector_free(contentAlignmentStack);
	LayoutParagraphVector_free(paragraphs);
	LayoutLineVector_free(lines);
	LayoutLineSegmentVector_free(segments);
	LayoutParagraphVector_free(state.block->paragraphs);
	LayoutLineVector_free(state.paragraph->lines);
	LayoutLineSegmentVector_free(state.line->segments);
	ASTNodePointerVector_free(state.segment->otherSegmentMarkers);
	ASTNodePointerVector_free(state.segment->content);

	return result;
}

void LayoutResolverResult_free(result)
LayoutResolverResult *result;
{
	if (result == NULL) {
		return;
	}

	switch (result->type) {
	case LayoutResolverResultType_SUCCESS:
		freeLayoutBlocks(result->result.blocks);
		break;
	case LayoutResolverResultType_ERROR:
		break;
	default:
		break;
	}

	LayoutResolverWarningVector_free(result->warnings);
	free(result);
}

static void freeLayoutBlocks(blocks)
LayoutBlockVector *blocks;
{
	LayoutBlock *block;
	LayoutParagraph *paragraph;
	LayoutLine *line;
	LayoutLineSegment *segment;
	ASTNodePointerVector *nodes;
	unsigned long blockIndex, paragraphIndex, lineIndex, segmentIndex;

	if (blocks == NULL) {
		return;
	}

	for (blockIndex = 0, block = blocks->items;
	     blockIndex < blocks->size.length && block != NULL;
	     blockIndex++, block++) {
		for (paragraphIndex = 0, paragraph = block->paragraphs->items;
		     paragraphIndex < block->paragraphs->size.length &&
		     paragraph != NULL; paragraphIndex++, paragraph++) {
			for (lineIndex = 0, line = paragraph->lines->items;
			     lineIndex < paragraph->lines->size.length &&
			     line != NULL; lineIndex++, line++) {
				for (segmentIndex = 0,
				     segment = line->segments->items;
				     segmentIndex < line->segments->size.length
				     && segment != NULL;
				     segmentIndex++, segment++) {
					nodes = segment->otherSegmentMarkers;
					ASTNodePointerVector_free(nodes);
					nodes = segment->content;
					ASTNodePointerVector_free(nodes);
				}
				LayoutLineSegmentVector_free(line->segments);
			}
			LayoutLineVector_free(paragraph->lines);
		}
		LayoutParagraphVector_free(block->paragraphs);
	}
	LayoutBlockVector_free(blocks);
}

static string *COMMAND_lt = NULL;
static string *COMMAND_Bold = NULL;
static string *COMMAND_Italic = NULL;
static string *COMMAND_Fixed = NULL;
static string *COMMAND_Smaller = NULL;
static string *COMMAND_Bigger = NULL;
static string *COMMAND_Underline = NULL;
static string *COMMAND_Subscript = NULL;
static string *COMMAND_Superscript = NULL;
static string *COMMAND_Center = NULL;
static string *COMMAND_FlushLeft = NULL;
static string *COMMAND_FlushRight = NULL;
static string *COMMAND_Indent = NULL;
static string *COMMAND_IndentRight = NULL;
static string *COMMAND_Outdent = NULL;
static string *COMMAND_OutdentRight = NULL;
static string *COMMAND_Excerpt = NULL;
static string *COMMAND_Signature = NULL;
static string *COMMAND_Paragraph = NULL;
static string *COMMAND_SamePage = NULL;
static string *COMMAND_Heading = NULL;
static string *COMMAND_Footing = NULL;
static string *COMMAND_ISO_8859_1 = NULL;
static string *COMMAND_ISO_8859_2 = NULL;
static string *COMMAND_ISO_8859_3 = NULL;
static string *COMMAND_ISO_8859_4 = NULL;
static string *COMMAND_ISO_8859_5 = NULL;
static string *COMMAND_ISO_8859_6 = NULL;
static string *COMMAND_ISO_8859_7 = NULL;
static string *COMMAND_ISO_8859_8 = NULL;
static string *COMMAND_ISO_8859_9 = NULL;
static string *COMMAND_US_ASCII = NULL;
static string *COMMAND_No_op = NULL;
static string *COMMAND_nl = NULL;
static string *COMMAND_np = NULL;

static LayoutResolverErrorCode processCommands(state, parent, nodes)
LayoutResolverState *state;
ASTNode *parent;
ASTNodePointerVector *nodes;
{
	ASTNode **nodePointer;
	ASTNode *node;
	unsigned long index = 0;
	CommandLayoutInterpretation layout;
	LayoutResolverErrorCode errorCode = LayoutResolverErrorCode_OK;

	if (nodes == NULL) {
		return LayoutResolverErrorCode_NULL_NODES_PROVIDED;
	}

	nodePointer = nodes->items;
	for (; index < nodes->size.length; nodePointer++, index++) {
		node = *nodePointer;
		if (node == NULL) {
			errorCode = LayoutResolverErrorCode_NULL_NODES_PROVIDED;
			if (state->errorLocation == NULL) {
				/*
				 * We set the location to the parentNode, since
				 * NULL would not be very helpful
				 */
				state->errorLocation = parent;
			}
			break;
		}

		switch (node->type) {
		case ASTNodeType_COMMAND:
			layout = getLayoutInterpretation(state, node);

			errorCode = processCommandStart(state, node, layout);
			if (errorCode != LayoutResolverErrorCode_OK) {
				break;
			}

			if (layout != CommandLayoutInterpretation_COMMENT
			    && node->children != NULL) {
				errorCode =
				    processCommands(state, node,
						    node->children);
			}
			if (errorCode != LayoutResolverErrorCode_OK) {
				break;
			}

			errorCode = processCommandEnd(state, node, layout);
			break;

		case ASTNodeType_TEXT:
		case ASTNodeType_WHITESPACE:
			errorCode = addSegmentContent(state, *nodePointer);
			break;

		default:
			errorCode =
			    LayoutResolverErrorCode_UNSUPPORTED_NODE_TYPE;
			break;
		}

		if (errorCode != LayoutResolverErrorCode_OK) {
			if (state->errorLocation == NULL) {
				state->errorLocation = node;
			}
			break;
		}
	}

	return errorCode;
}

static LayoutResolverErrorCode processCommandStart(state, node, layout)
LayoutResolverState *state;
ASTNode *node;
CommandLayoutInterpretation layout;
{
	LayoutBlockTypeVector *grownBlockTypeStack;
	LayoutBlockType currentBlockType = state->block->type;
	LayoutResolverErrorCode INVALID_CUSTOM_INTERPRETATION_ERROR =
	    LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND_INTERPRETATION;
	LayoutResolverErrorCode UNTRANSLATED_CUSTOM_LAYOUT_ERROR =
	    LayoutResolverErrorCode_UNTRANSLATED_CUSTOM_LAYOUT_INTERPRETATION;
	LayoutResolverErrorCode OOM_FOR_WARNINGS =
	    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_WARNINGS;
	LayoutResolverErrorCode errorCode = LayoutResolverErrorCode_OK;

	switch (layout) {
	case CommandLayoutInterpretation_HEADING_BLOCK:
	case CommandLayoutInterpretation_FOOTING_BLOCK:
	case CommandLayoutInterpretation_NEW_BLOCK:
		grownBlockTypeStack =
		    LayoutBlockTypeVector_append(state->blockTypeStack,
						 &state->block->type);
		if (grownBlockTypeStack == NULL) {
			return LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_BLOCKS;
		}
		state->blockTypeStack = grownBlockTypeStack;

		newBlock(state, node, layout, true);
		break;

	case CommandLayoutInterpretation_NEW_PAGE:
	case CommandLayoutInterpretation_SAME_PAGE:
		if (layout == CommandLayoutInterpretation_NEW_PAGE
		    && nodeHasParentOfType(node, COMMAND_SamePage,
					   state->caseInsensitiveCommands)) {
			LayoutResolverWarning warning;
			LayoutResolverWarningVector *grownWarnings;
			warning.cause = node;
			warning.code =
			    LayoutResolverWarningCode_NEW_PAGE_INSIDE_SAME_PAGE;
			grownWarnings =
			    LayoutResolverWarningVector_append(state->warnings,
							       &warning);
			if (grownWarnings == NULL) {
				errorCode = OOM_FOR_WARNINGS;
				break;
			}
			state->warnings = grownWarnings;
		}

		if (layout == CommandLayoutInterpretation_SAME_PAGE
		    && nodeHasParentOfType(node, COMMAND_SamePage,
					   state->caseInsensitiveCommands)) {
			LayoutResolverWarning warning;
			LayoutResolverWarningVector *grownWarnings;
			warning.cause = node;
			warning.code =
			    LayoutResolverWarningCode_NESTED_SAME_PAGE;
			grownWarnings =
			    LayoutResolverWarningVector_append(state->warnings,
							       &warning);
			if (grownWarnings == NULL) {
				errorCode = OOM_FOR_WARNINGS;
				break;
			}
			state->warnings = grownWarnings;
		}

		errorCode = newBlock(state, node, layout, true);
		if (errorCode != LayoutResolverErrorCode_OK) {
			break;
		}

		errorCode = newBlock(state, node, layout, true);
		if (errorCode != LayoutResolverErrorCode_OK) {
			break;
		}

		state->block->type = currentBlockType;
		break;

	case CommandLayoutInterpretation_NEW_PARAGRAPH:
	case CommandLayoutInterpretation_NEW_ISOLATED_PARAGRAPH:
		errorCode = newParagraph(state, node, layout);
		break;

	case CommandLayoutInterpretation_NEW_LINE:
	case CommandLayoutInterpretation_NEW_ISOLATED_LINE:
		errorCode = newLine(state, node);
		break;

	case CommandLayoutInterpretation_NEW_LINE_SEGMENT:
		errorCode = newSegment(state, node);
		if (errorCode != LayoutResolverErrorCode_OK) {
			break;
		}

		errorCode = updateSegmentOnCommandStart(state, node);
		break;

	case CommandLayoutInterpretation_INLINE_CONTENT:
	case CommandLayoutInterpretation_COMMENT:
		errorCode = addSegmentContent(state, node);
		break;

	case CommandLayoutInterpretation_NO_OP:
		break;

	case CommandLayoutInterpretation_INVALID_CUSTOM_COMMAND:
		return LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND;

	case CommandLayoutInterpretation_INVALID_CUSTOM_INTERPRETATION:
		return INVALID_CUSTOM_INTERPRETATION_ERROR;

	case CommandLayoutInterpretation_CUSTOM:
		/* This should never happen */
		return UNTRANSLATED_CUSTOM_LAYOUT_ERROR;

	default:
		return
		    LayoutResolverErrorCode_UNSUPPORTED_LAYOUT_INTERPRETATION;
	}

	return errorCode;
}

static LayoutResolverErrorCode processCommandEnd(state, node, layout)
LayoutResolverState *state;
ASTNode *node;
CommandLayoutInterpretation layout;
{
	LayoutBlockType currentBlockType = state->block->type;
	LayoutBlockTypeVector *reducedTypes = NULL;
	LayoutBlockType poppedType;
	LayoutResolverErrorCode INVALID_CUSTOM_INTERPRETATION_ERROR =
	    LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND_INTERPRETATION;
	LayoutResolverErrorCode UNTRANSLATED_CUSTOM_LAYOUT_ERROR =
	    LayoutResolverErrorCode_UNTRANSLATED_CUSTOM_LAYOUT_INTERPRETATION;
	LayoutResolverErrorCode errorCode = LayoutResolverErrorCode_OK;

	switch (layout) {
	case CommandLayoutInterpretation_HEADING_BLOCK:
	case CommandLayoutInterpretation_FOOTING_BLOCK:
	case CommandLayoutInterpretation_NEW_BLOCK:
		reducedTypes =
		    LayoutBlockTypeVector_pop(state->blockTypeStack,
					      &poppedType);
		if (reducedTypes == NULL) {
			errorCode =
			    LayoutResolverErrorCode_BLOCK_TYPE_STACK_UNDERFLOW;
			break;
		}
		state->blockTypeStack = reducedTypes;

		errorCode = newBlock(state, node, layout, false);
		if (errorCode == LayoutResolverErrorCode_OK) {
			break;
		}

		state->block->type = poppedType;

		break;

	case CommandLayoutInterpretation_SAME_PAGE:
		errorCode = newBlock(state, node, layout, false);
		if (errorCode != LayoutResolverErrorCode_OK) {
			break;
		}

		errorCode = newBlock(state, node, layout, false);
		if (errorCode != LayoutResolverErrorCode_OK) {
			break;
		}

		state->block->type = currentBlockType;
		break;

	case CommandLayoutInterpretation_NEW_ISOLATED_PARAGRAPH:
		errorCode = newParagraph(state, node, layout);
		break;

	case CommandLayoutInterpretation_NEW_ISOLATED_LINE:
		errorCode = newLine(state, node);
		break;

	case CommandLayoutInterpretation_NEW_LINE_SEGMENT:
		errorCode = newSegment(state, node);
		if (errorCode != LayoutResolverErrorCode_OK) {
			break;
		}

		errorCode = updateSegmentOnCommandEnd(state, node);
		if (errorCode != LayoutResolverErrorCode_OK) {
			break;
		}
		break;

	case CommandLayoutInterpretation_NEW_PAGE:
	case CommandLayoutInterpretation_NEW_PARAGRAPH:
	case CommandLayoutInterpretation_NEW_LINE:
	case CommandLayoutInterpretation_INLINE_CONTENT:
	case CommandLayoutInterpretation_COMMENT:
	case CommandLayoutInterpretation_NO_OP:
		/* Nothing to do */
		break;

	case CommandLayoutInterpretation_INVALID_CUSTOM_COMMAND:
		return LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND;

	case CommandLayoutInterpretation_INVALID_CUSTOM_INTERPRETATION:
		return INVALID_CUSTOM_INTERPRETATION_ERROR;

	case CommandLayoutInterpretation_CUSTOM:
		/* This should never happen */
		return UNTRANSLATED_CUSTOM_LAYOUT_ERROR;
	default:
		return
		    LayoutResolverErrorCode_UNSUPPORTED_LAYOUT_INTERPRETATION;
	}

	return errorCode;
}

static LayoutResolverErrorCode newBlock(state, node, layout, isCommandStart)
LayoutResolverState *state;
ASTNode *node;
CommandLayoutInterpretation layout;
bool isCommandStart;
{
	LayoutResolverErrorCode errorCode = LayoutResolverErrorCode_OK;
	LayoutParagraphVector *grownParagraphs = NULL;
	LayoutBlockVector *grownBlocks = NULL;
	LayoutResolverErrorCode OOM_FOR_PARAGRAPHS_ERROR =
	    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_PARAGRAPHS;
	errorCode = newParagraph(state, node, layout);
	if (errorCode != LayoutResolverErrorCode_OK) {
		return errorCode;
	}

	if (state->paragraphs->size.length > 0) {
		grownParagraphs =
		    LayoutParagraphVector_concat(state->block->paragraphs,
						 state->paragraphs);
		if (grownParagraphs == NULL) {
			return OOM_FOR_PARAGRAPHS_ERROR;
		}
		LayoutParagraphVector_free(state->block->paragraphs);
		state->block->paragraphs = grownParagraphs;

		LayoutParagraphVector_free(state->paragraphs);
		state->paragraphs = LayoutParagraphVector_new(0, 0);
		if (state->paragraphs == NULL) {
			return OOM_FOR_PARAGRAPHS_ERROR;
		}
	}

	if (state->block->paragraphs->size.length > 0
	    || state->block->type == LayoutBlockType_PAGE_BREAK
	    || state->block->type == LayoutBlockType_SAME_PAGE_START
	    || state->block->type == LayoutBlockType_SAME_PAGE_END) {
		grownBlocks =
		    LayoutBlockVector_append(state->blocks, state->block);
		if (grownBlocks == NULL) {
			return LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_BLOCKS;
		}
		state->blocks = grownBlocks;

		state->block->paragraphs = LayoutParagraphVector_new(0, 0);
		if (state->block->paragraphs == NULL) {
			return OOM_FOR_PARAGRAPHS_ERROR;
		}
	}

	state->block->causingCommand = node;

	switch (layout) {
	case CommandLayoutInterpretation_HEADING_BLOCK:
		state->block->type = LayoutBlockType_HEADING;
		break;

	case CommandLayoutInterpretation_FOOTING_BLOCK:
		state->block->type = LayoutBlockType_FOOTING;
		break;

	case CommandLayoutInterpretation_NEW_BLOCK:
		state->block->type = LayoutBlockType_CUSTOM;
		break;

	case CommandLayoutInterpretation_NEW_PAGE:
		state->block->type = LayoutBlockType_PAGE_BREAK;
		break;

	case CommandLayoutInterpretation_SAME_PAGE:
		if (isCommandStart) {
			state->block->type = LayoutBlockType_SAME_PAGE_START;
		} else {
			state->block->type = LayoutBlockType_SAME_PAGE_END;
		}
		break;

	default:
		/*
		 * Nothing to do, and this will most likely never happen, but
		 * even if it did, it is not really an issue.
		 */
		break;
	}

	return LayoutResolverErrorCode_OK;
}

static LayoutResolverErrorCode newParagraph(state, node, layout)
LayoutResolverState *state;
ASTNode *node;
CommandLayoutInterpretation layout;
{
	LayoutResolverErrorCode errorCode = LayoutResolverErrorCode_OK;
	LayoutLineVector *grownLines = NULL;
	LayoutParagraphVector *grownParagraphs = NULL;
	LayoutResolverErrorCode OOM_FOR_PARAGRAPHS_ERROR =
	    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_PARAGRAPHS;
	errorCode = newLine(state, node);
	if (errorCode != LayoutResolverErrorCode_OK) {
		return errorCode;
	}

	if (state->lines->size.length > 0) {
		grownLines =
		    LayoutLineVector_concat(state->paragraph->lines,
					    state->lines);
		if (grownLines == NULL) {
			return LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINES;
		}
		LayoutLineVector_free(state->paragraph->lines);
		state->paragraph->lines = grownLines;

		LayoutLineVector_free(state->lines);
		state->lines = LayoutLineVector_new(0, 0);
		if (state->lines == NULL) {
			return LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINES;
		}
	}

	if (state->paragraph->lines->size.length > 0) {
		grownParagraphs =
		    LayoutParagraphVector_append(state->paragraphs,
						 state->paragraph);
		if (grownParagraphs == NULL) {
			return OOM_FOR_PARAGRAPHS_ERROR;
		}
		state->paragraphs = grownParagraphs;

		state->paragraph->lines = LayoutLineVector_new(0, 0);
		if (state->paragraph->lines == NULL) {
			return LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINES;
		}
	}

	state->paragraph->causingCommand = node;
	state->paragraph->type =
	    layout == CommandLayoutInterpretation_NEW_ISOLATED_PARAGRAPH ?
	    LayoutParagraphType_EXPLICIT : LayoutParagraphType_IMPLICIT;

	return LayoutResolverErrorCode_OK;
}

static LayoutResolverErrorCode newLine(state, node)
LayoutResolverState *state;
ASTNode *node;
{
	LayoutResolverErrorCode errorCode = LayoutResolverErrorCode_OK;
	LayoutLineSegmentVector *grownSegments = NULL;
	LayoutLineVector *grownLines = NULL;
	LayoutResolverErrorCode OOM_FOR_LINE_SEGMENTS_ERROR =
	    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINE_SEGMENTS;
	errorCode = newSegment(state, node);
	if (errorCode != LayoutResolverErrorCode_OK) {
		return errorCode;
	}

	if (state->segments->size.length > 0) {
		grownSegments =
		    LayoutLineSegmentVector_concat(state->line->segments,
						   state->segments);
		if (grownSegments == NULL) {
			return OOM_FOR_LINE_SEGMENTS_ERROR;
		}
		LayoutLineSegmentVector_free(state->line->segments);
		state->line->segments = grownSegments;

		LayoutLineSegmentVector_free(state->segments);
		state->segments = LayoutLineSegmentVector_new(0, 0);
		if (state->segments == NULL) {
			return OOM_FOR_LINE_SEGMENTS_ERROR;
		}
	}

	if (state->line->segments->size.length > 0) {
		grownLines = LayoutLineVector_append(state->lines, state->line);
		if (grownLines == NULL) {
			return LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINES;
		}
		state->lines = grownLines;

		state->line->segments = LayoutLineSegmentVector_new(0, 0);
		if (state->line->segments == NULL) {
			return OOM_FOR_LINE_SEGMENTS_ERROR;
		}
	}

	state->line->causingCommand = node;

	return LayoutResolverErrorCode_OK;
}

static LayoutResolverErrorCode newSegment(state, node)
LayoutResolverState *state;
ASTNode *node;
{
	LayoutLineSegment *segment = state->segment;
	LayoutLineSegmentVector *grownSegments = NULL;
	LayoutResolverErrorCode OOM_FOR_SEGMENTS_ERROR =
	    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINE_SEGMENTS;
	unsigned long length;

	if (segment->content->size.length > 0) {
		grownSegments =
		    LayoutLineSegmentVector_append(state->segments, segment);
		if (grownSegments == NULL) {
			return OOM_FOR_SEGMENTS_ERROR;
		}
		state->segments = grownSegments;

		segment->content = ASTNodePointerVector_new(0, 0);
		if (segment->content == NULL) {
			return OOM_FOR_SEGMENTS_ERROR;
		}
		length = segment->otherSegmentMarkers->size.length;
		segment->otherSegmentMarkers =
		    ASTNodePointerVector_bigSlice(segment->otherSegmentMarkers,
						  0, length);
		if (segment->otherSegmentMarkers == NULL) {
			return OOM_FOR_SEGMENTS_ERROR;
		}
	}

	segment->causingCommand = node;

	return LayoutResolverErrorCode_OK;
}

static LayoutResolverErrorCode addSegmentContent(state, node)
LayoutResolverState *state;
ASTNode *node;
{
	ASTNodePointerVector *grownContent;
	grownContent =
	    ASTNodePointerVector_append(state->segment->content, &node);
	if (grownContent == NULL) {
		return LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINE_SEGMENTS;
	}
	state->segment->content = grownContent;
	return LayoutResolverErrorCode_OK;
}

static LayoutResolverErrorCode updateSegmentOnCommandStart(state, commandNode)
LayoutResolverState *state;
ASTNode *commandNode;
{
	LayoutContentAlignmentVector *contentAlignmentStack;
	LayoutContentAlignmentVector *grownAlignments;
	LayoutLineSegment *segment = state->segment;
	LayoutContentAlignment alignment = segment->contentAlignment;
	ASTNodePointerVector *grownMarkers;
	string *command = commandNode->value;
	bool caseInsensitive = state->caseInsensitiveCommands;
	unsigned long length;
	LayoutResolverErrorCode OOM_FOR_LINE_SEGMENTS =
	    LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINE_SEGMENTS;
	LayoutResolverErrorCode LEFT_INDENTATION_OVERFLOW_ERROR =
	    LayoutResolverErrorCode_LEFT_INDENTATION_LEVEL_OVERFLOW;
	LayoutResolverErrorCode LEFT_INDENTATION_UNDERFLOW_ERROR =
	    LayoutResolverErrorCode_LEFT_INDENTATION_LEVEL_UNDERFLOW;
	LayoutResolverErrorCode RIGHT_INDENTATION_OVERFLOW_ERROR =
	    LayoutResolverErrorCode_RIGHT_INDENTATION_LEVEL_OVERFLOW;
	LayoutResolverErrorCode RIGHT_INDENTATION_UNDERFLOW_ERROR =
	    LayoutResolverErrorCode_RIGHT_INDENTATION_LEVEL_UNDERFLOW;
	LayoutResolverErrorCode FONT_UNDERLINED_LEVEL_OVERFLOW_ERROR =
	    LayoutResolverErrorCode_FONT_UNDERLINED_LEVEL_OVERFLOW;

	contentAlignmentStack = state->contentAlignmentStack;

	/* contentAlignment */
	if (string_equals(command, COMMAND_FlushRight, caseInsensitive)) {
		grownAlignments =
		    LayoutContentAlignmentVector_append(contentAlignmentStack,
							&alignment);
		if (grownAlignments == NULL) {
			return OOM_FOR_LINE_SEGMENTS;
		}
		state->contentAlignmentStack = grownAlignments;
		segment->contentAlignment =
		    LayoutContentAlignment_JUSTIFY_RIGHT;
		return LayoutResolverErrorCode_OK;
	} else if (string_equals(command, COMMAND_FlushLeft, caseInsensitive)) {
		grownAlignments =
		    LayoutContentAlignmentVector_append(contentAlignmentStack,
							&alignment);
		if (grownAlignments == NULL) {
			return OOM_FOR_LINE_SEGMENTS;
		}
		state->contentAlignmentStack = grownAlignments;
		segment->contentAlignment = LayoutContentAlignment_JUSTIFY_LEFT;
		return LayoutResolverErrorCode_OK;
	} else if (string_equals(command, COMMAND_Center, caseInsensitive)) {
		grownAlignments =
		    LayoutContentAlignmentVector_append(contentAlignmentStack,
							&alignment);
		if (grownAlignments == NULL) {
			return OOM_FOR_LINE_SEGMENTS;
		}
		state->contentAlignmentStack = grownAlignments;
		segment->contentAlignment = LayoutContentAlignment_CENTER;
		return LayoutResolverErrorCode_OK;
	}

	/* leftIndentationLevel */
	if (string_equals(command, COMMAND_Indent, caseInsensitive)) {
		if (segment->leftIndentationLevel == SHRT_MAX) {
			return LEFT_INDENTATION_OVERFLOW_ERROR;
		}
		segment->leftIndentationLevel++;
		return LayoutResolverErrorCode_OK;
	} else if (string_equals(command, COMMAND_Outdent, caseInsensitive)) {
		if (segment->leftIndentationLevel == SHRT_MIN) {
			return LEFT_INDENTATION_UNDERFLOW_ERROR;
		}
		segment->leftIndentationLevel--;
		return LayoutResolverErrorCode_OK;
	}

	/* rightIndentationLevel */
	if (string_equals(command, COMMAND_IndentRight, caseInsensitive)) {
		if (segment->rightIndentationLevel == SHRT_MAX) {
			return RIGHT_INDENTATION_OVERFLOW_ERROR;
		}
		segment->rightIndentationLevel++;
		return LayoutResolverErrorCode_OK;
	} else
	    if (string_equals(command, COMMAND_OutdentRight, caseInsensitive)) {
		if (segment->rightIndentationLevel == SHRT_MIN) {
			return RIGHT_INDENTATION_UNDERFLOW_ERROR;
		}
		segment->rightIndentationLevel--;
		return LayoutResolverErrorCode_OK;
	}

	/* fontSizeChange */
	if (string_equals(command, COMMAND_Bigger, caseInsensitive)) {
		if (segment->fontSizeChange == SHRT_MAX) {
			return
			    LayoutResolverErrorCode_FONT_SIZE_CHANGE_OVERFLOW;
		}
		segment->fontSizeChange++;
		return LayoutResolverErrorCode_OK;
	} else if (string_equals(command, COMMAND_Smaller, caseInsensitive)) {
		if (segment->fontSizeChange == SHRT_MIN) {
			return
			    LayoutResolverErrorCode_FONT_SIZE_CHANGE_UNDERFLOW;
		}
		segment->fontSizeChange--;
		return LayoutResolverErrorCode_OK;
	}

	/* fontBoldLevel */
	if (string_equals(command, COMMAND_Bold, caseInsensitive)) {
		if (segment->fontBoldLevel == USHRT_MAX) {
			return LayoutResolverErrorCode_FONT_BOLD_LEVEL_OVERFLOW;
		}
		segment->fontBoldLevel++;
		return LayoutResolverErrorCode_OK;
	}

	/* fontItalicLevel */
	if (string_equals(command, COMMAND_Italic, caseInsensitive)) {
		if (segment->fontItalicLevel == USHRT_MAX) {
			return
			    LayoutResolverErrorCode_FONT_ITALIC_LEVEL_OVERFLOW;
		}
		segment->fontItalicLevel++;
		return LayoutResolverErrorCode_OK;
	}

	/* fontUnderlinedLevel */
	if (string_equals(command, COMMAND_Underline, caseInsensitive)) {
		if (segment->fontUnderlinedLevel == USHRT_MAX) {
			return FONT_UNDERLINED_LEVEL_OVERFLOW_ERROR;
		}
		segment->fontUnderlinedLevel++;
		return LayoutResolverErrorCode_OK;
	}

	/* fontFixedLevel */
	if (string_equals(command, COMMAND_Fixed, caseInsensitive)) {
		if (segment->fontFixedLevel == USHRT_MAX) {
			return
			    LayoutResolverErrorCode_FONT_FIXED_LEVEL_OVERFLOW;
		}
		segment->fontFixedLevel++;
		return LayoutResolverErrorCode_OK;
	}

	/* otherSegmentMarkers */
	length = segment->otherSegmentMarkers->size.length;
	grownMarkers =
	    ASTNodePointerVector_bigSlice(segment->otherSegmentMarkers, 0,
					  length);
	if (grownMarkers == NULL) {
		return OOM_FOR_LINE_SEGMENTS;
	}
	segment->otherSegmentMarkers = grownMarkers;
	grownMarkers =
	    ASTNodePointerVector_append(segment->otherSegmentMarkers,
					&commandNode);
	if (grownMarkers == NULL) {
		return OOM_FOR_LINE_SEGMENTS;
	}
	segment->otherSegmentMarkers = grownMarkers;

	return LayoutResolverErrorCode_OK;
}

static LayoutResolverErrorCode updateSegmentOnCommandEnd(state, commandNode)
LayoutResolverState *state;
ASTNode *commandNode;
{
	string *command = commandNode->value;
	bool caseInsensitive = state->caseInsensitiveCommands;
	ASTNodePointerVector *reducedMarkers = NULL;
	ASTNode *marker = NULL;
	unsigned long length;
	LayoutResolverErrorCode SEGMENT_MARKER_STACK_UNDERFLOW_ERROR =
	    LayoutResolverErrorCode_OTHER_SEGMENT_MARKER_STACK_UNDERFLOW;
	LayoutResolverErrorCode SEGMENT_MARKER_STACK_INCONSISTENCY_ERROR =
	    LayoutResolverErrorCode_OTHER_SEGMENT_MARKER_STACK_INCONSISTENCY;

	/* contentAlignment */
	if (string_equals(command, COMMAND_FlushRight, caseInsensitive)
	    || string_equals(command, COMMAND_FlushLeft, caseInsensitive)
	    || string_equals(command, COMMAND_Center, caseInsensitive)) {
		LayoutContentAlignmentVector *reducedAlignments;
		LayoutContentAlignment poppedAlignment;
		LayoutResolverErrorCode ALIGNMENT_STACK_UNDERFLOW_ERROR =
		    LayoutResolverErrorCode_CONTENT_ALIGNMENT_STACK_UNDERFLOW;
		reducedAlignments =
		    LayoutContentAlignmentVector_pop
		    (state->contentAlignmentStack, &poppedAlignment);
		if (reducedAlignments == NULL) {
			return ALIGNMENT_STACK_UNDERFLOW_ERROR;
		}
		state->contentAlignmentStack = reducedAlignments;
		state->segment->contentAlignment = poppedAlignment;
		return LayoutResolverErrorCode_OK;
	}

	/* leftIndentationLevel */
	if (string_equals(command, COMMAND_Indent, caseInsensitive)) {
		LayoutResolverErrorCode LEFT_INDENTATION_UNDERFLOW_ERROR =
		    LayoutResolverErrorCode_LEFT_INDENTATION_LEVEL_UNDERFLOW;
		if (state->segment->leftIndentationLevel == SHRT_MIN) {
			return LEFT_INDENTATION_UNDERFLOW_ERROR;
		}
		state->segment->leftIndentationLevel--;
		return LayoutResolverErrorCode_OK;
	} else if (string_equals(command, COMMAND_Outdent, caseInsensitive)) {
		LayoutResolverErrorCode LEFT_INDENTATION_OVERFLOW_ERROR =
		    LayoutResolverErrorCode_LEFT_INDENTATION_LEVEL_OVERFLOW;
		if (state->segment->leftIndentationLevel == SHRT_MAX) {
			return LEFT_INDENTATION_OVERFLOW_ERROR;
		}
		state->segment->leftIndentationLevel++;
		return LayoutResolverErrorCode_OK;
	}

	/* rightIndentationLevel */
	if (string_equals(command, COMMAND_IndentRight, caseInsensitive)) {
		LayoutResolverErrorCode RIGHT_INDENTATION_UNDERFLOW_ERROR =
		    LayoutResolverErrorCode_RIGHT_INDENTATION_LEVEL_UNDERFLOW;
		if (state->segment->rightIndentationLevel == SHRT_MIN) {
			return RIGHT_INDENTATION_UNDERFLOW_ERROR;
		}
		state->segment->rightIndentationLevel--;
		return LayoutResolverErrorCode_OK;
	} else
	    if (string_equals(command, COMMAND_OutdentRight, caseInsensitive)) {
		LayoutResolverErrorCode RIGHT_INDENTATION_OVERFLOW =
		    LayoutResolverErrorCode_RIGHT_INDENTATION_LEVEL_OVERFLOW;
		if (state->segment->rightIndentationLevel == SHRT_MAX) {
			return RIGHT_INDENTATION_OVERFLOW;
		}
		state->segment->rightIndentationLevel++;
		return LayoutResolverErrorCode_OK;
	}

	/* fontSizeChange */
	if (string_equals(command, COMMAND_Bigger, caseInsensitive)) {
		if (state->segment->fontSizeChange == SHRT_MIN) {
			return
			    LayoutResolverErrorCode_FONT_SIZE_CHANGE_UNDERFLOW;
		}
		state->segment->fontSizeChange--;
		return LayoutResolverErrorCode_OK;
	} else if (string_equals(command, COMMAND_Smaller, caseInsensitive)) {
		if (state->segment->fontSizeChange == SHRT_MAX) {
			return
			    LayoutResolverErrorCode_FONT_SIZE_CHANGE_OVERFLOW;
		}
		state->segment->fontSizeChange++;
		return LayoutResolverErrorCode_OK;
	}

	/* fontBoldLevel */
	if (string_equals(command, COMMAND_Bold, caseInsensitive)) {
		if (state->segment->fontBoldLevel == 0) {
			return
			    LayoutResolverErrorCode_FONT_BOLD_LEVEL_UNDERFLOW;
		}
		state->segment->fontBoldLevel--;
		return LayoutResolverErrorCode_OK;
	}

	/* fontItalicLevel */
	if (string_equals(command, COMMAND_Italic, caseInsensitive)) {
		if (state->segment->fontItalicLevel == 0) {
			return
			    LayoutResolverErrorCode_FONT_ITALIC_LEVEL_UNDERFLOW;
		}
		state->segment->fontItalicLevel--;
		return LayoutResolverErrorCode_OK;
	}

	/* fontUnderlinedLevel */
	if (string_equals(command, COMMAND_Underline, caseInsensitive)) {
		LayoutResolverErrorCode FONT_UNDERLINED_LEVEL_UNDERFLOW_ERROR =
		    LayoutResolverErrorCode_FONT_UNDERLINED_LEVEL_UNDERFLOW;
		if (state->segment->fontUnderlinedLevel == 0) {
			return FONT_UNDERLINED_LEVEL_UNDERFLOW_ERROR;
		}
		state->segment->fontUnderlinedLevel--;
		return LayoutResolverErrorCode_OK;
	}

	/* fontFixedLevel */
	if (string_equals(command, COMMAND_Fixed, caseInsensitive)) {
		if (state->segment->fontFixedLevel == 0) {
			return
			    LayoutResolverErrorCode_FONT_FIXED_LEVEL_UNDERFLOW;
		}
		state->segment->fontFixedLevel--;
		return LayoutResolverErrorCode_OK;
	}

	/* otherSegmentMarkers */
	length = state->segment->otherSegmentMarkers->size.length;
	reducedMarkers =
	    ASTNodePointerVector_bigSlice(state->segment->otherSegmentMarkers,
					  0, length);
	if (reducedMarkers == NULL) {
		return LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINE_SEGMENTS;
	}
	state->segment->otherSegmentMarkers = reducedMarkers;
	reducedMarkers =
	    ASTNodePointerVector_pop(state->segment->otherSegmentMarkers,
				     &marker);
	if (reducedMarkers == NULL) {
		return SEGMENT_MARKER_STACK_UNDERFLOW_ERROR;
	}
	state->segment->otherSegmentMarkers = reducedMarkers;
	if (!string_equals(marker->value, command, false)) {
		return SEGMENT_MARKER_STACK_INCONSISTENCY_ERROR;
	}

	return LayoutResolverErrorCode_OK;
}

static bool nodeHasParentOfType(node, type, caseInsensitive)
ASTNode *node;
string *type;
bool caseInsensitive;
{
	if (node == NULL || node->parent == NULL) {
		return false;
	}

	if (node->parent->type != ASTNodeType_COMMAND) {
		return false;	/* This should not happen, but, just in case */
	}

	node = node->parent;
	while (!string_equals(node->value, type, caseInsensitive)) {
		node = node->parent;
		if (node == NULL || node->type != ASTNodeType_COMMAND) {
			return false;
		}
	}

	return true;
}

static CommandLayoutInterpretation getLayoutInterpretation(state, commandNode)
LayoutResolverState *state;
ASTNode *commandNode;
{
	return getCommandLayoutInterpretation(commandNode,
					      state->customCommandHook,
					      state->caseInsensitiveCommands);
}

static CommandLayoutInterpretation
getCommandLayoutInterpretation(command, customCommandHook, caseInsensitive)
ASTNode *command;
CustomCommandLayoutInterpretation customCommandHook(ASTNode *, bool);
bool caseInsensitive;
{
	CommandLayoutInterpretation interpretation;
	CustomCommandLayoutInterpretation customInterpretation;

	interpretation =
	    getStandardCommandLayoutInterpretation(command->value,
						   caseInsensitive);
	if (interpretation != CommandLayoutInterpretation_CUSTOM) {
		return interpretation;
	}

	if (customCommandHook == NULL) {
		return CommandLayoutInterpretation_NO_OP;
	}

	customInterpretation = customCommandHook(command, caseInsensitive);
	switch (customInterpretation) {
	case CustomCommandLayoutInterpretation_NEW_BLOCK:
		return CommandLayoutInterpretation_NEW_BLOCK;

	case CustomCommandLayoutInterpretation_NEW_PARAGRAPH:
		return CommandLayoutInterpretation_NEW_PARAGRAPH;

	case CustomCommandLayoutInterpretation_NEW_ISOLATED_PARAGRAPH:
		return CommandLayoutInterpretation_NEW_ISOLATED_PARAGRAPH;

	case CustomCommandLayoutInterpretation_NEW_LINE:
		return CommandLayoutInterpretation_NEW_LINE;

	case CustomCommandLayoutInterpretation_NEW_ISOLATED_LINE:
		return CommandLayoutInterpretation_NEW_ISOLATED_LINE;

	case CustomCommandLayoutInterpretation_NEW_LINE_SEGMENT:
		return CommandLayoutInterpretation_NEW_LINE_SEGMENT;

	case CustomCommandLayoutInterpretation_INLINE_CONTENT:
		return CommandLayoutInterpretation_INLINE_CONTENT;

	case CustomCommandLayoutInterpretation_NO_OP:
		return CommandLayoutInterpretation_NO_OP;

	case CustomCommandLayoutInterpretation_INVALID_COMMAND:
		return CommandLayoutInterpretation_INVALID_CUSTOM_COMMAND;

	default:
		return
		    CommandLayoutInterpretation_INVALID_CUSTOM_INTERPRETATION;
		break;
	}
}

static CommandLayoutInterpretation
getStandardCommandLayoutInterpretation(command, caseInsensitive)
string *command;
bool caseInsensitive;
{
	if (COMMAND_lt == NULL) {
		COMMAND_lt = string_from("lt");
		COMMAND_Bold = string_from("Bold");
		COMMAND_Italic = string_from("Italic");
		COMMAND_Fixed = string_from("Fixed");
		COMMAND_Smaller = string_from("Smaller");
		COMMAND_Bigger = string_from("Bigger");
		COMMAND_Underline = string_from("Underline");
		COMMAND_Subscript = string_from("Subscript");
		COMMAND_Superscript = string_from("Superscript");
		COMMAND_Center = string_from("Center");
		COMMAND_FlushLeft = string_from("FlushLeft");
		COMMAND_FlushRight = string_from("FlushRight");
		COMMAND_Indent = string_from("Indent");
		COMMAND_IndentRight = string_from("IndentRight");
		COMMAND_Outdent = string_from("Outdent");
		COMMAND_OutdentRight = string_from("OutdentRight");
		COMMAND_Excerpt = string_from("Excerpt");
		COMMAND_Signature = string_from("Signature");
		COMMAND_Paragraph = string_from("Paragraph");
		COMMAND_SamePage = string_from("SamePage");
		COMMAND_Heading = string_from("Heading");
		COMMAND_Footing = string_from("Footing");
		COMMAND_ISO_8859_1 = string_from("ISO-8859-1");
		COMMAND_ISO_8859_2 = string_from("ISO-8859-2");
		COMMAND_ISO_8859_3 = string_from("ISO-8859-3");
		COMMAND_ISO_8859_4 = string_from("ISO-8859-4");
		COMMAND_ISO_8859_5 = string_from("ISO-8859-5");
		COMMAND_ISO_8859_6 = string_from("ISO-8859-6");
		COMMAND_ISO_8859_7 = string_from("ISO-8859-7");
		COMMAND_ISO_8859_8 = string_from("ISO-8859-8");
		COMMAND_ISO_8859_9 = string_from("ISO-8859-9");
		COMMAND_US_ASCII = string_from("US-ASCII");
		COMMAND_No_op = string_from("No-op");
		COMMAND_Comment = string_from("Comment");
		COMMAND_nl = string_from("nl");
		COMMAND_np = string_from("np");
	}

	if (string_equals(command, COMMAND_lt, caseInsensitive)
	    || string_equals(command, COMMAND_Subscript, caseInsensitive)
	    || string_equals(command, COMMAND_Superscript, caseInsensitive)
	    || string_equals(command, COMMAND_Excerpt, caseInsensitive)
	    || string_equals(command, COMMAND_Signature, caseInsensitive)) {
		return CommandLayoutInterpretation_INLINE_CONTENT;
	}

	if (string_equals(command, COMMAND_Bold, caseInsensitive)
	    || string_equals(command, COMMAND_Italic, caseInsensitive)
	    || string_equals(command, COMMAND_Underline, caseInsensitive)
	    || string_equals(command, COMMAND_Fixed, caseInsensitive)
	    || string_equals(command, COMMAND_Smaller, caseInsensitive)
	    || string_equals(command, COMMAND_Bigger, caseInsensitive)
	    || string_equals(command, COMMAND_Center, caseInsensitive)
	    || string_equals(command, COMMAND_FlushLeft, caseInsensitive)
	    || string_equals(command, COMMAND_FlushRight, caseInsensitive)
	    || string_equals(command, COMMAND_Indent, caseInsensitive)
	    || string_equals(command, COMMAND_IndentRight, caseInsensitive)
	    || string_equals(command, COMMAND_Outdent, caseInsensitive)
	    || string_equals(command, COMMAND_OutdentRight, caseInsensitive)) {
		return CommandLayoutInterpretation_NEW_LINE_SEGMENT;
	}

	if (string_equals(command, COMMAND_Paragraph, caseInsensitive)) {
		return CommandLayoutInterpretation_NEW_ISOLATED_PARAGRAPH;
	}

	if (string_equals(command, COMMAND_SamePage, caseInsensitive)) {
		return CommandLayoutInterpretation_SAME_PAGE;
	}

	if (string_equals(command, COMMAND_Heading, caseInsensitive)) {
		return CommandLayoutInterpretation_HEADING_BLOCK;
	}

	if (string_equals(command, COMMAND_Footing, caseInsensitive)) {
		return CommandLayoutInterpretation_FOOTING_BLOCK;
	}

	if (string_equals(command, COMMAND_ISO_8859_1, caseInsensitive)
	    || string_equals(command, COMMAND_ISO_8859_2, caseInsensitive)
	    || string_equals(command, COMMAND_ISO_8859_3, caseInsensitive)
	    || string_equals(command, COMMAND_ISO_8859_4, caseInsensitive)
	    || string_equals(command, COMMAND_ISO_8859_5, caseInsensitive)
	    || string_equals(command, COMMAND_ISO_8859_6, caseInsensitive)
	    || string_equals(command, COMMAND_ISO_8859_7, caseInsensitive)
	    || string_equals(command, COMMAND_ISO_8859_8, caseInsensitive)
	    || string_equals(command, COMMAND_ISO_8859_9, caseInsensitive)
	    || string_equals(command, COMMAND_US_ASCII, caseInsensitive)
	    || string_equals(command, COMMAND_No_op, caseInsensitive)
	    ) {
		return CommandLayoutInterpretation_NO_OP;
	}

	if (string_equals(command, COMMAND_Comment, caseInsensitive)) {
		return CommandLayoutInterpretation_COMMENT;
	}

	if (string_equals(command, COMMAND_nl, caseInsensitive)) {
		return CommandLayoutInterpretation_NEW_LINE;
	}

	if (string_equals(command, COMMAND_np, caseInsensitive)) {
		return CommandLayoutInterpretation_NEW_PAGE;
	}

	return CommandLayoutInterpretation_CUSTOM;
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

Vector_ofTypeImplementation(LayoutLineSegment)
    Vector_ofTypeImplementation(LayoutLine)
    Vector_ofTypeImplementation(LayoutParagraph)
    Vector_ofTypeImplementation(LayoutBlock)
    Vector_ofTypeImplementation(LayoutResolverWarning)
    Vector_ofTypeImplementation(LayoutContentAlignment)
    Vector_ofTypeImplementation(LayoutBlockType)
