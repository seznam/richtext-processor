#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../string.h"
#include "../typed_pointer_vector.h"
#include "json_encoder.h"

static const char *STRINGIFIED_NULL_VALUE = "null";
static const char *STRINGIFIED_FALSE_VALUE = "false";
static const char *STRINGIFIED_TRUE_VALUE = "true";

/* -pow(2, 53) + 1 */
static const double MIN_SAFE_INTEGER = -9007199254740991;
/* pow(2, 53) - 1 */
static const double MAX_SAFE_INTEGER = 9007199254740991;

Vector_ofPointer(string)
static string *encodeString(JSONValue *value);

static string *encodeArray(JSONValue *value);

static string *encodeObject(JSONValue *value);

static string *createCompositeValueResult(stringPointerVector *items,
					  string *prefix, string *separator,
					  string *suffix);

static unsigned long getJoinedStringLength(stringPointerVector *strings,
					   unsigned long separatorLength);

static void joinStrings(unsigned char *destination,
			stringPointerVector *strings, string *separator);

static void freeStrings(stringPointerVector *strings);

string *JSON_encode(value)
JSONValue *value;
{
	string *result = NULL;
	double number = 0;
	char *stringifiedNumber = NULL;

	if (value == NULL) {
		return NULL;
	}

	switch (value->type) {
	case JSONValueType_NULL:
		return string_from(STRINGIFIED_NULL_VALUE);

	case JSONValueType_BOOLEAN:
		return string_from(value->value.boolean ?
				   STRINGIFIED_TRUE_VALUE :
				   STRINGIFIED_FALSE_VALUE);

	case JSONValueType_NUMBER:
		stringifiedNumber = malloc(sizeof(char) * 21);
		if (stringifiedNumber == NULL) {
			return NULL;
		}

		number = value->value.number;
		if ((long)number == number && number >= MIN_SAFE_INTEGER
		    && number <= MAX_SAFE_INTEGER) {
			if (sprintf(stringifiedNumber, "%.0f", number) < 0) {
				free(stringifiedNumber);
				return NULL;
			}
		} else {
			/*
			 * This does not preserve as many decimal places as we
			 * would like, however, floats are not used in the
			 * structures we are going to encode, so this is not
			 * really an issue.
			 */
			if (sprintf(stringifiedNumber, "%g", number) < 0) {
				free(stringifiedNumber);
				return NULL;
			}
		}

		result = string_from(stringifiedNumber);
		free(stringifiedNumber);
		return result;

	case JSONValueType_STRING:
		return encodeString(value);

	case JSONValueType_ARRAY:
		return encodeArray(value);

	case JSONValueType_OBJECT:
		return encodeObject(value);

	default:
		return NULL;
	}
}

static string *encodeString(value)
JSONValue *value;
{
	string *stringValue = NULL;
	unsigned long length = 0;
	unsigned char *currentChar = NULL;
	unsigned long i = 0;
	string *result = NULL;
	unsigned char *currentOutputChar = NULL;
	char *codepointHexDigits = NULL;
	char codepoint = 0;

	if (value == NULL || value->type != JSONValueType_STRING) {
		return NULL;
	}

	length = 2;		/* starting and ending quotes */
	stringValue = value->value.string;
	currentChar = stringValue->content;
	for (i = 0; i < stringValue->length; i++, currentChar++) {
		if (*currentChar == '"' || *currentChar == '/') {
			if (ULONG_MAX - length < 2) {
				return NULL;
			}
			length += 2;
		} else if (*currentChar <= 31) {
			if (ULONG_MAX - length < 6) {
				return NULL;
			}
			length += 6;	/* \uXXXX */
		} else {
			if (ULONG_MAX - length < 1) {
				return NULL;
			}
			length++;
		}
	}

	result = string_new(length);
	if (result == NULL) {
		return NULL;
	}

	currentChar = stringValue->content;
	currentOutputChar = result->content;
	codepointHexDigits = malloc(sizeof(char) * 5);
	if (codepointHexDigits == NULL) {
		string_free(result);
		return NULL;
	}

	*currentOutputChar = '"';
	currentOutputChar++;
	for (i = 0; i < stringValue->length; i++, currentChar++) {
		if (*currentChar == '"' || *currentChar == '/') {
			*currentOutputChar = '\\';
			currentOutputChar++;
			*currentOutputChar = *currentChar;
			currentOutputChar++;
		} else if (*currentChar <= 31) {
			codepoint = *currentChar;
			sprintf(codepointHexDigits, "%04x", codepoint);
			*currentOutputChar = '\\';
			currentOutputChar++;
			*currentOutputChar = 'u';
			currentOutputChar++;
			*currentOutputChar = *codepointHexDigits;
			currentOutputChar++;
			*currentOutputChar = *(codepointHexDigits + 1);
			currentOutputChar++;
			*currentOutputChar = *(codepointHexDigits + 2);
			currentOutputChar++;
			*currentOutputChar = *(codepointHexDigits + 3);
			currentOutputChar++;
		} else {
			*currentOutputChar = *currentChar;
			currentOutputChar++;
		}
	}
	*currentOutputChar = '"';

	free(codepointHexDigits);

	return result;
}

static string *encodeArray(value)
JSONValue *value;
{
	JSONValuePointerVector *items = NULL;
	JSONValue **itemPointer = NULL;
	stringPointerVector *encodedItems = NULL;
	unsigned long i = 0;
	string *result = NULL;
	string *prefix = NULL;
	string *itemSeparator = NULL;
	string *suffix = NULL;

	if (value == NULL || value->type != JSONValueType_ARRAY) {
		return NULL;
	}

	items = value->value.array;
	encodedItems = stringPointerVector_new(0, items->size.length);
	if (encodedItems == NULL) {
		return NULL;
	}

	itemPointer = items->items;
	for (i = 0; i < items->size.length; i++, itemPointer++) {
		string *encodedItem = JSON_encode(*itemPointer);
		stringPointerVector *grownItems;
		if (encodedItem == NULL) {
			freeStrings(encodedItems);
			return NULL;
		}

		grownItems =
		    stringPointerVector_append(encodedItems, &encodedItem);
		if (grownItems == NULL) {
			string_free(encodedItem);
			freeStrings(encodedItems);
			return NULL;
		}
		encodedItems = grownItems;
	}

	prefix = string_from("[");
	itemSeparator = string_from(",");
	suffix = string_from("]");

	if (prefix != NULL && itemSeparator != NULL && suffix != NULL) {
		result =
		    createCompositeValueResult(encodedItems, prefix,
					       itemSeparator, suffix);
	}

	string_free(prefix);
	string_free(itemSeparator);
	string_free(suffix);
	freeStrings(encodedItems);

	return result;
}

static string *encodeObject(value)
JSONValue *value;
{
	JSONObjectPropertyVector *properties = NULL;
	JSONObjectProperty *property = NULL;
	JSONValue *keyWrapper = NULL;
	stringPointerVector *encodedItems = NULL;
	unsigned char *currentOutputChar = NULL;
	unsigned long i = 0;
	string *result = NULL;
	string *prefix = NULL;
	string *propertySeparator = NULL;
	string *suffix = NULL;

	if (value == NULL || value->type != JSONValueType_OBJECT) {
		return NULL;
	}

	properties = value->value.object;
	encodedItems = stringPointerVector_new(0, properties->size.length);
	if (encodedItems == NULL) {
		return NULL;
	}

	property = properties->items;
	keyWrapper = JSONValue_newString(string_from(""));
	if (keyWrapper == NULL) {
		return NULL;
	}
	string_free(keyWrapper->value.string);
	keyWrapper->value.string = NULL;

	for (i = 0; i < properties->size.length; i++, property++) {
		string *encodedKey, *encodedValue, *encodedProperty;
		stringPointerVector *grownItems;
		keyWrapper->value.string = property->key;
		encodedKey = encodeString(keyWrapper);
		if (encodedKey == NULL) {
			freeStrings(encodedItems);
			return NULL;
		}
		encodedValue = JSON_encode(property->value);
		if (encodedValue == NULL) {
			string_free(encodedKey);
			freeStrings(encodedItems);
			return NULL;
		}

		if (ULONG_MAX - encodedKey->length < 1
		    || ULONG_MAX - encodedKey->length - 1 <
		    encodedValue->length) {
			string_free(encodedKey);
			string_free(encodedValue);
			freeStrings(encodedItems);
			return NULL;
		}

		encodedProperty =
		    string_new(encodedKey->length + 1 + encodedValue->length);
		if (encodedProperty == NULL) {
			string_free(encodedKey);
			string_free(encodedValue);
			freeStrings(encodedItems);
			return NULL;
		}
		memcpy(encodedProperty->content, encodedKey->content,
		       encodedKey->length);
		*(encodedProperty->content + encodedKey->length) = ':';
		currentOutputChar =
		    encodedProperty->content + encodedKey->length + 1;
		memcpy(currentOutputChar, encodedValue->content,
		       encodedValue->length);

		string_free(encodedKey);
		string_free(encodedValue);

		grownItems =
		    stringPointerVector_append(encodedItems, &encodedProperty);
		if (grownItems == NULL) {
			string_free(encodedProperty);
			freeStrings(encodedItems);
			return NULL;
		}
		encodedItems = grownItems;
	}

	prefix = string_from("{");
	propertySeparator = string_from(",");
	suffix = string_from("}");

	if (prefix != NULL && propertySeparator != NULL && suffix != NULL) {
		result =
		    createCompositeValueResult(encodedItems, prefix,
					       propertySeparator, suffix);
	}

	string_free(prefix);
	string_free(propertySeparator);
	string_free(suffix);
	freeStrings(encodedItems);

	return result;
}

static string *createCompositeValueResult(items, prefix, separator, suffix)
stringPointerVector *items;
string *prefix;
string *separator;
string *suffix;
{
	unsigned long length;
	string *result;
	unsigned char *currentOutputChar;

	if (prefix == NULL || items == NULL || separator == NULL
	    || suffix == NULL) {
		return NULL;
	}

	length = getJoinedStringLength(items, separator->length);
	if (length == 0 && items->size.length > 0) {
		return NULL;
	}

	if (ULONG_MAX - length < prefix->length) {
		return NULL;
	}
	length += prefix->length;
	if (ULONG_MAX - length < suffix->length) {
		return NULL;
	}
	length += suffix->length;

	result = string_new(length);
	if (result == NULL) {
		return NULL;
	}

	currentOutputChar = result->content;
	memcpy(currentOutputChar, prefix->content, prefix->length);
	joinStrings(currentOutputChar + prefix->length, items, separator);
	memcpy(currentOutputChar + length - suffix->length, suffix->content,
	       suffix->length);

	return result;
}

static unsigned long getJoinedStringLength(strings, separatorLength)
stringPointerVector *strings;
unsigned long separatorLength;
{
	unsigned long length = 0;
	string **itemPointer = NULL;
	unsigned long i = 0;

	if (strings == NULL) {
		return 0;
	}

	itemPointer = strings->items;
	for (i = 0; i < strings->size.length; i++, itemPointer++) {
		if (*itemPointer == NULL) {
			return 0;
		}

		if (i > 0) {
			if (ULONG_MAX - length < separatorLength) {
				return 0;
			}
			length += separatorLength;
		}

		if (ULONG_MAX - length < (*itemPointer)->length) {
			return 0;
		}

		length += (*itemPointer)->length;
	}

	return length;
}

static void joinStrings(destination, strings, separator)
unsigned char *destination;
stringPointerVector *strings;
string *separator;
{
	unsigned long i;

	if (destination == NULL || strings == NULL || separator == NULL) {
		return;
	}

	for (i = 0; i < strings->size.length; i++) {
		string *item;

		if (i > 0) {
			memcpy(destination, separator->content,
			       separator->length);
			destination += separator->length;
		}

		item = *(strings->items + i);
		memcpy(destination, item->content, item->length);
		destination += item->length;
	}
}

static void freeStrings(strings)
stringPointerVector *strings;
{
	unsigned long i;
	for (i = 0; i < strings->size.length; i++) {
		string_free(*(strings->items + i));
	}
	stringPointerVector_free(strings);
}

Vector_ofPointerImplementation(string)
