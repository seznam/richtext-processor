#ifndef JSON_VALUE_HEADER_FILE
#define JSON_VALUE_HEADER_FILE 1

#include "../bool.h"
#include "../string.h"
#include "../vector.h"
#include "../typed_vector.h"

typedef enum JSONValueType {
	JSONValueType_NULL,
	JSONValueType_BOOLEAN,
	JSONValueType_NUMBER,
	JSONValueType_STRING,
	JSONValueType_ARRAY,
	JSONValueType_OBJECT
} JSONValueType;

struct JSONValue;
typedef struct JSONValue JSONValue;

Vector_ofType(JSONValue)
struct JSONObjectProperty;
typedef struct JSONObjectProperty JSONObjectProperty;

Vector_ofType(JSONObjectProperty)
struct JSONValue {
	JSONValueType type;
	union {
		bool boolean;
		double number;
		string *string;
		JSONValueVector *array;
		JSONObjectPropertyVector *object;
	} value;
};

struct JSONObjectProperty {
	string *key;
	JSONValue *value;
};

JSONValue *JSONValue_new(JSONValueType type);

JSONValue *JSONValue_newNull(void);

JSONValue *JSONValue_newBoolean(bool value);

JSONValue *JSONValue_newNumber(double value);

JSONValue *JSONValue_newString(string * value);

JSONValue *JSONValue_newArray(void);

JSONValue *JSONValue_newObject(void);

JSONValue *JSONValue_pushToArray(JSONValue * array, JSONValue * value);

JSONValue *JSONValue_setObjectProperty(JSONValue * object, string * key,
				       JSONValue * value);

JSONValue *JSONValue_deleteObjectProperty(JSONValue * object, string * key);

void JSONValue_free(JSONValue * value);

#endif
