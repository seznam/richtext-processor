#include "../layout_resolver.h"
#include "ast_node.h"
#include "layout_content_alignment.h"
#include "layout_line_segment.h"

static const char *causingCommandKeyValue = "causingCommand";
static const char *contentAlignmentKeyValue = "contentAlignment";
static const char *leftIndentationLevelKeyValue = "leftIndentationLevel";
static const char *rightIndentationLevelKeyValue = "rightIndentationLevel";
static const char *fontSizeChangeKeyValue = "fontSizeChange";
static const char *fontBoldLevelKeyValue = "fontBoldLevel";
static const char *fontItalicLevelKeyValue = "fontItalicLevel";
static const char *fontUnderlinedLevelKeyValue = "fontUnderlinedLevel";
static const char *fontFixedLevelKeyValue = "fontFixedLevel";
static const char *otherSegmentMarkersKeyValue = "otherSegmentMarkers";
static const char *contentKeyValue = "content";

JSONValue *LayoutLineSegment_toJSON(segment)
LayoutLineSegment *segment;
{
	JSONValue *segmentJson;
	string *causingCommandKey;
	JSONValue *causingCommandValue;
	string *contentAlignmentKey;
	JSONValue *contentAlignmentValue;
	string *leftIndentationLevelKey;
	JSONValue *leftIndentationLevelValue;
	string *rightIndentationLevelKey;
	JSONValue *rightIndentationLevelValue;
	string *fontSizeChangeKey;
	JSONValue *fontSizeChangeValue;
	string *fontBoldLevelKey;
	JSONValue *fontBoldLevelValue;
	string *fontItalicLevelKey;
	JSONValue *fontItalicLevelValue;
	string *fontUnderlinedLevelKey;
	JSONValue *fontUnderlinedLevelValue;
	string *fontFixedLevelKey;
	JSONValue *fontFixedLevelValue;
	string *otherSegmentMarkersKey;
	JSONValue *otherSegmentMarkersValue;
	string *contentKey;
	JSONValue *contentValue;

	if (segment == NULL) {
		return JSONValue_newNull();
	}

	segmentJson = JSONValue_newObject();
	if (segmentJson == NULL) {
		return NULL;
	}

	causingCommandKey = string_from(causingCommandKeyValue);
	contentAlignmentKey = string_from(contentAlignmentKeyValue);
	leftIndentationLevelKey = string_from(leftIndentationLevelKeyValue);
	rightIndentationLevelKey = string_from(rightIndentationLevelKeyValue);
	fontSizeChangeKey = string_from(fontSizeChangeKeyValue);
	fontBoldLevelKey = string_from(fontBoldLevelKeyValue);
	fontItalicLevelKey = string_from(fontItalicLevelKeyValue);
	fontUnderlinedLevelKey = string_from(fontUnderlinedLevelKeyValue);
	fontFixedLevelKey = string_from(fontFixedLevelKeyValue);
	otherSegmentMarkersKey = string_from(otherSegmentMarkersKeyValue);
	contentKey = string_from(contentKeyValue);
	if (causingCommandKey == NULL || contentAlignmentKey == NULL
	    || leftIndentationLevelKey == NULL
	    || rightIndentationLevelKey == NULL
	    || fontSizeChangeKey == NULL
	    || fontBoldLevelKey == NULL
	    || fontItalicLevelKey == NULL
	    || fontUnderlinedLevelKey == NULL
	    || fontFixedLevelKey == NULL
	    || otherSegmentMarkersKey == NULL || contentKey == NULL) {
		string_free(causingCommandKey);
		string_free(contentAlignmentKey);
		string_free(leftIndentationLevelKey);
		string_free(rightIndentationLevelKey);
		string_free(fontSizeChangeKey);
		string_free(fontBoldLevelKey);
		string_free(fontItalicLevelKey);
		string_free(fontUnderlinedLevelKey);
		string_free(fontFixedLevelKey);
		string_free(otherSegmentMarkersKey);
		string_free(contentKey);
		JSONValue_free(segmentJson);
		return NULL;
	}

	causingCommandValue = ASTNode_toJSON(segment->causingCommand);
	contentAlignmentValue =
	    LayoutContentAlignment_toJSON(segment->contentAlignment);
	leftIndentationLevelValue =
	    JSONValue_newNumber(segment->leftIndentationLevel);
	rightIndentationLevelValue =
	    JSONValue_newNumber(segment->rightIndentationLevel);
	fontSizeChangeValue = JSONValue_newNumber(segment->fontSizeChange);
	fontBoldLevelValue = JSONValue_newNumber(segment->fontBoldLevel);
	fontItalicLevelValue = JSONValue_newNumber(segment->fontItalicLevel);
	fontUnderlinedLevelValue =
	    JSONValue_newNumber(segment->fontUnderlinedLevel);
	fontFixedLevelValue = JSONValue_newNumber(segment->fontFixedLevel);
	otherSegmentMarkersValue =
	    ASTNodePointerVector_toJSON(segment->otherSegmentMarkers);
	contentValue = ASTNodePointerVector_toJSON(segment->content);

	if (causingCommandKey == NULL
	    || contentAlignmentKey == NULL
	    || leftIndentationLevelKey == NULL
	    || rightIndentationLevelKey == NULL
	    || fontSizeChangeKey == NULL
	    || fontBoldLevelKey == NULL
	    || fontItalicLevelKey == NULL
	    || fontUnderlinedLevelKey == NULL
	    || fontFixedLevelKey == NULL
	    || otherSegmentMarkersKey == NULL
	    || contentKey == NULL
	    || causingCommandValue == NULL
	    || contentAlignmentValue == NULL
	    || leftIndentationLevelValue == NULL
	    || rightIndentationLevelValue == NULL
	    || fontSizeChangeValue == NULL
	    || fontBoldLevelValue == NULL
	    || fontItalicLevelValue == NULL
	    || fontUnderlinedLevelValue == NULL
	    || fontFixedLevelValue == NULL
	    || otherSegmentMarkersValue == NULL
	    || contentValue == NULL
	    || JSONValue_setObjectProperty(segmentJson, causingCommandKey,
					   causingCommandValue) == NULL
	    || JSONValue_setObjectProperty(segmentJson, contentAlignmentKey,
					   contentAlignmentValue) == NULL
	    || JSONValue_setObjectProperty(segmentJson, leftIndentationLevelKey,
					   leftIndentationLevelValue) == NULL
	    || JSONValue_setObjectProperty(segmentJson,
					   rightIndentationLevelKey,
					   rightIndentationLevelValue) == NULL
	    || JSONValue_setObjectProperty(segmentJson, fontSizeChangeKey,
					   fontSizeChangeValue) == NULL
	    || JSONValue_setObjectProperty(segmentJson, fontBoldLevelKey,
					   fontBoldLevelValue) == NULL
	    || JSONValue_setObjectProperty(segmentJson, fontItalicLevelKey,
					   fontItalicLevelValue) == NULL
	    || JSONValue_setObjectProperty(segmentJson, fontUnderlinedLevelKey,
					   fontUnderlinedLevelValue) == NULL
	    || JSONValue_setObjectProperty(segmentJson, fontFixedLevelKey,
					   fontFixedLevelValue) == NULL
	    || JSONValue_setObjectProperty(segmentJson, otherSegmentMarkersKey,
					   otherSegmentMarkersValue) == NULL
	    || JSONValue_setObjectProperty(segmentJson, contentKey,
					   contentValue) == NULL) {
		string_free(causingCommandKey);
		string_free(contentAlignmentKey);
		string_free(leftIndentationLevelKey);
		string_free(rightIndentationLevelKey);
		string_free(fontSizeChangeKey);
		string_free(fontBoldLevelKey);
		string_free(fontItalicLevelKey);
		string_free(fontUnderlinedLevelKey);
		string_free(fontFixedLevelKey);
		string_free(otherSegmentMarkersKey);
		string_free(contentKey);

		JSONValue_free(causingCommandValue);
		JSONValue_free(contentAlignmentValue);
		JSONValue_free(leftIndentationLevelValue);
		JSONValue_free(rightIndentationLevelValue);
		JSONValue_free(fontSizeChangeValue);
		JSONValue_free(fontBoldLevelValue);
		JSONValue_free(fontItalicLevelValue);
		JSONValue_free(fontUnderlinedLevelValue);
		JSONValue_free(fontFixedLevelValue);
		JSONValue_free(otherSegmentMarkersValue);
		JSONValue_free(contentValue);

		JSONValue_free(segmentJson);

		return NULL;
	}

	return segmentJson;
}

JSONValue *LayoutLineSegmentVector_toJSON(segments)
LayoutLineSegmentVector *segments;
{
	JSONValue *segmentsJson;
	unsigned long i;
	LayoutLineSegment *segment;

	if (segments == NULL) {
		return JSONValue_newNull();
	}

	segmentsJson = JSONValue_newArray();
	if (segmentsJson == NULL) {
		return NULL;
	}

	segment = segments->items;
	for (i = 0; i < segments->size.length; i++, segment++) {
		JSONValue *segmentJson = LayoutLineSegment_toJSON(segment);
		JSONValue *grownArray = NULL;
		if (segmentJson == NULL) {
			JSONValue_freeRecursive(segmentsJson);
			return NULL;
		}
		grownArray = JSONValue_pushToArray(segmentsJson, segmentJson);
		if (grownArray == NULL) {
			JSONValue_freeRecursive(segmentJson);
			JSONValue_freeRecursive(segmentsJson);
			return NULL;
		}
	}

	return segmentsJson;
}
