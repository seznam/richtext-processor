#ifndef LAYOUT_PARAGRAPH_TYPE_HEADER_FILE
#define LAYOUT_PARAGRAPH_TYPE_HEADER_FILE 1

typedef enum LayoutParagraphType {
	/*
	 * Only used for paragraphs started by the <Paragraph> command or
	 * </Paragraph> command end if nested within another <Paragraph>, or
	 * custom command representing isolated paragraph.
	 */
	LayoutParagraphType_EXPLICIT,
	LayoutParagraphType_IMPLICIT
} LayoutParagraphType;

#endif
