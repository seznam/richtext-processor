#include <stdlib.h>
#include "../../src/json/layout_line_segment.h"
#include "../../src/json/json_encoder.h"
#include "../../src/layout_resolver.h"
#include "../../src/parser.h"
#include "../../src/string.h"
#include "../unit.h"

/*
   This file does not bother to free heap-allocated memory because it's a suite
   of unit tests, ergo it's not a big deal.
 */

unsigned int tests_run = 0;
unsigned int tests_failed = 0;

static void all_tests(void);

int main(void);

START_TEST(LayoutLineSegment_toJSON_returnsJsonNullForNullInput)
{
	JSONValue *result = LayoutLineSegment_toJSON(NULL);
	assert(result != NULL
	       && result->type == JSONValueType_NULL,
	       "Expected JSON null as result");
END_TEST}

START_TEST(LayoutLineSegment_toJSON_encodesProvidedSegment)
{
	LayoutLineSegment segment;
	ASTNode causingCommand, *marker, *content;
	JSONValue *result;
	string *serialized, *expected;
	char *serializedContent;
	char *serializedSegmentTemplate, *serializedSegment;

	marker = malloc(sizeof(ASTNode));
	marker->byteIndex = 5;
	marker->codepointIndex = 7;
	marker->tokenIndex = 6;
	marker->type = ASTNodeType_COMMAND;
	marker->value = string_from("bar");
	marker->parent = NULL;
	marker->children = ASTNodePointerVector_new(0, 0);

	content = malloc(sizeof(ASTNode));
	content->byteIndex = 9;
	content->codepointIndex = 7;
	content->tokenIndex = 4;
	content->type = ASTNodeType_TEXT;
	content->value = string_from("baz");
	content->parent = &causingCommand;
	content->children = ASTNodePointerVector_new(0, 0);

	causingCommand.byteIndex = 1;
	causingCommand.codepointIndex = 2;
	causingCommand.tokenIndex = 3;
	causingCommand.type = ASTNodeType_COMMAND;
	causingCommand.value = string_from("foo");
	causingCommand.parent = NULL;
	causingCommand.children = ASTNodePointerVector_of1(content);

	segment.causingCommand = &causingCommand;
	segment.contentAlignment = LayoutContentAlignment_CENTER;
	segment.leftIndentationLevel = 3;
	segment.rightIndentationLevel = -2;
	segment.fontSizeChange = 4;
	segment.fontBoldLevel = 5;
	segment.fontItalicLevel = 1;
	segment.fontUnderlinedLevel = 7;
	segment.fontFixedLevel = 9;
	segment.otherSegmentMarkers = ASTNodePointerVector_of1(marker);
	segment.content = ASTNodePointerVector_of1(content);

	result = LayoutLineSegment_toJSON(&segment);
	serialized = JSON_encode(result);
	serializedContent =
	    "{\"byteIndex\":9,\"codepointIndex\":7,\"tokenIndex\":4,\"type\":\"TEXT\",\"value\":\"baz\",\"children\":[]}";
	serializedSegmentTemplate =
	    "{\"causingCommand\":{\"byteIndex\":1,\"codepointIndex\":2,\"tokenIndex\":3,\"type\":\"COMMAND\",\"value\":\"foo\",\"children\":[%s]},\"contentAlignment\":\"CENTER\",\"leftIndentationLevel\":3,\"rightIndentationLevel\":-2,\"fontSizeChange\":4,\"fontBoldLevel\":5,\"fontItalicLevel\":1,\"fontUnderlinedLevel\":7,\"fontFixedLevel\":9,\"otherSegmentMarkers\":[{\"byteIndex\":5,\"codepointIndex\":7,\"tokenIndex\":6,\"type\":\"COMMAND\",\"value\":\"bar\",\"children\":[]}],\"content\":[%s]}";
	serializedSegment =
	    malloc(strlen(serializedSegmentTemplate) +
		   2 * strlen(serializedContent) + 1);
	sprintf(serializedSegment, serializedSegmentTemplate, serializedContent,
		serializedContent);
	expected = string_from(serializedSegment);
	assert(string_compare(serialized, expected) == 0,
	       "The serialized line segment did not match expected output");
END_TEST}

START_TEST(LayoutLineSegmentVector_toJSON_returnsJsonNullForNullInput)
{
	JSONValue *result;
	result = LayoutLineSegmentVector_toJSON(NULL);
	assert(result != NULL
	       && result->type == JSONValueType_NULL,
	       "Expected JSON null result");
END_TEST}

START_TEST(LayoutLineSegmentVector_toJSON_returnsEmptyArrayForEmptyVector)
{
	JSONValue *result;
	string *serialized;

	result =
	    LayoutLineSegmentVector_toJSON(LayoutLineSegmentVector_new(0, 0));
	serialized = JSON_encode(result);
	assert(string_compare(serialized, string_from("[]")) == 0,
	       "The serialized empty segments vectors was not an empty array");
END_TEST}

START_TEST(LayoutLineSegmentVector_toJSON_serializedAllSegments)
{

	LayoutLineSegment segment1, segment2;
	ASTNode causingCommand, *marker, *content;
	LayoutLineSegmentVector *segments;
	JSONValue *result;
	string *serialized, *expected;
	char *serializedContent;
	char *serializedCauseTemplate, *serializedCause;
	char *serializedMarker;
	char *serializedSegmentTemplate, *serializedSegment1,
	    *serializedSegment2;
	char *serializedSegments;
	unsigned long i;

	marker = malloc(sizeof(ASTNode));
	marker->byteIndex = 5;
	marker->codepointIndex = 7;
	marker->tokenIndex = 6;
	marker->type = ASTNodeType_COMMAND;
	marker->value = string_from("bar");
	marker->parent = NULL;
	marker->children = ASTNodePointerVector_new(0, 0);

	content = malloc(sizeof(ASTNode));
	content->byteIndex = 9;
	content->codepointIndex = 7;
	content->tokenIndex = 4;
	content->type = ASTNodeType_TEXT;
	content->value = string_from("baz");
	content->parent = &causingCommand;
	content->children = ASTNodePointerVector_new(0, 0);

	causingCommand.byteIndex = 1;
	causingCommand.codepointIndex = 2;
	causingCommand.tokenIndex = 3;
	causingCommand.type = ASTNodeType_COMMAND;
	causingCommand.value = string_from("foo");
	causingCommand.parent = NULL;
	causingCommand.children = ASTNodePointerVector_of1(content);

	segment1.causingCommand = &causingCommand;
	segment1.contentAlignment = LayoutContentAlignment_CENTER;
	segment1.leftIndentationLevel = 3;
	segment1.rightIndentationLevel = -2;
	segment1.fontSizeChange = 4;
	segment1.fontBoldLevel = 5;
	segment1.fontItalicLevel = 1;
	segment1.fontUnderlinedLevel = 7;
	segment1.fontFixedLevel = 9;
	segment1.otherSegmentMarkers = ASTNodePointerVector_of1(marker);
	segment1.content = ASTNodePointerVector_of1(content);

	segment2.causingCommand = &causingCommand;
	segment2.contentAlignment = LayoutContentAlignment_JUSTIFY_RIGHT;
	segment2.leftIndentationLevel = -1;
	segment2.rightIndentationLevel = 8;
	segment2.fontSizeChange = 0;
	segment2.fontBoldLevel = 3;
	segment2.fontItalicLevel = 4;
	segment2.fontUnderlinedLevel = 2;
	segment2.fontFixedLevel = 6;
	segment2.otherSegmentMarkers = ASTNodePointerVector_of1(marker);
	segment2.content = ASTNodePointerVector_of1(content);

	segments = LayoutLineSegmentVector_of2(segment1, segment2);
	result = LayoutLineSegmentVector_toJSON(segments);
	serialized = JSON_encode(result);

	serializedContent =
	    "{\"byteIndex\":9,\"codepointIndex\":7,\"tokenIndex\":4,\"type\":\"TEXT\",\"value\":\"baz\",\"children\":[]}";
	serializedCauseTemplate =
	    "{\"byteIndex\":1,\"codepointIndex\":2,\"tokenIndex\":3,\"type\":\"COMMAND\",\"value\":\"foo\",\"children\":[%s]}";
	serializedCause =
	    malloc(strlen(serializedCauseTemplate) + strlen(serializedContent) +
		   1);
	sprintf(serializedCause, serializedCauseTemplate, serializedContent);
	serializedMarker =
	    "{\"byteIndex\":5,\"codepointIndex\":7,\"tokenIndex\":6,\"type\":\"COMMAND\",\"value\":\"bar\",\"children\":[]}";
	serializedSegmentTemplate =
	    "{\"causingCommand\":%s,\"contentAlignment\":\"%s\",\"leftIndentationLevel\":%d,\"rightIndentationLevel\":%d,\"fontSizeChange\":%d,\"fontBoldLevel\":%u,\"fontItalicLevel\":%u,\"fontUnderlinedLevel\":%u,\"fontFixedLevel\":%u,\"otherSegmentMarkers\":[%s],\"content\":[%s]}";
	serializedSegment1 =
	    malloc(strlen(serializedSegmentTemplate) + strlen(serializedCause) +
		   13 + 20 + 20 + 20 + 20 + 20 + 20 + 20 +
		   strlen(serializedMarker) + strlen(serializedContent) + 1);
	sprintf(serializedSegment1, serializedSegmentTemplate, serializedCause,
		"CENTER", segment1.leftIndentationLevel,
		segment1.rightIndentationLevel, segment1.fontSizeChange,
		segment1.fontBoldLevel, segment1.fontItalicLevel,
		segment1.fontUnderlinedLevel, segment1.fontFixedLevel,
		serializedMarker, serializedContent);
	serializedSegment2 =
	    malloc(strlen(serializedSegmentTemplate) + strlen(serializedCause) +
		   13 + 20 + 20 + 20 + 20 + 20 + 20 + 20 +
		   strlen(serializedMarker) + strlen(serializedContent) + 1);
	sprintf(serializedSegment2, serializedSegmentTemplate, serializedCause,
		"JUSTIFY_RIGHT", segment2.leftIndentationLevel,
		segment2.rightIndentationLevel, segment2.fontSizeChange,
		segment2.fontBoldLevel, segment2.fontItalicLevel,
		segment2.fontUnderlinedLevel, segment2.fontFixedLevel,
		serializedMarker, serializedContent);

	serializedSegments =
	    malloc(3 + strlen(serializedSegment1) + strlen(serializedSegment2) +
		   1);
	sprintf(serializedSegments, "[%s,%s]", serializedSegment1,
		serializedSegment2);

	expected = string_from(serializedSegments);
	for (i = 0; i < expected->length; i++) {
		if (*(serialized->content + i) != *(expected->content + i)) {
			printf("%lu\n", i);
			break;
		}
	}
	assert(string_compare(serialized, expected) == 0,
	       "The serialized line segments did not match expected output");
END_TEST}

static void all_tests()
{
	runTest(LayoutLineSegment_toJSON_returnsJsonNullForNullInput);
	runTest(LayoutLineSegment_toJSON_encodesProvidedSegment);
	runTest(LayoutLineSegmentVector_toJSON_returnsJsonNullForNullInput);
	runTest(LayoutLineSegmentVector_toJSON_returnsEmptyArrayForEmptyVector);
	runTest(LayoutLineSegmentVector_toJSON_serializedAllSegments);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
