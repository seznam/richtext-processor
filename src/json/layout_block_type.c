#include "../layout_block_type.h"
#include "json_value.h"
#include "layout_block_type.h"

static const char *headingTypeValue = "HEADING";
static const char *footingTypeValue = "FOOTING";
static const char *mainContentTypeValue = "MAIN_CONTENT";
static const char *pageBreakTypeValue = "PAGE_BREAK";
static const char *samePageStartTypeValue = "SAME_PAGE_START";
static const char *samePageEndTypeValue = "SAME_PAGE_END";
static const char *customTypeValue = "CUSTOM";

JSONValue *LayoutBlockType_toJSON(type)
LayoutBlockType type;
{
	string *typeString;
	JSONValue *typeJson;

	switch (type) {
	case LayoutBlockType_HEADING:
		typeString = string_from(headingTypeValue);
		break;
	case LayoutBlockType_FOOTING:
		typeString = string_from(footingTypeValue);
		break;
	case LayoutBlockType_MAIN_CONTENT:
		typeString = string_from(mainContentTypeValue);
		break;
	case LayoutBlockType_PAGE_BREAK:
		typeString = string_from(pageBreakTypeValue);
		break;
	case LayoutBlockType_SAME_PAGE_START:
		typeString = string_from(samePageStartTypeValue);
		break;
	case LayoutBlockType_SAME_PAGE_END:
		typeString = string_from(samePageEndTypeValue);
		break;
	case LayoutBlockType_CUSTOM:
		typeString = string_from(customTypeValue);
		break;
	default:
		return NULL;
	}

	if (typeString == NULL) {
		return NULL;
	}

	typeJson = JSONValue_newString(typeString);
	if (typeJson == NULL) {
		string_free(typeString);
		return NULL;
	}

	return typeJson;
}
