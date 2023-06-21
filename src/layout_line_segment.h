#ifndef LAYOUT_LINE_SEGMENT_HEADER_FILE
#define LAYOUT_LINE_SEGMENT_HEADER_FILE 1

#include "ast_node.h"
#include "ast_node_pointer_vector.h"
#include "layout_content_alignment.h"

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

#endif
