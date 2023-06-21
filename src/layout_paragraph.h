#ifndef LAYOUT_PARAGRAPH_HEADER_FILE
#define LAYOUT_PARAGRAPH_HEADER_FILE 1

#include "ast_node.h"
#include "layout_paragraph_type.h"
#include "layout_line_vector.h"

typedef struct LayoutParagraph {
	ASTNode *causingCommand;
	LayoutParagraphType type;
	LayoutLineVector *lines;
} LayoutParagraph;

#endif
