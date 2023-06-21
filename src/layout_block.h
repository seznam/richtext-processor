#ifndef LAYOUT_BLOCK_HEADER_FILE
#define LAYOUT_BLOCK_HEADER_FILE 1

#include "ast_node.h"
#include "layout_block_type.h"
#include "layout_paragraph_vector.h"

typedef struct LayoutBlock {
	ASTNode *causingCommand;
	LayoutBlockType type;
	LayoutParagraphVector *paragraphs;
} LayoutBlock;

#endif
