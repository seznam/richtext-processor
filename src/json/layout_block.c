#include "../layout_block.h"
#include "../layout_block_vector.h"
#include "../string.h"
#include "ast_node.h"
#include "layout_block.h"
#include "layout_block_type.h"
#include "layout_paragraph.h"
#include "json_value.h"

static char *causingCommandKeyValue = "causingCommand";
static char *typeKeyValue = "type";
static char *paragraphsKeyValue = "paragraphs";

JSONValue *LayoutBlock_toJSON(block)
LayoutBlock *block;
{
	JSONValue *blockJson;
	string *causingCommandKey, *typeKey, *paragraphsKey;
	JSONValue *causingCommandValue, *typeValue, *paragraphsValue;

	if (block == NULL) {
		return JSONValue_newNull();
	}

	blockJson = JSONValue_newObject();
	if (blockJson == NULL) {
		return NULL;
	}

	causingCommandKey = string_from(causingCommandKeyValue);
	typeKey = string_from(typeKeyValue);
	paragraphsKey = string_from(paragraphsKeyValue);

	causingCommandValue = ASTNode_toJSON(block->causingCommand);
	typeValue = LayoutBlockType_toJSON(block->type);
	paragraphsValue = LayoutParagraphVector_toJSON(block->paragraphs);

	if (causingCommandKey == NULL || typeKey == NULL
	    || paragraphsKey == NULL || causingCommandValue == NULL
	    || typeValue == NULL || paragraphsValue == NULL
	    || JSONValue_setObjectProperty(blockJson, causingCommandKey,
					   causingCommandValue) == NULL
	    || JSONValue_setObjectProperty(blockJson, typeKey,
					   typeValue) == NULL
	    || JSONValue_setObjectProperty(blockJson, paragraphsKey,
					   paragraphsValue) == NULL) {
		string_free(causingCommandKey);
		string_free(typeKey);
		string_free(paragraphsKey);
		JSONValue_freeRecursive(causingCommandValue);
		JSONValue_freeRecursive(typeValue);
		JSONValue_freeRecursive(paragraphsValue);
		JSONValue_free(blockJson);
	}

	return blockJson;
}

JSONValue *LayoutBlockVector_toJSON(blocks)
LayoutBlockVector *blocks;
{
	JSONValue *blocksJson;
	LayoutBlock *block;
	unsigned long i;

	if (blocks == NULL) {
		return JSONValue_newNull();
	}

	blocksJson = JSONValue_newArray();

	block = blocks->items;
	for (i = 0; i < blocks->size.length; i++, block++) {
		JSONValue *blockJson = LayoutBlock_toJSON(block);
		if (blockJson == NULL
		    || JSONValue_pushToArray(blocksJson, blockJson) == NULL) {
			JSONValue_freeRecursive(blockJson);
			JSONValue_freeRecursive(blocksJson);
			return NULL;
		}
	}

	return blocksJson;
}
