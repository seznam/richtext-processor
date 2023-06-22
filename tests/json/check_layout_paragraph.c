#include "../../src/json/json_encoder.h"
#include "../../src/json/json_value.h"
#include "../../src/json/layout_paragraph.h"
#include "../../src/json/layout_paragraph_type.h"
#include "../../src/ast_node.h"
#include "../../src/ast_node_pointer_vector.h"
#include "../../src/ast_node_type.h"
#include "../../src/layout_line.h"
#include "../../src/layout_line_segment_vector.h"
#include "../../src/layout_paragraph.h"
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

START_TEST(LayoutParagraph_toJSON_returnsJsonNullForNullInput)
{
	JSONValue *result;
	result = LayoutParagraph_toJSON(NULL);
	assert(result != NULL
	       && result->type == JSONValueType_NULL,
	       "Expected JSON null result");
END_TEST}

START_TEST(LayoutParagraph_toJSON_encodedProvidedParagraphToJson)
{
	JSONValue *asJson;
	string *encoded, *expectedString;
	ASTNode cause;
	LayoutLine line;
	LayoutParagraph paragraph;
	char *expectedCause;
	char *expectedParagraphTemplate, *expectedParagraph;

	cause.byteIndex = 1;
	cause.codepointIndex = 2;
	cause.tokenIndex = 3;
	cause.type = ASTNodeType_COMMAND;
	cause.value = string_from("foo");
	cause.parent = NULL;
	cause.children = ASTNodePointerVector_new(0, 0);

	line.causingCommand = &cause;
	line.segments = LayoutLineSegmentVector_new(0, 0);

	paragraph.causingCommand = &cause;
	paragraph.type = LayoutParagraphType_EXPLICIT;
	paragraph.lines = LayoutLineVector_of1(line);

	asJson = LayoutParagraph_toJSON(&paragraph);
	encoded = JSON_encode(asJson);

	expectedCause =
	    "{\"byteIndex\":1,\"codepointIndex\":2,\"tokenIndex\":3,\"type\":\"COMMAND\",\"value\":\"foo\",\"children\":[]}";
	expectedParagraphTemplate =
	    "{\"causingCommand\":%s,\"type\":\"EXPLICIT\",\"lines\":[{\"causingCommand\":%s,\"segments\":[]}]}";
	expectedParagraph =
	    malloc(sizeof(char) *
		   (strlen(expectedParagraphTemplate) + strlen(expectedCause) +
		    strlen(expectedCause) + 1));
	sprintf(expectedParagraph, expectedParagraphTemplate, expectedCause,
		expectedCause);
	expectedString = string_from(expectedParagraph);

	assert(string_compare(encoded, expectedString) == 0,
	       "The JSON output did not match expectations");
END_TEST}

START_TEST(LayoutParagraphVector_toJSON_returnsJsonNullForNullInput)
{
	JSONValue *asJson = LayoutParagraphVector_toJSON(NULL);
	assert(asJson != NULL
	       && asJson->type == JSONValueType_NULL,
	       "Expected JSON null as result");
END_TEST}

START_TEST(LayoutParagraphVector_toJSON_encodesAnEmptyVectorToEmptyArray)
{
	JSONValue *asJson;
	string *encoded;
	asJson = LayoutParagraphVector_toJSON(LayoutParagraphVector_new(0, 0));
	encoded = JSON_encode(asJson);
	assert(string_compare(encoded, string_from("[]")) == 0,
	       "The resulting JSON did not match expectations");
END_TEST}

START_TEST(LayoutParagraph_toJSON_encodesAnArrayOfParagraphs)
{
	JSONValue *asJson;
	string *encoded, *expected;
	LayoutParagraph paragraph1, paragraph2;
	LayoutParagraphVector *paragraphs;

	paragraph1.causingCommand = NULL;
	paragraph1.type = LayoutParagraphType_EXPLICIT;
	paragraph1.lines = LayoutLineVector_new(0, 0);

	paragraph2.causingCommand = NULL;
	paragraph2.type = LayoutParagraphType_IMPLICIT;
	paragraph2.lines = LayoutLineVector_new(0, 0);

	paragraphs = LayoutParagraphVector_of2(paragraph1, paragraph2);
	asJson = LayoutParagraphVector_toJSON(paragraphs);
	encoded = JSON_encode(asJson);

	expected =
	    string_from
	    ("[{\"causingCommand\":null,\"type\":\"EXPLICIT\",\"lines\":[]},{\"causingCommand\":null,\"type\":\"IMPLICIT\",\"lines\":[]}]");
	assert(string_compare(encoded, expected) == 0,
	       "The generated JSON output did not match expectations");
END_TEST}

static void all_tests()
{
	runTest(LayoutParagraph_toJSON_returnsJsonNullForNullInput);
	runTest(LayoutParagraph_toJSON_encodedProvidedParagraphToJson);
	runTest(LayoutParagraphVector_toJSON_returnsJsonNullForNullInput);
	runTest(LayoutParagraphVector_toJSON_encodesAnEmptyVectorToEmptyArray);
	runTest(LayoutParagraph_toJSON_encodesAnArrayOfParagraphs);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
