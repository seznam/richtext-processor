#ifndef OUTPUT_RENDERER_HEADER_FILE
#define OUTPUT_RENDERER_HEADER_FILE 1

#include "ast_node.h"
#include "layout_block.h"
#include "layout_block_vector.h"
#include "layout_line.h"
#include "layout_line_segment.h"
#include "layout_paragraph.h"
#include "string.h"
#include "typed_vector.h"

typedef unsigned int OutputRendererWarningCode;

typedef struct OutputRendererWarning {
	LayoutBlock *block;
	LayoutParagraph *paragraph;
	LayoutLine *line;
	LayoutLineSegment *segment;
	ASTNode *node;
	OutputRendererWarningCode code;
} OutputRendererWarning;

Vector_ofType(OutputRendererWarning);

typedef unsigned int OutputRendererErrorCode;

typedef struct OutputRendererError {
	LayoutBlock *block;
	LayoutParagraph *paragraph;
	LayoutLine *line;
	LayoutLineSegment *segment;
	ASTNode *node;
	OutputRendererErrorCode code;
} OutputRendererError;

typedef enum OutputRendererResultType {
	OutputRendererResultType_SUCCESS,
	OutputRendererResultType_ERROR
} OutputRendererResultType;

typedef struct OutputRendererResult {
	OutputRendererResultType type;
	union {
		string *output;
		OutputRendererError error;
	} result;
	OutputRendererWarningVector *warnings;
} OutputRendererResult;

typedef OutputRendererResult *OutputRenderer(LayoutBlockVector *
					     richtextDocument,
					     void *configuration);

void OutputRendererResult_free(OutputRendererResult * result);

#endif
