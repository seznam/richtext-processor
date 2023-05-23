#include "../../src/json/json_encoder.h"
#include "../../src/json/json_value.h"
#include "../../src/bool.h"
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

START_TEST(JSON_encode_returnNullForNullInput)
{
	assert(JSON_encode(NULL) == NULL, "Expected NULL result");
END_TEST}

START_TEST(JSON_encode_encodesJsonNull)
{
	assert(string_compare
	       (JSON_encode(JSONValue_newNull()), string_from("null")) == 0,
	       "Expected the result to be the \"null\" string");
END_TEST}

START_TEST(JSON_encode_encodesFalse)
{
	assert(string_compare
	       (JSON_encode(JSONValue_newBoolean(false)), string_from("false"))
	       == 0, "Expected the result to be the \"false\" string");
END_TEST}

START_TEST(JSON_encode_encodesTrue)
{
	assert(string_compare
	       (JSON_encode(JSONValue_newBoolean(true)), string_from("true"))
	       == 0, "Expected the result to be the \"true\" string");
END_TEST}

#define assert_json(jsonValue, expectedString, errorMessage)\
assert(string_compare(JSON_encode(jsonValue), string_from(expectedString)) \
       == 0, errorMessage)

START_TEST(JSON_encode_encodesIntegers)
{
	assert_json(JSONValue_newNumber(0), "0",
		    "Expected the result to be the \"0\" string");
	assert_json(JSONValue_newNumber(-11), "-11",
		    "Expected the result to be the \"-11\" string");
	/* Maximum and minimum safe integers */
	assert_json(JSONValue_newNumber(9007199254740991), "9007199254740991",
		    "Expected the result to be the \"9007199254740991\" string");
	assert_json(JSONValue_newNumber(-9007199254740991), "-9007199254740991",
		    "Expected the result to be the \"-9007199254740991\" string");
END_TEST}

START_TEST(JSON_encode_encodesFloats)
{
	assert_json(JSONValue_newNumber(-11.73), "-11.73",
		    "Expected the result to be the \"-11.73\" string");
	assert_json(JSONValue_newNumber(1357.95), "1357.95",
		    "Expected the result to be the \"1357.95\" string");
	/* Large integers */
	assert_json(JSONValue_newNumber(9007199254740992), "9.0072e+15",
		    "Expected the result to be the \"9.0072e+15\" string");
	assert_json(JSONValue_newNumber(-9007199254740992), "-9.0072e+15",
		    "Expected the result to be the \"-9.0072e+15\" string");
END_TEST}

START_TEST(JSON_encode_encodesStrings)
{
	string *sourceString = string_new(3);
	assert_json(JSONValue_newString(string_from("")), "\"\"",
		    "Expected the result to be the '\"\"' string");
	assert_json(JSONValue_newString(string_from("abc def")), "\"abc def\"",
		    "Expected the result to be the '\"abc def\"' string");
	*sourceString->content = '1';
	*(sourceString->content + 1) = 0;
	*(sourceString->content + 2) = '!';
	assert(string_compare
	       (JSON_encode(JSONValue_newString(sourceString)),
		string_from("\"1\\u0000!\"")) == 0,
	       "Expected the result to be the '\"1\\u0000!\"' string");
	assert_json(JSONValue_newString(string_from("\n\t")),
		    "\"\\u000a\\u0009\"",
		    "Expected the result to be the '\"\\u000a\\u0008\"' string");
END_TEST}

START_TEST(JSON_encode_encodesHeterogenousArrays)
{
	JSONValue *arr = JSONValue_newArray();
	JSONValue_pushToArray(arr, JSONValue_newNumber(1));
	JSONValue_pushToArray(arr, JSONValue_newBoolean(false));
	JSONValue_pushToArray(arr, JSONValue_newString(string_from("abc")));
	JSONValue_pushToArray(arr, JSONValue_newNull());
	assert_json(arr, "[1,false,\"abc\",null]",
		    "Expected the result to be the '[1,false,\"abc\",null]' string");
END_TEST}

START_TEST(JSON_encode_encodesArraysOfArrays)
{
	JSONValue *topArray = JSONValue_newArray();
	JSONValue *subArr1 = JSONValue_newArray();
	JSONValue *subArr2 = JSONValue_newArray();
	JSONValue *subArr3 = JSONValue_newArray();
	JSONValue_pushToArray(topArray, subArr1);
	JSONValue_pushToArray(topArray, subArr2);
	JSONValue_pushToArray(subArr1, JSONValue_newBoolean(true));
	JSONValue_pushToArray(subArr2, JSONValue_newNumber(1));
	JSONValue_pushToArray(subArr2, subArr3);
	JSONValue_pushToArray(subArr3, JSONValue_newNull());
	assert_json(topArray, "[[true],[1,[null]]]",
		    "Expected the result to be the \"[[true],[1,[null]]]\" string");
END_TEST}

START_TEST(JSON_encode_encodesObjectOfVariousPropertyValues)
{
	JSONValue *obj = JSONValue_newObject();
	JSONValue_setObjectProperty(obj, string_from("x"),
				    JSONValue_newNumber(1));
	JSONValue_setObjectProperty(obj, string_from("y"),
				    JSONValue_newBoolean(true));
	JSONValue_setObjectProperty(obj, string_from("z"), JSONValue_newNull());
	assert_json(obj, "{\"x\":1,\"y\":true,\"z\":null}",
		    "Expected the result to be the '{\"x\":1,\"y\":true,\"z\":null}' string");
END_TEST}

START_TEST(JSON_encode_encodesObjectContainingObjectContainingArray)
{
	JSONValue *topObject = JSONValue_newObject();
	JSONValue *subObject = JSONValue_newObject();
	JSONValue *arr = JSONValue_newArray();

	JSONValue_setObjectProperty(topObject, string_from("x"), subObject);
	JSONValue_setObjectProperty(topObject, string_from("y"),
				    JSONValue_newNumber(11));
	JSONValue_setObjectProperty(subObject, string_from("y"), arr);
	JSONValue_pushToArray(arr, JSONValue_newBoolean(true));

	assert_json(topObject, "{\"x\":{\"y\":[true]},\"y\":11}",
		    "Expected the result to be the '{\"x\":{\"y\":[true]},\"y\":11}' string");
END_TEST}

#undef assert_json

static void all_tests()
{
	runTest(JSON_encode_returnNullForNullInput);
	runTest(JSON_encode_encodesJsonNull);
	runTest(JSON_encode_encodesFalse);
	runTest(JSON_encode_encodesTrue);
	runTest(JSON_encode_encodesIntegers);
	runTest(JSON_encode_encodesFloats);
	runTest(JSON_encode_encodesStrings);
	runTest(JSON_encode_encodesHeterogenousArrays);
	runTest(JSON_encode_encodesArraysOfArrays);
	runTest(JSON_encode_encodesObjectOfVariousPropertyValues);
	runTest(JSON_encode_encodesObjectContainingObjectContainingArray);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
