#ifndef LAYOUT_POST_PROCESSOR_HEADER_FILE
#define LAYOUT_POST_PROCESSOR_HEADER_FILE 1

#include "ast_node.h"
#include "layout_block.h"
#include "layout_block_vector.h"
#include "layout_line.h"
#include "layout_line_segment.h"
#include "layout_paragraph.h"
#include "typed_vector.h"

typedef unsigned int LayoutPostProcessorWarningCode;

typedef struct LayoutPostProcessorWarning {
	LayoutBlock *block;
	LayoutParagraph *paragraph;
	LayoutLine *line;
	LayoutLineSegment *segment;
	ASTNode *node;
	LayoutPostProcessorWarningCode code;
} LayoutPostProcessorWarning;

Vector_ofType(LayoutPostProcessorWarning);

typedef unsigned int LayoutPostProcessorErrorCode;

typedef struct LayoutPostProcessorError {
	LayoutBlock *block;
	LayoutParagraph *paragraph;
	LayoutLine *line;
	LayoutLineSegment *segment;
	ASTNode *node;
	LayoutPostProcessorErrorCode code;
} LayoutPostProcessorError;

typedef struct LayoutPostProcessorResult {
	LayoutPostProcessorError *error;
	LayoutPostProcessorWarningVector *warnings;
} LayoutPostProcessorResult;

typedef LayoutPostProcessorResult *LayoutPostProcessor(LayoutBlockVector *
						       document);

void LayoutPostProcessorResult_free(LayoutPostProcessorResult * result);

#endif
