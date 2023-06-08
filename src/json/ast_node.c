#include "../parser.h"
#include "ast_node.h"
#include "json_value.h"

static char *byteIndexKeyContent = "byteIndex";
static char *codepointIndexKeyContent = "codepointIndex";
static char *tokenIndexKeyContent = "tokenIndex";
static char *typeKeyContent = "type";
static char *valueKeyContent = "value";
static char *childrenKeyContent = "children";

static char *typeCOMMANDContent = "COMMAND";
static char *typeTEXTContent = "TEXT";
static char *typeWHITESPACEContent = "WHITESPACE";

static JSONValue *getNodeTypeAsJson(ASTNodeType type);

JSONValue *ASTNode_toJSON(node)
ASTNode *node;
{
	JSONValue *nodeJson = ASTNode_toFlatJSON(node);
	JSONValue *childrenArray = NULL;
	string *childrenKey = NULL;

	if (nodeJson == NULL || nodeJson->type != JSONValueType_OBJECT) {
		return nodeJson;
	}

	if (node->children == NULL) {
		return nodeJson;
	}

	childrenKey = string_from(childrenKeyContent);
	if (childrenKey == NULL) {
		string_free(childrenKey);
		JSONValue_freeRecursive(nodeJson);
		return NULL;
	}

	childrenArray = ASTNodePointerVector_toJSON(node->children);
	if (childrenArray == NULL) {
		string_free(childrenKey);
		JSONValue_freeRecursive(nodeJson);
		return NULL;
	}

	if (JSONValue_setObjectProperty(nodeJson, childrenKey, childrenArray) ==
	    NULL) {
		string_free(childrenKey);
		JSONValue_freeRecursive(childrenArray);
		JSONValue_freeRecursive(nodeJson);
		return NULL;
	}

	return nodeJson;
}

JSONValue *ASTNode_toFlatJSON(node)
ASTNode *node;
{
	JSONValue *nodeJson;
	string *byteIndexKey = NULL;
	JSONValue *byteIndexValue;
	string *codepointIndexKey = NULL;
	JSONValue *codepointIndexValue;
	string *tokenIndexKey = NULL;
	JSONValue *tokenIndexValue;
	string *typeKey = NULL;
	JSONValue *typeValue;
	string *valueKey = NULL;
	JSONValue *valueValue;
	JSONValue *modifiedObject = NULL;

	if (node == NULL) {
		return JSONValue_newNull();
	}

	byteIndexKey = string_from(byteIndexKeyContent);
	codepointIndexKey = string_from(codepointIndexKeyContent);
	tokenIndexKey = string_from(tokenIndexKeyContent);
	typeKey = string_from(typeKeyContent);
	valueKey = string_from(valueKeyContent);
	if (byteIndexKey == NULL || codepointIndexKey == NULL
	    || tokenIndexKey == NULL || typeKey == NULL || valueKey == NULL) {
		string_free(byteIndexKey);
		string_free(codepointIndexKey);
		string_free(tokenIndexKey);
		string_free(typeKey);
		string_free(valueKey);
		return NULL;
	}

	nodeJson = JSONValue_newObject();
	if (nodeJson == NULL) {
		return NULL;
	}

	byteIndexValue = JSONValue_newNumber(node->byteIndex);
	if (JSONValue_setObjectProperty(nodeJson, byteIndexKey, byteIndexValue)
	    == NULL) {
		string_free(byteIndexKey);
		string_free(codepointIndexKey);
		string_free(tokenIndexKey);
		string_free(typeKey);
		string_free(valueKey);
		JSONValue_free(byteIndexValue);
		JSONValue_freeRecursive(nodeJson);
		return NULL;
	}

	codepointIndexValue = JSONValue_newNumber(node->codepointIndex);
	if (JSONValue_setObjectProperty
	    (nodeJson, codepointIndexKey, codepointIndexValue) == NULL) {
		string_free(codepointIndexKey);
		string_free(tokenIndexKey);
		string_free(typeKey);
		string_free(valueKey);
		JSONValue_free(codepointIndexValue);
		JSONValue_freeRecursive(nodeJson);
		return NULL;
	}

	tokenIndexValue = JSONValue_newNumber(node->tokenIndex);
	if (JSONValue_setObjectProperty
	    (nodeJson, tokenIndexKey, tokenIndexValue)
	    == NULL) {
		string_free(tokenIndexKey);
		string_free(typeKey);
		string_free(valueKey);
		JSONValue_free(tokenIndexValue);
		JSONValue_freeRecursive(nodeJson);
		return NULL;
	}

	typeValue = getNodeTypeAsJson(node->type);
	if (JSONValue_setObjectProperty(nodeJson, typeKey, typeValue) == NULL) {
		string_free(typeKey);
		string_free(valueKey);
		JSONValue_free(typeValue);
		JSONValue_freeRecursive(nodeJson);
		return NULL;
	}

	valueValue = JSONValue_newString(node->value);
	modifiedObject =
	    JSONValue_setObjectProperty(nodeJson, valueKey, valueValue);
	if (modifiedObject == NULL) {
		string_free(valueKey);
		JSONValue_free(valueValue);
		JSONValue_freeRecursive(nodeJson);
		return NULL;
	}

	return nodeJson;
}

JSONValue *ASTNodePointerVector_toJSON(nodes)
ASTNodePointerVector *nodes;
{
	JSONValue *array = JSONValue_newArray();
	ASTNode **nodePointer = NULL;
	unsigned long i = 0;

	if (nodes == NULL || array == NULL) {
		return NULL;
	}

	nodePointer = nodes->items;
	for (i = 0; i < nodes->size.length; i++, nodePointer++) {
		JSONValue *childJson = ASTNode_toJSON(*nodePointer);
		if (childJson == NULL) {
			JSONValue_freeRecursive(array);
			return NULL;
		}
		if (JSONValue_pushToArray(array, childJson) == NULL) {
			JSONValue_freeRecursive(childJson);
			JSONValue_freeRecursive(array);
			return NULL;
		}
	}

	return array;
}

static JSONValue *getNodeTypeAsJson(nodeType)
ASTNodeType nodeType;
{
	switch (nodeType) {
	case ASTNodeType_COMMAND:
		return JSONValue_newString(string_from(typeCOMMANDContent));

	case ASTNodeType_TEXT:
		return JSONValue_newString(string_from(typeTEXTContent));

	case ASTNodeType_WHITESPACE:
		return JSONValue_newString(string_from(typeWHITESPACEContent));

	default:
		return NULL;
	}
}
