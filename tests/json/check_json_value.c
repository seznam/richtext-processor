#include "../../src/json/json_value.h"
#include "../../src/bool.h"
#include "../unit.h"

/*
   This file does not bother to free heap-allocated memory because it's a suite
   of unit tests, ergo it's not a big deal.
 */

unsigned int tests_run = 0;
unsigned int tests_failed = 0;

static void all_tests(void);

int main(void);

START_TEST(JSONValue_new_createsNewValueOfSpecifiedType)
{
	JSONValue *value = JSONValue_new(JSONValueType_NULL);
	assert(value->type == JSONValueType_NULL,
	       "Expected JSONValue with type JSONValueType_NULL");
	value = JSONValue_new(JSONValueType_NUMBER);
	assert(value->type == JSONValueType_NUMBER,
	       "Expected JSONValue with type JSONValueType_NUMBER");
END_TEST}

START_TEST(JSONValue_newNull_createsJSONValueOfNull)
{
	JSONValue *value = JSONValue_newNull();
	assert(value->type == JSONValueType_NULL,
	       "Expected JSONValue with type JSONValueType_NULL");
END_TEST}

START_TEST(JSONValue_newBoolean_createsJSONValueOfProvidedBoolean)
{
	JSONValue *value = JSONValue_newBoolean(true);
	assert(value->type == JSONValueType_BOOLEAN,
	       "Expected JSONValue with type JSONValueType_BOOLEAN");
	assert(value->value.boolean == true,
	       "Expected created JSONValue's value to be true");

	value = JSONValue_newBoolean(false);
	assert(value->type == JSONValueType_BOOLEAN,
	       "Expected JSONValue with type JSONValueType_BOOLEAN");
	assert(value->value.boolean == false,
	       "Expected created JSONValue's value to be false");
END_TEST}

START_TEST(JSONValue_newNumber_createsJSONValueOfProvidedDouble)
{
	JSONValue *value = JSONValue_newNumber(3.141592653589);	/* PI */
	assert(value->type == JSONValueType_NUMBER,
	       "Expected JSONValue with type JSONValueType_NUMBER");
	assert(value->value.number == 3.141592653589,
	       "Expected created JSONValue's value to be 3.141592653589");

	value = JSONValue_newNumber(100.25);
	assert(value->type == JSONValueType_NUMBER,
	       "Expected JSONValue with type JSONValueType_NUMBER");
	assert(value->value.number == 100.25,
	       "Expected created JSONValue's value to be 100.25");
END_TEST}

START_TEST(JSONValue_newString_rejectsNull)
{
	JSONValue *value = JSONValue_newString(NULL);
	assert(value == NULL, "Expected NULL return value");
END_TEST}

START_TEST(JSONValue_newString_createsJSONValueOfProvidedString)
{
	string *input1 = string_from("first input string");
	string *input2 = string_from("second input string");
	JSONValue *value = JSONValue_newString(input1);
	assert(value->type == JSONValueType_STRING,
	       "Expected JSONValue with type JSONValueType_STRING");
	assert(value->value.string == input1,
	       "Expected created JSONValue's value to be 'first input string'");

	value = JSONValue_newString(input2);
	assert(value->type == JSONValueType_STRING,
	       "Expected JSONValue with type JSONValueType_STRING");
	assert(value->value.string == input2,
	       "Expected created JSONValue's value to be 'second input string'");
END_TEST}

START_TEST(JSONValue_newArray_createsNewJSONValueForArrayOfValues)
{
	JSONValue *value = JSONValue_newArray();
	assert(value->type == JSONValueType_ARRAY,
	       "Expected JSONValue with type JSONValueType_ARRAY");
	assert(value->value.array != NULL,
	       "Expected created JSONValue's value to not be NULL");
END_TEST}

START_TEST(JSONValue_newObject_createsNewJSONValueForObject)
{
	JSONValue *value = JSONValue_newObject();
	assert(value->type == JSONValueType_OBJECT,
	       "Expected JSONValue with type JSONValueType_ARRAY");
	assert(value->value.object != NULL,
	       "Expected created JSONValue's value to not be NULL");
END_TEST}

START_TEST(JSONValue_pushToArray_rejectsNullArray)
{
	JSONValue *value = JSONValue_newNumber(1);
	assert(JSONValue_pushToArray(NULL, value) == NULL,
	       "Expected NULL return value");
END_TEST}

START_TEST(JSONValue_pushToArray_rejectsNullValue)
{
	JSONValue *array = JSONValue_newArray();
	JSONValue *nullValue = JSONValue_newNull();
	assert(JSONValue_pushToArray(array, NULL) == NULL,
	       "Expected NULL return value");
	assert(JSONValue_pushToArray(array, nullValue) == array,
	       "Expected a JSON null to be pushable into a JSON array");
END_TEST}

START_TEST(JSONValue_pushToArray_rejectNonArraysInArrayArgument)
{
	JSONValue *nonArray = JSONValue_newNumber(1);
	JSONValue *value = JSONValue_newBoolean(true);
	assert(JSONValue_pushToArray(nonArray, value) == NULL,
	       "Expected NULL return value");
END_TEST}

START_TEST(JSONValue_pushToArray_addTheValueToTheEnd)
{
	JSONValue *array = JSONValue_newArray();
	JSONValue *value1 = JSONValue_newString(string_from("abc"));
	JSONValue *value2 = JSONValue_newNumber(2);
	JSONValue *value3 = JSONValue_newBoolean(false);
	JSONValuePointerVector *values;
	assert(JSONValue_pushToArray(array, value1) == array,
	       "Expected the array to be returned");
	assert(JSONValue_pushToArray(array, value2) == array,
	       "Expected the array to be returned");
	assert(JSONValue_pushToArray(array, value3) == array,
	       "Expected the array to be returned");
	values = array->value.array;
	assert(values->size.length == 3, "Expected 3 items in the array");
	assert((*values->items)->type == value1->type,
	       "Expected the 1st item to be string 'abc'");
	assert((*values->items)->value.string == value1->value.string,
	       "Expected the 1st item to be string 'abc'");
	assert((*(values->items + 1))->type == value2->type,
	       "Expected the 2nd item to be the number 2");
	assert((*(values->items + 1))->value.number == value2->value.number,
	       "Expected the 2nd item to be the number 2");
	assert((*(values->items + 2))->type == value3->type,
	       "Expected the 3rd item to be the bool false");
	assert((*(values->items + 2))->value.boolean == value3->value.boolean,
	       "Expected the 3rd item to be bool false");
END_TEST}

START_TEST(JSONValue_setObjectProperty_rejectsNullObject)
{
	string *key = string_from("");
	JSONValue *value = JSONValue_newArray();
	assert(JSONValue_setObjectProperty(NULL, key, value) == NULL,
	       "Expected NULL result");
END_TEST}

START_TEST(JSONValue_setObjectProperty_rejectsNullKey)
{
	JSONValue *object = JSONValue_newObject();
	JSONValue *value = JSONValue_newNull();
	assert(JSONValue_setObjectProperty(object, NULL, value) == NULL,
	       "Expected NULL result");
END_TEST}

START_TEST(JSONValue_setObjectProperty_rejectsNonObjectValueForObjectArgument)
{
	JSONValue *array = JSONValue_newArray();
	string *key = string_from("a");
	JSONValue *value = JSONValue_newNumber(1);
	assert(JSONValue_setObjectProperty(array, key, value) == NULL,
	       "Expected NULL result");
END_TEST}

START_TEST(JSONValue_setObjectProperty_rejectsNullValue)
{
	JSONValue *object = JSONValue_newObject();
	string *key = string_from("x");
	assert(JSONValue_setObjectProperty(object, key, NULL) == NULL,
	       "Expected NULL result");
END_TEST}

START_TEST(JSONValue_setObjectProperty_addsNewProperties)
{
	JSONValue *object = JSONValue_newObject();
	string *key1 = string_from("x");
	JSONValue *value1 = JSONValue_newNull();
	string *key2 = string_from("y");
	JSONValue *value2 = JSONValue_newNumber(15.51);
	string *key3 = string_from("z");
	JSONValue *value3 = JSONValue_newBoolean(true);
	JSONObjectPropertyVector *properties;
	JSONObjectProperty *property;

	assert(JSONValue_setObjectProperty(object, key1, value1) == object,
	       "Expected the object be returned on success");
	assert(JSONValue_setObjectProperty(object, key2, value2) == object,
	       "Expected the object be returned on success");
	assert(JSONValue_setObjectProperty(object, key3, value3) == object,
	       "Expected the object be returned on success");
	properties = object->value.object;
	assert(properties->size.length == 3,
	       "Expected the object to have 3 properties");
	property = properties->items;
	assert(property->key == key1,
	       "Expected the 1st property key to match the 1st added key");
	assert(property->value == value1,
	       "Expected the 1st property value to match the 1st added value");
	assert((property + 1)->key == key2,
	       "Expected the 2nd property key to match the 2nd added key");
	assert((property + 1)->value == value2,
	       "Expected the 2nd property value to match the 2nd added value");
	assert((property + 2)->key == key3,
	       "Expected the 3rd property key to match the 3rd added key");
	assert((property + 2)->value == value3,
	       "Expected the 3rd property value to match the 3rd added value");
END_TEST}

START_TEST(JSONValue_setObjectProperty_reconfiguresExistingProperties)
{
	JSONValue *object = JSONValue_newObject();
	string *key1 = string_from("x");
	JSONValue *value1 = JSONValue_newNull();
	string *key2 = string_from("y");
	JSONValue *value2 = JSONValue_newNumber(15.51);
	string *key3 = string_from("z");
	JSONValue *value3 = JSONValue_newBoolean(true);
	JSONValue *value4 = JSONValue_newString(string_from("foobar"));
	JSONObjectPropertyVector *properties;
	JSONObjectProperty *property;

	assert(JSONValue_setObjectProperty(object, key1, value1) == object,
	       "Expected the object be returned on success");
	assert(JSONValue_setObjectProperty(object, key2, value2) == object,
	       "Expected the object be returned on success");
	assert(JSONValue_setObjectProperty(object, key3, value3) == object,
	       "Expected the object be returned on success");
	properties = object->value.object;
	property = properties->items;
	assert(properties->size.length == 3,
	       "Expected the object to have 3 properties");
	assert((property + 1)->key == key2,
	       "Expected the 2nd property key to match the 2nd added key");
	assert((property + 1)->value == value2,
	       "Expected the 2nd property value to match the 2nd added value");

	assert(JSONValue_setObjectProperty(object, key2, value4) == object,
	       "Expected the object be returned on success");
	assert(property->key == key1,
	       "Expected the 1st property key to match the 1st added key");
	assert(property->value == value1,
	       "Expected the 1st property value to match the 1st added value");
	assert((property + 1)->key == key2,
	       "Expected the 2nd property key to match the 2nd added key");
	assert((property + 1)->value == value4,
	       "Expected the 2nd property value to match the newly set value");
	assert((property + 2)->key == key3,
	       "Expected the 3rd property key to match the 3rd added key");
	assert((property + 2)->value == value3,
	       "Expected the 3rd property value to match the 3rd added value");
END_TEST}

START_TEST(JSONValue_deleteObjectProperty_rejectsNullObject)
{
	string *key = string_from("x");
	assert(JSONValue_deleteObjectProperty(NULL, key) == NULL,
	       "Expected NULL result");
END_TEST}

START_TEST(JSONValue_deleteObjectProperty_rejectsNullKey)
{
	JSONValue *object = JSONValue_newObject();
	assert(JSONValue_deleteObjectProperty(object, NULL) == NULL,
	       "Expected NULL result");
END_TEST}

START_TEST(JSONValue_deleteObjectProperty_rejectsNonObjectInObjectArgument)
{
	JSONValue *number = JSONValue_newNumber(23);
	string *key = string_from("z");
	assert(JSONValue_deleteObjectProperty(number, key) == NULL,
	       "Expected NULL result");
END_TEST}

START_TEST(JSONValue_deleteObjectProperty_removesTheSpecifiedProperty)
{
	JSONValue *object = JSONValue_newObject();
	string *key1 = string_from("x");
	JSONValue *value1 = JSONValue_newNull();
	string *key2 = string_from("y");
	JSONValue *value2 = JSONValue_newNumber(15.51);
	string *key3 = string_from("z");
	JSONValue *value3 = JSONValue_newBoolean(true);
	JSONObjectPropertyVector *properties;
	JSONObjectProperty *property;

	assert(JSONValue_setObjectProperty(object, key1, value1) == object,
	       "Expected the object be returned on success");
	assert(JSONValue_setObjectProperty(object, key2, value2) == object,
	       "Expected the object be returned on success");
	assert(JSONValue_setObjectProperty(object, key3, value3) == object,
	       "Expected the object be returned on success");
	properties = object->value.object;
	assert(properties->size.length == 3,
	       "Expected the object to have 3 properties");

	assert(JSONValue_deleteObjectProperty(object, string_from("y")) ==
	       object, "Expected the object be returned on success");

	assert(properties->size.length == 2,
	       "Expected the object to have 2 properties");
	property = properties->items;
	assert(property->key == key1,
	       "Expected the 1st property key to match the 1st added key");
	assert(property->value == value1,
	       "Expected the 1st property value to match the 1st added value");
	assert((property + 1)->key == key3,
	       "Expected the 2nd property key to match the 3rd added key");
	assert((property + 1)->value == value3,
	       "Expected the 2nd property value to match the 3rd added value");
END_TEST}

START_TEST(JSONValue_deleteObjectProperty_hasNoEffectForKeyNotInObject)
{
	JSONValue *object = JSONValue_newObject();
	string *key1 = string_from("x");
	JSONValue *value1 = JSONValue_newNull();
	string *key2 = string_from("y");
	JSONValue *value2 = JSONValue_newNumber(15.51);
	JSONObjectPropertyVector *properties;
	JSONObjectProperty *property;

	assert(JSONValue_setObjectProperty(object, key1, value1) == object,
	       "Expected the object be returned on success");
	assert(JSONValue_setObjectProperty(object, key2, value2) == object,
	       "Expected the object be returned on success");
	assert(JSONValue_deleteObjectProperty(object, string_from("z")) ==
	       object,
	       "Expected the object be returned for not found keys as well");

	properties = object->value.object;
	assert(properties->size.length == 2,
	       "Expected the object to have 2 properties");
	property = properties->items;
	assert(property->key == key1,
	       "Expected the 1st property key to match the 1st added key");
	assert(property->value == value1,
	       "Expected the 1st property value to match the 1st added value");
	assert((property + 1)->key == key2,
	       "Expected the 2nd property key to match the 2nd added key");
	assert((property + 1)->value == value2,
	       "Expected the 2nd property value to match the 2nd added value");
END_TEST}

START_TEST(JSONValue_free_acceptsNull)
{
	JSONValue_free(NULL);
END_TEST}

START_TEST(JSONValue_free_freesPrimitives)
{
	JSONValue_free(JSONValue_newNull());
	JSONValue_free(JSONValue_newBoolean(true));
	JSONValue_free(JSONValue_newNumber(1));
	JSONValue_free(JSONValue_newString(string_from("a")));
END_TEST}

START_TEST(JSONValue_free_freesObjectsAndArrays)
{
	JSONValue_free(JSONValue_newArray());
	JSONValue_free(JSONValue_newObject());
END_TEST}

START_TEST(JSONValue_freeRecursive_acceptsNull)
{
	JSONValue_freeRecursive(NULL);
END_TEST}

START_TEST(JSONValue_freeRecursive_freesNestedObjectsAndArrays)
{
	JSONValue *obj1 = JSONValue_newObject();
	JSONValue *obj2 = JSONValue_newObject();
	JSONValue *arr = JSONValue_newArray();

	JSONValue_setObjectProperty(obj1, string_from("x"), arr);
	JSONValue_pushToArray(arr, obj2);

	JSONValue_freeRecursive(obj1);
END_TEST}

static void all_tests()
{
	runTest(JSONValue_new_createsNewValueOfSpecifiedType);
	runTest(JSONValue_newNull_createsJSONValueOfNull);
	runTest(JSONValue_newBoolean_createsJSONValueOfProvidedBoolean);
	runTest(JSONValue_newNumber_createsJSONValueOfProvidedDouble);
	runTest(JSONValue_newString_rejectsNull);
	runTest(JSONValue_newString_createsJSONValueOfProvidedString);
	runTest(JSONValue_newArray_createsNewJSONValueForArrayOfValues);
	runTest(JSONValue_newObject_createsNewJSONValueForObject);
	runTest(JSONValue_pushToArray_rejectsNullArray);
	runTest(JSONValue_pushToArray_rejectsNullValue);
	runTest(JSONValue_pushToArray_rejectNonArraysInArrayArgument);
	runTest(JSONValue_pushToArray_addTheValueToTheEnd);
	runTest(JSONValue_setObjectProperty_rejectsNullObject);
	runTest(JSONValue_setObjectProperty_rejectsNullKey);
	runTest
	    (JSONValue_setObjectProperty_rejectsNonObjectValueForObjectArgument);
	runTest(JSONValue_setObjectProperty_rejectsNullValue);
	runTest(JSONValue_setObjectProperty_addsNewProperties);
	runTest(JSONValue_setObjectProperty_reconfiguresExistingProperties);
	runTest(JSONValue_deleteObjectProperty_rejectsNullObject);
	runTest(JSONValue_deleteObjectProperty_rejectsNullKey);
	runTest
	    (JSONValue_deleteObjectProperty_rejectsNonObjectInObjectArgument);
	runTest(JSONValue_deleteObjectProperty_removesTheSpecifiedProperty);
	runTest(JSONValue_deleteObjectProperty_hasNoEffectForKeyNotInObject);
	runTest(JSONValue_free_acceptsNull);
	runTest(JSONValue_free_freesPrimitives);
	runTest(JSONValue_free_freesObjectsAndArrays);
	runTest(JSONValue_freeRecursive_acceptsNull);
	runTest(JSONValue_freeRecursive_freesNestedObjectsAndArrays);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
