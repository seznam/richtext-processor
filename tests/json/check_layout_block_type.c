#include "../../src/json/json_value.h"
#include "../../src/json/layout_block_type.h"
#include "../../src/layout_block_type.h"
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

START_TEST(LayoutBlockType_toJSON_encodesTheProvidedTypeToJsonString)
{
	JSONValue *result;
	result = LayoutBlockType_toJSON(LayoutBlockType_HEADING);
	assert(result != NULL && result->type == JSONValueType_STRING
	       && string_compare(result->value.string,
				 string_from("HEADING")) == 0,
	       "Expected the \"HEADING\" JSON string");
	result = LayoutBlockType_toJSON(LayoutBlockType_FOOTING);
	assert(result != NULL && result->type == JSONValueType_STRING
	       && string_compare(result->value.string,
				 string_from("FOOTING")) == 0,
	       "Expected the \"FOOTING\" JSON string");
	result = LayoutBlockType_toJSON(LayoutBlockType_MAIN_CONTENT);
	assert(result != NULL && result->type == JSONValueType_STRING
	       && string_compare(result->value.string,
				 string_from("MAIN_CONTENT")) == 0,
	       "Expected the \"MAIN_CONTENT\" JSON string");
	result = LayoutBlockType_toJSON(LayoutBlockType_PAGE_BREAK);
	assert(result != NULL && result->type == JSONValueType_STRING
	       && string_compare(result->value.string,
				 string_from("PAGE_BREAK")) == 0,
	       "Expected the \"PAGE_BREAK\" JSON string");
	result = LayoutBlockType_toJSON(LayoutBlockType_SAME_PAGE_START);
	assert(result != NULL && result->type == JSONValueType_STRING
	       && string_compare(result->value.string,
				 string_from("SAME_PAGE_START")) == 0,
	       "Expected the \"SAME_PAGE_START\" JSON string");
	result = LayoutBlockType_toJSON(LayoutBlockType_SAME_PAGE_END);
	assert(result != NULL && result->type == JSONValueType_STRING
	       && string_compare(result->value.string,
				 string_from("SAME_PAGE_END")) == 0,
	       "Expected the \"SAME_PAGE_END\" JSON string");
	result = LayoutBlockType_toJSON(LayoutBlockType_CUSTOM);
	assert(result != NULL && result->type == JSONValueType_STRING
	       && string_compare(result->value.string,
				 string_from("CUSTOM")) == 0,
	       "Expected the \"CUSTOM\" JSON string");
END_TEST}

START_TEST(LayoutBlockType_toJSON_returnNullForInvalidInput)
{
	JSONValue *result;
	result = LayoutBlockType_toJSON(65535);
	assert(result == NULL, "Expected NULL result for invalid input");
END_TEST}

static void all_tests()
{
	runTest(LayoutBlockType_toJSON_encodesTheProvidedTypeToJsonString);
	runTest(LayoutBlockType_toJSON_returnNullForInvalidInput);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
