#include "../../src/json/json_encoder.h"
#include "../../src/json/json_value.h"
#include "../../src/json/layout_block.h"
#include "../../src/ast_node.h"
#include "../../src/ast_node_pointer_vector.h"
#include "../../src/ast_node_type.h"
#include "../../src/layout_block.h"
#include "../../src/layout_block_type.h"
#include "../../src/layout_line_vector.h"
#include "../../src/layout_paragraph.h"
#include "../../src/layout_paragraph_type.h"
#include "../../src/layout_paragraph_vector.h"
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

START_TEST(LayoutBlock_toJSON_returnsJsonNullForNullInput)
{
	JSONValue *result = LayoutBlock_toJSON(NULL);
	assert(result != NULL
	       && result->type == JSONValueType_NULL,
	       "Expected a JSON null for NULL input");
END_TEST}

START_TEST(LayoutBlock_toJSON_encodesProvidedBlockToJson)
{
	JSONValue *blockJson;
	string *encoded, *expected;
	LayoutBlock block;
	ASTNode cause;
	LayoutParagraph paragraph;

	cause.byteIndex = 1;
	cause.codepointIndex = 2;
	cause.tokenIndex = 3;
	cause.type = ASTNodeType_TEXT;
	cause.value = string_from("foo");
	cause.parent = NULL;
	cause.children = ASTNodePointerVector_new(0, 0);

	paragraph.causingCommand = NULL;
	paragraph.type = LayoutParagraphType_IMPLICIT;
	paragraph.lines = LayoutLineVector_new(0, 0);

	block.causingCommand = &cause;
	block.type = LayoutBlockType_MAIN_CONTENT;
	block.paragraphs = LayoutParagraphVector_of1(paragraph);

	blockJson = LayoutBlock_toJSON(&block);
	encoded = JSON_encode(blockJson);

	expected =
	    string_from
	    ("{\"causingCommand\":{\"byteIndex\":1,\"codepointIndex\":2,\"tokenIndex\":3,\"type\":\"TEXT\",\"value\":\"foo\",\"children\":[]},\"type\":\"MAIN_CONTENT\",\"paragraphs\":[{\"causingCommand\":null,\"type\":\"IMPLICIT\",\"lines\":[]}]}");
	assert(string_compare(encoded, expected) == 0,
	       "The provided JSON output did not match expectations");
END_TEST}

START_TEST(LayoutBlockVector_toJSON_returnsJsonNullForNullInput)
{
	JSONValue *result = LayoutBlockVector_toJSON(NULL);
	assert(result != NULL
	       && result->type == JSONValueType_NULL,
	       "Expected JSON null for NULL input");
END_TEST}

START_TEST(LayoutBlockVector_toJSON_returnsEmptyArrayForEmptyInputVector)
{
	JSONValue *result;
	result = LayoutBlockVector_toJSON(LayoutBlockVector_new(0, 0));
	assert(result != NULL && result->type == JSONValueType_ARRAY
	       && result->value.array->size.length == 0,
	       "Expected an empty JSON array for empty input blocks vector");
END_TEST}

START_TEST(LayoutBlockVector_toJSON_encodesProvidedBlocksToJsonArray)
{
	JSONValue *blocksJson;
	LayoutBlock block1, block2;
	LayoutBlockVector *blocks;
	string *serialized, *expected;

	block1.causingCommand = NULL;
	block1.type = LayoutBlockType_SAME_PAGE_START;
	block1.paragraphs = LayoutParagraphVector_new(0, 0);

	block2.causingCommand = NULL;
	block2.type = LayoutBlockType_PAGE_BREAK;
	block2.paragraphs = LayoutParagraphVector_new(0, 0);

	blocks = LayoutBlockVector_of2(block1, block2);
	blocksJson = LayoutBlockVector_toJSON(blocks);
	serialized = JSON_encode(blocksJson);

	expected =
	    string_from
	    ("[{\"causingCommand\":null,\"type\":\"SAME_PAGE_START\",\"paragraphs\":[]},{\"causingCommand\":null,\"type\":\"PAGE_BREAK\",\"paragraphs\":[]}]");
	assert(string_compare(serialized, expected) == 0,
	       "The provided JSON output did not match expectations");
END_TEST}

static void all_tests()
{
	runTest(LayoutBlock_toJSON_returnsJsonNullForNullInput);
	runTest(LayoutBlock_toJSON_encodesProvidedBlockToJson);
	runTest(LayoutBlockVector_toJSON_returnsJsonNullForNullInput);
	runTest(LayoutBlockVector_toJSON_returnsEmptyArrayForEmptyInputVector);
	runTest(LayoutBlockVector_toJSON_encodesProvidedBlocksToJsonArray);
}

int main()
{
	runTestSuite(all_tests);
	return tests_failed;
}
