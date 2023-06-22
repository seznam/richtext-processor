#include "../layout_paragraph.h"
#include "../string.h"
#include "ast_node.h"
#include "json_value.h"
#include "layout_line.h"
#include "layout_paragraph.h"
#include "layout_paragraph_type.h"

static const char *causingCommandKeyValue = "causingCommand";
static const char *typeKeyValue = "type";
static const char *linesKeyValue = "lines";

JSONValue *LayoutParagraph_toJSON(paragraph)
LayoutParagraph *paragraph;
{
	JSONValue *paragraphJson;
	string *causingCommandKey, *typeKey, *linesKey;
	JSONValue *causingCommandValue, *typeValue, *linesValue;

	if (paragraph == NULL) {
		return JSONValue_newNull();
	}

	paragraphJson = JSONValue_newObject();
	causingCommandKey = string_from(causingCommandKeyValue);
	typeKey = string_from(typeKeyValue);
	linesKey = string_from(linesKeyValue);
	if (paragraphJson == NULL || causingCommandKey == NULL
	    || typeKey == NULL || linesKey == NULL) {
		JSONValue_free(paragraphJson);
		string_free(causingCommandKey);
		string_free(typeKey);
		string_free(linesKey);
		return NULL;
	}

	causingCommandValue = ASTNode_toJSON(paragraph->causingCommand);
	typeValue = LayoutParagraphType_toJSON(paragraph->type);
	linesValue = LayoutLineVector_toJSON(paragraph->lines);

	if (causingCommandValue == NULL || typeValue == NULL
	    || linesValue == NULL
	    || JSONValue_setObjectProperty(paragraphJson, causingCommandKey,
					   causingCommandValue) == NULL
	    || JSONValue_setObjectProperty(paragraphJson, typeKey,
					   typeValue) == NULL
	    || JSONValue_setObjectProperty(paragraphJson, linesKey,
					   linesValue) == NULL) {
		string_free(causingCommandKey);
		string_free(typeKey);
		string_free(linesKey);
		JSONValue_freeRecursive(causingCommandValue);
		JSONValue_freeRecursive(typeValue);
		JSONValue_freeRecursive(linesValue);
		JSONValue_free(paragraphJson);
		return NULL;
	}

	return paragraphJson;
}

JSONValue *LayoutParagraphVector_toJSON(paragraphs)
LayoutParagraphVector *paragraphs;
{
	JSONValue *paragraphsJson;
	LayoutParagraph *paragraph;
	unsigned long i;

	if (paragraphs == NULL) {
		return JSONValue_newNull();
	}

	paragraphsJson = JSONValue_newArray();
	if (paragraphsJson == NULL) {
		return NULL;
	}

	paragraph = paragraphs->items;
	for (i = 0; i < paragraphs->size.length; i++, paragraph++) {
		JSONValue *paragraphJson = LayoutParagraph_toJSON(paragraph);
		if (paragraphJson == NULL
		    || JSONValue_pushToArray(paragraphsJson,
					     paragraphJson) == NULL) {
			JSONValue_freeRecursive(paragraphJson);
			JSONValue_freeRecursive(paragraphsJson);
			return NULL;
		}
	}

	return paragraphsJson;
}
