#include <stdlib.h>
#include "../typed_vector.h"
#include "json_value.h"

JSONValue *JSONValue_new(type)
JSONValueType type;
{
	JSONValue *value = malloc(sizeof(JSONValue));
	if (value == NULL) {
		return NULL;
	}

	value->type = type;
	return value;
}

JSONValue *JSONValue_newNull()
{
	return JSONValue_new(JSONValueType_NULL);
}

JSONValue *JSONValue_newBoolean(value)
bool value;
{
	JSONValue *jsonValue = JSONValue_new(JSONValueType_BOOLEAN);
	if (jsonValue != NULL) {
		jsonValue->value.boolean = value;
	}
	return jsonValue;
}

JSONValue *JSONValue_newNumber(value)
double value;
{
	JSONValue *jsonValue = JSONValue_new(JSONValueType_NUMBER);
	if (jsonValue != NULL) {
		jsonValue->value.number = value;
	}
	return jsonValue;
}

JSONValue *JSONValue_newString(value)
string *value;
{
	JSONValue *jsonValue;

	if (value == NULL) {
		return NULL;
	}

	jsonValue = JSONValue_new(JSONValueType_STRING);
	if (jsonValue != NULL) {
		jsonValue->value.string = value;
	}
	return jsonValue;
}

JSONValue *JSONValue_newArray()
{
	JSONValue *jsonValue = JSONValue_new(JSONValueType_ARRAY);
	if (jsonValue != NULL) {
		jsonValue->value.array = JSONValuePointerVector_new(0, 0);
		if (jsonValue->value.array == NULL) {
			JSONValue_free(jsonValue);
			return NULL;
		}
	}
	return jsonValue;
}

JSONValue *JSONValue_newObject()
{
	JSONValue *jsonValue = JSONValue_new(JSONValueType_OBJECT);
	if (jsonValue != NULL) {
		jsonValue->value.object = JSONObjectPropertyVector_new(0, 0);
		if (jsonValue->value.object == NULL) {
			JSONValue_free(jsonValue);
			return NULL;
		}
	}
	return jsonValue;
}

JSONValue *JSONValue_pushToArray(array, value)
JSONValue *array;
JSONValue *value;
{
	JSONValuePointerVector *grownArray;
	if (array == NULL || value == NULL) {
		return NULL;
	}
	if (array->type != JSONValueType_ARRAY) {
		return NULL;
	}

	grownArray = JSONValuePointerVector_append(array->value.array, &value);
	if (grownArray == NULL) {
		return NULL;
	}

	array->value.array = grownArray;
	return array;
}

JSONValue *JSONValue_setObjectProperty(object, key, value)
JSONValue *object;
string *key;
JSONValue *value;
{
	JSONObjectProperty property;
	JSONObjectPropertyVector *properties;
	JSONObjectPropertyVector *success;
	unsigned long i;

	if (object == NULL || key == NULL || value == NULL) {
		return NULL;
	}
	if (object->type != JSONValueType_OBJECT) {
		return NULL;
	}

	property.key = key;
	property.value = value;

	properties = object->value.object;
	for (i = 0; i < properties->size.length; i++) {
		JSONObjectProperty existingProperty;
		success =
		    JSONObjectPropertyVector_get(properties, i,
						 &existingProperty);
		if (success == NULL) {
			return NULL;
		}
		if (string_compare(key, existingProperty.key) != 0) {
			continue;
		}

		existingProperty.value = value;
		success =
		    JSONObjectPropertyVector_set(properties, i,
						 &existingProperty);
		return success != NULL ? object : NULL;
	}

	success = JSONObjectPropertyVector_append(properties, &property);
	if (success == NULL) {
		return NULL;
	}

	object->value.object = success;
	return object;
}

JSONValue *JSONValue_deleteObjectProperty(object, key)
JSONValue *object;
string *key;
{
	unsigned long i;
	unsigned long j;
	JSONObjectPropertyVector *properties;

	if (object == NULL || key == NULL) {
		return NULL;
	}
	if (object->type != JSONValueType_OBJECT) {
		return NULL;
	}

	properties = object->value.object;
	for (i = 0; i < properties->size.length; i++) {
		JSONObjectPropertyVector *success;
		JSONObjectProperty property;
		success =
		    JSONObjectPropertyVector_get(properties, i, &property);
		if (success == NULL) {
			return NULL;
		}
		if (string_compare(key, property.key) != 0) {
			continue;
		}

		/* Shift the following properties by 1 position towards start */
		for (j = i + 1; j < properties->size.length; j++) {
			success =
			    JSONObjectPropertyVector_get(properties, j,
							 &property);
			if (success == NULL) {
				return NULL;
			}
			success =
			    JSONObjectPropertyVector_set(properties, j - 1,
							 &property);
			if (success == NULL) {
				return NULL;
			}
		}
		success = JSONObjectPropertyVector_pop(properties, &property);
		if (success == NULL) {
			return NULL;
		}
		break;
	}

	return object;
}

void JSONValue_free(value)
JSONValue *value;
{
	if (value == NULL) {
		return;
	}

	switch (value->type) {
	case JSONValueType_NULL:
	case JSONValueType_BOOLEAN:
	case JSONValueType_NUMBER:
	case JSONValueType_STRING:
		/* nothing to do */
		break;

	case JSONValueType_ARRAY:
		JSONValuePointerVector_free(value->value.array);
		value->value.array = NULL;
		break;

	case JSONValueType_OBJECT:
		JSONObjectPropertyVector_free(value->value.object);
		value->value.object = NULL;
		break;

	default:
		/* invalid type, ignore */
		break;
	}

	free(value);
}

Vector_ofPointerImplementation(JSONValue)
    Vector_ofTypeImplementation(JSONObjectProperty)
