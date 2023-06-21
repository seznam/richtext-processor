#include "../layout_paragraph_type.h"
#include "json_value.h"
#include "layout_paragraph_type.h"

static const char *explicitTypeValue = "EXPLICIT";
static const char *implicitTypeValue = "IMPLICIT";

JSONValue *LayoutParagraphType_toJSON(type)
LayoutParagraphType type;
{
	switch (type) {
	case LayoutParagraphType_EXPLICIT:
		return JSONValue_newString(string_from(explicitTypeValue));

	case LayoutParagraphType_IMPLICIT:
		return JSONValue_newString(string_from(implicitTypeValue));

	default:
		return NULL;
	}
}
