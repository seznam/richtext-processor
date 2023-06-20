#ifndef LAYOUT_RESOLVER_HEADER_FILE
#define LAYOUT_RESOLVER_HEADER_FILE 1

#include "ast_node.h"
#include "ast_node_pointer_vector.h"
#include "bool.h"
#include "typed_vector.h"
#include "vector.h"

typedef enum CustomCommandLayoutInterpretation {
	/*
	 * Can be used for implementing additional content areas, for example an
	 * aside column.
	 */
	CustomCommandLayoutInterpretation_NEW_BLOCK,

	/*
	 * Starts a new implicit paragraph that continues after the causing
	 * command's end. The command's end will not terminate the current line
	 * either (use the <nl> command if desired), but will start a new line
	 * segment.
	 */
	CustomCommandLayoutInterpretation_NEW_PARAGRAPH,

	/*
	 * Starts a new implicit paragraph that ends after the causing command's
	 * end.
	 */
	CustomCommandLayoutInterpretation_NEW_ISOLATED_PARAGRAPH,

	/*
	 * Starts a new line that continues after the causing command's end.
	 * The command's end will start a new line segment.
	 */
	CustomCommandLayoutInterpretation_NEW_LINE,

	/*
	 * Starts a new line that ends afters the causing command's end.
	 */
	CustomCommandLayoutInterpretation_NEW_ISOLATED_LINE,

	/*
	 * Starts a new line segment that ends after the causing command's end.
	 */
	CustomCommandLayoutInterpretation_NEW_LINE_SEGMENT,

	/*
	 * The command will be appended to the current line segment's content.
	 * Note that the content of the command will be processed too, with its
	 * inline content appended to the current line segment's content - this
	 * is to enable processing of line segments' content in a linear
	 * fashion instead of having to traverse more tree structures.
	 */
	CustomCommandLayoutInterpretation_INLINE_CONTENT,

	/*
	 * The command has no effect, layout, formatting or otherwise, and will
	 * be replaced by its children. This is the default behavior for custom
	 * commands if no custom command hook is provided.
	 */
	CustomCommandLayoutInterpretation_NO_OP,

	/*
	 * The command is not accepted by the custom command layout interpreter.
	 * This can be used to reject invalid, incorrectly placed, or forbidden
	 * commands, or can be used to more strictly validate the command names.
	 *
	 * Note that implementations should treat all unknown commands as No-op,
	 * and a custom command layout interpreter should conform to specified
	 * behavior as well. Custom command implementors are recommended to use
	 * this interpretation for stricter command name and hierarchy
	 * validation only.
	 */
	CustomCommandLayoutInterpretation_INVALID_COMMAND
} CustomCommandLayoutInterpretation;

typedef enum LayoutContentAlignment {
	LayoutContentAlignment_DEFAULT,
	LayoutContentAlignment_JUSTIFY_LEFT,
	LayoutContentAlignment_JUSTIFY_RIGHT,
	LayoutContentAlignment_CENTER
} LayoutContentAlignment;

typedef struct LayoutLineSegment {
	ASTNode *causingCommand;
	LayoutContentAlignment contentAlignment;
	signed short leftIndentationLevel;
	signed short rightIndentationLevel;
	signed short fontSizeChange;
	unsigned short fontBoldLevel;
	unsigned short fontItalicLevel;
	unsigned short fontUnderlinedLevel;
	unsigned short fontFixedLevel;
	/*
	 * Used for the <Subscript>, <Superscript>, <Excerpt>, <Signature> and
	 * new line segment-causing custom commands.
	 */
	ASTNodePointerVector *otherSegmentMarkers;
	ASTNodePointerVector *content;
} LayoutLineSegment;

Vector_ofType(LayoutLineSegment)
typedef struct LayoutLine {
	ASTNode *causingCommand;
	LayoutLineSegmentVector *segments;
} LayoutLine;

Vector_ofType(LayoutLine)
typedef enum LayoutParagraphType {
	/*
	 * Only used for paragraphs started by the <Paragraph> command or
	 * </Paragraph> command end if nested within another <Paragraph>, or
	 * custom command representing isolated paragraph.
	 */
	LayoutParagraphType_EXPLICIT,
	LayoutParagraphType_IMPLICIT
} LayoutParagraphType;

typedef struct LayoutParagraph {
	ASTNode *causingCommand;
	LayoutParagraphType type;
	LayoutLineVector *lines;
} LayoutParagraph;

Vector_ofType(LayoutParagraph)
typedef enum LayoutBlockType {
	LayoutBlockType_HEADING,
	LayoutBlockType_FOOTING,
	LayoutBlockType_MAIN_CONTENT,
	LayoutBlockType_PAGE_BREAK,
	LayoutBlockType_SAME_PAGE_START,
	LayoutBlockType_SAME_PAGE_END,
	LayoutBlockType_CUSTOM
} LayoutBlockType;

typedef struct LayoutBlock {
	ASTNode *causingCommand;
	LayoutBlockType type;
	LayoutParagraphVector *paragraphs;
} LayoutBlock;

Vector_ofType(LayoutBlock)
typedef enum LayoutResolverErrorCode {
	LayoutResolverErrorCode_OK,
	LayoutResolverErrorCode_NULL_NODES_PROVIDED,
	LayoutResolverErrorCode_NON_ROOT_NODES_PROVIDED,
	LayoutResolverErrorCode_UNSUPPORTED_NODE_TYPE,
	LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND,
	LayoutResolverErrorCode_INVALID_CUSTOM_COMMAND_INTERPRETATION,
	LayoutResolverErrorCode_UNSUPPORTED_LAYOUT_INTERPRETATION,
	LayoutResolverErrorCode_UNTRANSLATED_CUSTOM_LAYOUT_INTERPRETATION,
	LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINE_SEGMENTS,
	LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_LINES,
	LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_PARAGRAPHS,
	LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_BLOCKS,
	LayoutResolverErrorCode_OUT_OF_MEMORY_FOR_WARNINGS,
	LayoutResolverErrorCode_BLOCK_TYPE_STACK_UNDERFLOW,
	LayoutResolverErrorCode_CONTENT_ALIGNMENT_STACK_UNDERFLOW,
	LayoutResolverErrorCode_OTHER_SEGMENT_MARKER_STACK_UNDERFLOW,
	LayoutResolverErrorCode_OTHER_SEGMENT_MARKER_STACK_INCONSISTENCY,
	LayoutResolverErrorCode_LEFT_INDENTATION_LEVEL_UNDERFLOW,
	LayoutResolverErrorCode_LEFT_INDENTATION_LEVEL_OVERFLOW,
	LayoutResolverErrorCode_RIGHT_INDENTATION_LEVEL_UNDERFLOW,
	LayoutResolverErrorCode_RIGHT_INDENTATION_LEVEL_OVERFLOW,
	LayoutResolverErrorCode_FONT_SIZE_CHANGE_UNDERFLOW,
	LayoutResolverErrorCode_FONT_SIZE_CHANGE_OVERFLOW,
	LayoutResolverErrorCode_FONT_BOLD_LEVEL_UNDERFLOW,
	LayoutResolverErrorCode_FONT_BOLD_LEVEL_OVERFLOW,
	LayoutResolverErrorCode_FONT_ITALIC_LEVEL_UNDERFLOW,
	LayoutResolverErrorCode_FONT_ITALIC_LEVEL_OVERFLOW,
	LayoutResolverErrorCode_FONT_UNDERLINED_LEVEL_UNDERFLOW,
	LayoutResolverErrorCode_FONT_UNDERLINED_LEVEL_OVERFLOW,
	LayoutResolverErrorCode_FONT_FIXED_LEVEL_UNDERFLOW,
	LayoutResolverErrorCode_FONT_FIXED_LEVEL_OVERFLOW
} LayoutResolverErrorCode;

typedef struct LayoutResolverError {
	ASTNode *location;
	LayoutResolverErrorCode code;
} LayoutResolverError;

typedef enum LayoutResolverWarningCode {
	LayoutResolverWarningCode_NEW_PAGE_INSIDE_SAME_PAGE,
	LayoutResolverWarningCode_NESTED_SAME_PAGE
} LayoutResolverWarningCode;

typedef struct LayoutResolverWarning {
	ASTNode *cause;
	LayoutResolverWarningCode code;
} LayoutResolverWarning;

Vector_ofType(LayoutResolverWarning)
typedef enum LayoutResolverResultType {
	LayoutResolverResultType_SUCCESS,
	LayoutResolverResultType_ERROR
} LayoutResolverResultType;

typedef struct LayoutResolverResult {
	LayoutResolverResultType type;
	union {
		LayoutBlockVector *blocks;
		LayoutResolverError error;
	} result;
	LayoutResolverWarningVector *warnings;
} LayoutResolverResult;

LayoutResolverResult *resolveLayout(ASTNodePointerVector * nodes,
				    CustomCommandLayoutInterpretation
				    customCommandHook(ASTNode *, bool),
				    bool caseInsensitiveCommands);

void LayoutResolverResult_free(LayoutResolverResult * result);

#endif
