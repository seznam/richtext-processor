#include "../../src/json/layout_line.h"
#include "../../src/json/json_encoder.h"
#include "../../src/layout_content_alignment.h"
#include "../../src/layout_line.h"
#include "../../src/layout_line_vector.h"
#include "../../src/ast_node.h"
#include "../../src/ast_node_pointer_vector.h"
#include "../../src/ast_node_type.h"
#include "../unit.h"

/*
   This file does not bother to free heap-allocated memory because it's a suite
   of unit tests, ergo it's not a big deal.
 */

unsigned int tests_run = 0;
unsigned int tests_failed = 0;

static void all_tests(void);

int main(void);

START_TEST(LayoutLine_toJSON_returnsJsonNullForNullInput)
{
	JSONValue *result = LayoutLine_toJSON(NULL);
	assert(result != NULL
	       && result->type == JSONValueType_NULL,
	       "Expected JSON null result");
END_TEST}

START_TEST(LayoutLine_toJSON_encodesTheProvidedLine)
{
	JSONValue *json;
	LayoutLine line;
	ASTNode causingCommand;
	LayoutLineSegment segment;
	char *serializedCommand;
	char *serializedSegmentTemplate, *serializedSegment;
	char *serializedLineTemplate, *serializedLine;
	string *serializedJson;

	causingCommand.byteIndex = 1;
	causingCommand.codepointIndex = 2;
	causingCommand.tokenIndex = 3;
	causingCommand.type = ASTNodeType_COMMAND;
	causingCommand.value = string_from("foo");
	causingCommand.parent = NULL;
	causingCommand.children = ASTNodePointerVector_new(0, 0);

	segment.causingCommand = &causingCommand;
	segment.contentAlignment = LayoutContentAlignment_CENTER;
	segment.leftIndentationLevel = 3;
	segment.rightIndentationLevel = -2;
	segment.fontSizeChange = 4;
	segment.fontBoldLevel = 5;
	segment.fontItalicLevel = 1;
	segment.fontUnderlinedLevel = 7;
	segment.fontFixedLevel = 9;
	segment.otherSegmentMarkers = ASTNodePointerVector_new(0, 0);
	segment.content = ASTNodePointerVector_new(0, 0);

	line.causingCommand = &causingCommand;
	line.segments = LayoutLineSegmentVector_of1(segment);

	json = LayoutLine_toJSON(&line);
	serializedJson = JSON_encode(json);

	serializedCommand =
	    "{\"byteIndex\":1,\"codepointIndex\":2,\"tokenIndex\":3,\"type\":\"COMMAND\",\"value\":\"foo\",\"children\":[]}";
	serializedSegmentTemplate =
	    "{\"causingCommand\":%s,\"contentAlignment\":\"CENTER\",\"leftIndentationLevel\":3,\"rightIndentationLevel\":-2,\"fontSizeChange\":4,\"fontBoldLevel\":5,\"fontItalicLevel\":1,\"fontUnderlinedLevel\":7,\"fontFixedLevel\":9,\"otherSegmentMarkers\":[],\"content\":[]}";
	serializedSegment =
	    malloc(sizeof(char) *
		   (strlen(serializedCommand) +
		    strlen(serializedSegmentTemplate) + 1));
	sprintf(serializedSegment, serializedSegmentTemplate,
		serializedCommand);
	serializedLineTemplate = "{\"causingCommand\":%s,\"segments\":[%s]}";
	serializedLine =
	    malloc(sizeof(char) *
		   (strlen(serializedLineTemplate) + strlen(serializedCommand) +
		    strlen(serializedSegment) + 1));
	sprintf(serializedLine, serializedLineTemplate, serializedCommand,
		serializedSegment);

	assert(string_compare(serializedJson, string_from(serializedLine)) == 0,
	       "The serialized line did not match expected output");
END_TEST}

START_TEST(LayoutLineVector_toJSON_returnsJsonNullForNullInput)
{
	JSONValue *result = LayoutLineVector_toJSON(NULL);
	assert(result != NULL
	       && result->type == JSONValueType_NULL,
	       "Expected JSON null for NULL input");
END_TEST}

START_TEST(LayoutLineVector_toJSON_encodesTheProvidedLines)
{
	LayoutLine line1, line2;
	ASTNode causingCommand1, causingCommand2;
	JSONValue *linesJson;
	string *result, *expected;

	causingCommand1.byteIndex = 1;
	causingCommand1.codepointIndex = 2;
	causingCommand1.tokenIndex = 3;
	causingCommand1.type = ASTNodeType_COMMAND;
	causingCommand1.value = string_from("foo");
	causingCommand1.parent = NULL;
	causingCommand1.children = ASTNodePointerVector_new(0, 0);

	causingCommand2.byteIndex = 4;
	causingCommand2.codepointIndex = 5;
	causingCommand2.tokenIndex = 6;
	causingCommand2.type = ASTNodeType_TEXT;
	causingCommand2.value = string_from("bar");
	causingCommand2.parent = NULL;
	causingCommand2.children = ASTNodePointerVector_new(0, 0);

	line1.causingCommand = &causingCommand1;
	line1.segments = LayoutLineSegmentVector_new(0, 0);

	line2.causingCommand = &causingCommand2;
	line2.segments = LayoutLineSegmentVector_new(0, 0);

	linesJson = LayoutLineVector_toJSON(LayoutLineVector_of2(line1, line2));
	result = JSON_encode(linesJson);
	expected =
	    string_from
	    ("[{\"causingCommand\":{\"byteIndex\":1,\"codepointIndex\":2,\"tokenIndex\":3,\"type\":\"COMMAND\",\"value\":\"foo\",\"children\":[]},\"segments\":[]},{\"causingCommand\":{\"byteIndex\":4,\"codepointIndex\":5,\"tokenIndex\":6,\"type\":\"TEXT\",\"value\":\"bar\",\"children\":[]},\"segments\":[]}]");
	assert(string_compare(result, expected) == 0,
	       "The serialized line vector did not match expected output");
END_TEST}

static void all_tests()
{
	runTest(LayoutLine_toJSON_returnsJsonNullForNullInput);
	runTest(LayoutLine_toJSON_encodesTheProvidedLine);
	runTest(LayoutLineVector_toJSON_returnsJsonNullForNullInput);
	runTest(LayoutLineVector_toJSON_encodesTheProvidedLines);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
