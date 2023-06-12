#include "../../src/json/layout_content_alignment.h"
#include "../../src/layout_resolver.h"
#include "../../src/json/json_encoder.h"
#include "../unit.h"

/*
   This file does not bother to free heap-allocated memory because it's a suite
   of unit tests, ergo it's not a big deal.
 */

unsigned int tests_run = 0;
unsigned int tests_failed = 0;

static void all_tests(void);

int main(void);

START_TEST(LayoutContentAlignment_toJSON_returnsNullForInvalidValue)
{
	JSONValue *result;
	result = LayoutContentAlignment_toJSON(32535);
	assert(result == NULL, "Expected NULL for invalid input value");
END_TEST}

START_TEST(LayoutContentAlignment_toJSON_encodesAlignmentToJsonString)
{
	JSONValue *result;
	JSONValue *defaultVal = JSONValue_newString(string_from("DEFAULT"));
	JSONValue *justifyLeftVal =
	    JSONValue_newString(string_from("JUSTIFY_LEFT"));
	JSONValue *justifyRightVal =
	    JSONValue_newString(string_from("JUSTIFY_RIGHT"));
	JSONValue *centerVal = JSONValue_newString(string_from("CENTER"));

	result = LayoutContentAlignment_toJSON(LayoutContentAlignment_DEFAULT);
	assert(string_compare(JSON_encode(result), JSON_encode(defaultVal)) ==
	       0, "Expected the \"DEFAULT\" string");
	result =
	    LayoutContentAlignment_toJSON(LayoutContentAlignment_JUSTIFY_LEFT);
	assert(string_compare(JSON_encode(result), JSON_encode(justifyLeftVal))
	       == 0, "Expected the \"JUSTIFY_LEFT\" string");
	result =
	    LayoutContentAlignment_toJSON(LayoutContentAlignment_JUSTIFY_RIGHT);
	assert(string_compare(JSON_encode(result), JSON_encode(justifyRightVal))
	       == 0, "Expected the \"JUSTIFY_RIGHT\" string");
	result = LayoutContentAlignment_toJSON(LayoutContentAlignment_CENTER);
	assert(string_compare(JSON_encode(result), JSON_encode(centerVal)) == 0,
	       "Expected the \"CENTER\" string");
END_TEST}

static void all_tests()
{
	runTest(LayoutContentAlignment_toJSON_returnsNullForInvalidValue);
	runTest(LayoutContentAlignment_toJSON_encodesAlignmentToJsonString);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
