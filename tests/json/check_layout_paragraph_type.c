#include "../../src/json/json_encoder.h"
#include "../../src/json/json_value.h"
#include "../../src/json/layout_paragraph_type.h"
#include "../../src/layout_resolver.h"
#include "../../src/string.h"
#include "../unit.h"

/*
   This file does not bother to free heap-allocated memory because it's a suite
   of unit tests, ergo it's not a big deal.
 */

unsigned int tests_run = 0;
unsigned int tests_failed = 0;

static void all_tests(void);

START_TEST(LayoutParagraphType_toJSON_returnsNullForInvalidInput)
{
	JSONValue *result;
	result = LayoutParagraphType_toJSON(65535);
	assert(result == NULL, "Expected NULL result");
END_TEST}

START_TEST(LayoutParagraphType_toJSON_encodesInputToJsonString)
{
	JSONValue *json;
	string *result;
	json = LayoutParagraphType_toJSON(LayoutParagraphType_EXPLICIT);
	result = JSON_encode(json);
	assert(string_compare(result, string_from("\"EXPLICIT\"")) == 0,
	       "Expected the \"EXPLICIT\" JSON string");
	json = LayoutParagraphType_toJSON(LayoutParagraphType_IMPLICIT);
	result = JSON_encode(json);
	assert(string_compare(result, string_from("\"IMPLICIT\"")) == 0,
	       "Expected the \"IMPLICIT\" JSON string");
END_TEST}

int main(void);

static void all_tests()
{
	runTest(LayoutParagraphType_toJSON_returnsNullForInvalidInput);
	runTest(LayoutParagraphType_toJSON_encodesInputToJsonString);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
