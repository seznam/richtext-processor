#ifndef JSON_LAYOUT_PARAGRAPH_HEADER_FILE
#define JSON_LAYOUT_PARAGRAPH_HEADER_FILE 1

#include "../layout_paragraph.h"
#include "../layout_paragraph_vector.h"
#include "json_value.h"

JSONValue *LayoutParagraph_toJSON(LayoutParagraph * paragraph);

JSONValue *LayoutParagraphVector_toJSON(LayoutParagraphVector * paragraphs);

#endif
