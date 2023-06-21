#include "../layout_content_alignment.h"
#include "json_value.h"
#include "layout_content_alignment.h"

static const char *defaultAlignment = "DEFAULT";
static const char *justifyLeftAlignment = "JUSTIFY_LEFT";
static const char *justifyRightAlignment = "JUSTIFY_RIGHT";
static const char *centerAlignment = "CENTER";

JSONValue *LayoutContentAlignment_toJSON(alignment)
LayoutContentAlignment alignment;
{
	switch (alignment) {
	case LayoutContentAlignment_DEFAULT:
		return JSONValue_newString(string_from(defaultAlignment));

	case LayoutContentAlignment_JUSTIFY_LEFT:
		return JSONValue_newString(string_from(justifyLeftAlignment));

	case LayoutContentAlignment_JUSTIFY_RIGHT:
		return JSONValue_newString(string_from(justifyRightAlignment));

	case LayoutContentAlignment_CENTER:
		return JSONValue_newString(string_from(centerAlignment));

	default:
		return NULL;
	}
}
