#include "../layout_resolver.h"
#include "../string.h"
#include "ast_node.h"
#include "layout_line.h"
#include "layout_line_segment.h"

static const char *causingCommandKeyValue = "causingCommand";
static const char *segmentsKeyValue = "segments";

JSONValue *LayoutLine_toJSON(line)
LayoutLine *line;
{
	JSONValue *lineJson;
	string *causingCommandKey;
	JSONValue *causingCommandValue;
	string *segmentsKey;
	JSONValue *segmentsValue;

	if (line == NULL) {
		return JSONValue_newNull();
	}

	lineJson = JSONValue_newObject();
	if (lineJson == NULL) {
		return NULL;
	}

	causingCommandKey = string_from(causingCommandKeyValue);
	causingCommandValue = ASTNode_toJSON(line->causingCommand);
	segmentsKey = string_from(segmentsKeyValue);
	segmentsValue = LayoutLineSegmentVector_toJSON(line->segments);

	if (causingCommandKey == NULL
	    || causingCommandValue == NULL
	    || segmentsKey == NULL
	    || segmentsValue == NULL
	    || JSONValue_setObjectProperty(lineJson, causingCommandKey,
					   causingCommandValue) == NULL
	    || JSONValue_setObjectProperty(lineJson, segmentsKey,
					   segmentsValue) == NULL) {
		string_free(causingCommandKey);
		JSONValue_free(causingCommandValue);
		string_free(segmentsKey);
		JSONValue_free(segmentsValue);
		JSONValue_free(lineJson);
		return NULL;
	}

	return lineJson;
}

JSONValue *LayoutLineVector_toJSON(lines)
LayoutLineVector *lines;
{
	JSONValue *linesJson;
	LayoutLine *line;
	unsigned long i;

	if (lines == NULL) {
		return JSONValue_newNull();
	}

	linesJson = JSONValue_newArray();
	if (lines == NULL) {
		return NULL;
	}

	line = lines->items;
	for (i = 0; i < lines->size.length; i++, line++) {
		JSONValue *lineJson = LayoutLine_toJSON(line);
		if (lineJson == NULL
		    || JSONValue_pushToArray(linesJson, lineJson) == NULL) {
			JSONValue_freeRecursive(lineJson);
			JSONValue_freeRecursive(linesJson);
			return NULL;
		}
	}

	return linesJson;
}
