#ifndef LAYOUT_BLOCK_TYPE_HEADER_FILE
#define LAYOUT_BLOCK_TYPE_HEADER_FILE 1

typedef enum LayoutBlockType {
	LayoutBlockType_HEADING,
	LayoutBlockType_FOOTING,
	LayoutBlockType_MAIN_CONTENT,
	LayoutBlockType_PAGE_BREAK,
	LayoutBlockType_SAME_PAGE_START,
	LayoutBlockType_SAME_PAGE_END,
	LayoutBlockType_CUSTOM
} LayoutBlockType;

#endif
