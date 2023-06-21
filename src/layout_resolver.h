#ifndef LAYOUT_RESOLVER_HEADER_FILE
#define LAYOUT_RESOLVER_HEADER_FILE 1

#include "ast_node.h"
#include "ast_node_pointer_vector.h"
#include "bool.h"
#include "custom_command_layout_interpretation.h"
#include "layout_block_vector.h"
#include "typed_vector.h"
#include "vector.h"

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
				    customCommandInterpreter(ASTNode *, bool),
				    bool caseInsensitiveCommands);

void LayoutResolverResult_free(LayoutResolverResult * result);

#endif
