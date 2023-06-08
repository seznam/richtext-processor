#include "../../src/parser.h"
#include "../../src/json/ast_node.h"
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

START_TEST(ASTNode_toFlatJSON_returnsJsonNullForNullInput)
{
	JSONValue *result = ASTNode_toFlatJSON(NULL);
	assert(result != NULL, "Expected JSON null as result");
	assert(result->type == JSONValueType_NULL,
	       "Expected JSON null as result");
END_TEST}

START_TEST(ASTNode_toFlatJSON_serializesInputNodeToJson)
{
	ASTNode node;
	ASTNode parent;
	JSONValue *result;
	JSONObjectPropertyVector *properties;
	string *serialized;

	node.byteIndex = 1;
	node.codepointIndex = 7;
	node.tokenIndex = 4;
	node.type = ASTNodeType_COMMAND;
	node.value = string_from("foobar");
	node.parent = &parent;	/* check that parent is not included */
	node.children = ASTNodePointerVector_new(0, 0);
	result = ASTNode_toFlatJSON(&node);

	assert(result != NULL, "Expected non-NULL result");
	assert(result->type == JSONValueType_OBJECT, "Expected JSON object");
	properties = result->value.object;
	assert(properties->size.length == 5,
	       "Expected the JSON object to have 5 properties");

	serialized = JSON_encode(result);
	assert(string_compare
	       (serialized,
		string_from
		("{\"byteIndex\":1,\"codepointIndex\":7,\"tokenIndex\":4,\"type\":\"COMMAND\",\"value\":\"foobar\"}"))
	       == 0,
	       "Expected the JSON object's properties to match the node's properties");
END_TEST}

START_TEST(ASTNodePointerVector_toJSON_returnsNullPointerForNullInput)
{
	assert(ASTNodePointerVector_toJSON(NULL) == NULL,
	       "Expected NULL result");
END_TEST}

START_TEST(ASTNodePointerVector_toJSON_serializesNodesInInputVectorRecursively)
{
	ASTNode *node1, *node2, *subNode1;
	ASTNodePointerVector *nodes, *subNodes;
	JSONValue *result;
	string *serialized;

	nodes = ASTNodePointerVector_new(0, 0);
	subNodes = ASTNodePointerVector_new(0, 0);

	subNode1 = malloc(sizeof(ASTNode));
	subNode1->byteIndex = 7;
	subNode1->codepointIndex = 5;
	subNode1->tokenIndex = 3;
	subNode1->type = ASTNodeType_TEXT;
	subNode1->value = string_from("bar");
	subNode1->parent = node1;
	subNode1->children = NULL;

	subNodes = ASTNodePointerVector_append(subNodes, &subNode1);

	node1 = malloc(sizeof(ASTNode));
	node1->byteIndex = 2;
	node1->codepointIndex = 1;
	node1->tokenIndex = 2;
	node1->type = ASTNodeType_COMMAND;
	node1->value = string_from("foo");
	node1->parent = NULL;
	node1->children = subNodes;

	node2 = malloc(sizeof(ASTNode));
	node2->byteIndex = 15;
	node2->codepointIndex = 10;
	node2->tokenIndex = 4;
	node2->type = ASTNodeType_WHITESPACE;
	node2->value = string_from(" ");
	node2->parent = NULL;
	node2->children = NULL;

	nodes = ASTNodePointerVector_append(nodes, &node1);
	nodes = ASTNodePointerVector_append(nodes, &node2);

	result = ASTNodePointerVector_toJSON(nodes);
	serialized = JSON_encode(result);

	assert(string_compare
	       (serialized,
		string_from
		("[{\"byteIndex\":2,\"codepointIndex\":1,\"tokenIndex\":2,\"type\":\"COMMAND\",\"value\":\"foo\",\"children\":[{\"byteIndex\":7,\"codepointIndex\":5,\"tokenIndex\":3,\"type\":\"TEXT\",\"value\":\"bar\"}]},{\"byteIndex\":15,\"codepointIndex\":10,\"tokenIndex\":4,\"type\":\"WHITESPACE\",\"value\":\" \"}]"))
	       == 0,
	       "Expected the provided nodes to be serialized to JSON recursively");
END_TEST}

START_TEST(ASTNode_toJSON_returnsJsonNullForNullInput)
{
	JSONValue *result;
	result = ASTNode_toJSON(NULL);
	assert(result != NULL
	       && result->type == JSONValueType_NULL,
	       "Expected JSON null result");
END_TEST}

START_TEST(ASTNode_toJSON_serializedNodeIncludingChildrenRecursively)
{
	ASTNode *node1, *node2, *node3;
	JSONValue *encoded;
	string *stringified;

	node1 = malloc(sizeof(ASTNode));
	node2 = malloc(sizeof(ASTNode));
	node3 = malloc(sizeof(ASTNode));

	node1->byteIndex = 1;
	node1->codepointIndex = 2;
	node1->tokenIndex = 3;
	node1->parent = NULL;
	node1->type = ASTNodeType_COMMAND;
	node1->value = string_from("foo");
	node1->children = ASTNodePointerVector_of1(node2);

	node2->byteIndex = 4;
	node2->codepointIndex = 5;
	node2->tokenIndex = 6;
	node2->parent = node1;
	node2->type = ASTNodeType_COMMAND;
	node2->value = string_from("bar");
	node2->children = ASTNodePointerVector_of1(node3);

	node3->byteIndex = 7;
	node3->codepointIndex = 8;
	node3->tokenIndex = 9;
	node3->parent = node2;
	node3->type = ASTNodeType_TEXT;
	node3->value = string_from("baz");
	node3->children = ASTNodePointerVector_new(0, 0);

	encoded = ASTNode_toJSON(node1);
	stringified = JSON_encode(encoded);
	assert(string_compare
	       (stringified,
		string_from
		("{\"byteIndex\":1,\"codepointIndex\":2,\"tokenIndex\":3,\"type\":\"COMMAND\",\"value\":\"foo\",\"children\":[{\"byteIndex\":4,\"codepointIndex\":5,\"tokenIndex\":6,\"type\":\"COMMAND\",\"value\":\"bar\",\"children\":[{\"byteIndex\":7,\"codepointIndex\":8,\"tokenIndex\":9,\"type\":\"TEXT\",\"value\":\"baz\",\"children\":[]}]}]}"))
	       == 0, "Expected recursively serialized ");
END_TEST}

static void all_tests()
{
	runTest(ASTNode_toFlatJSON_returnsJsonNullForNullInput);
	runTest(ASTNode_toFlatJSON_serializesInputNodeToJson);
	runTest(ASTNodePointerVector_toJSON_returnsNullPointerForNullInput);
	runTest
	    (ASTNodePointerVector_toJSON_serializesNodesInInputVectorRecursively);
	runTest(ASTNode_toJSON_returnsJsonNullForNullInput);
	runTest(ASTNode_toJSON_serializedNodeIncludingChildrenRecursively);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
