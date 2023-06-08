#ifndef JSON_AST_NODE_HEADER_FILE
#define JSON_AST_NODE_HEADER_FILE 1

#include "../parser.h"
#include "json_value.h"

JSONValue *ASTNode_toJSON(ASTNode * node);

JSONValue *ASTNode_toFlatJSON(ASTNode * node);

JSONValue *ASTNodePointerVector_toJSON(ASTNodePointerVector * nodes);

#endif
