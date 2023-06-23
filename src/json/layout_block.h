#ifndef JSON_LAYOUT_BLOCK_HEADER_FILE
#define JSON_LAYOUT_BLOCK_HEADER_FILE 1

#include "../layout_block.h"
#include "../layout_block_vector.h"
#include "json_value.h"

JSONValue *LayoutBlock_toJSON(LayoutBlock * block);

JSONValue *LayoutBlockVector_toJSON(LayoutBlockVector * blocks);

#endif
