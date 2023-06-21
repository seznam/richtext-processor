#ifndef LAYOUT_LINE_HEADER_FILE
#define LAYOUT_LINE_HEADER_FILE 1

#include "ast_node.h"
#include "layout_line_segment_vector.h"

typedef struct LayoutLine {
	ASTNode *causingCommand;
	LayoutLineSegmentVector *segments;
} LayoutLine;

#endif
