#ifndef AST_NODE_HEADER_FILE
#define AST_NODE_HEADER_FILE 1

#include "ast_node_type.h"
#include "string.h"

struct ASTNode;
typedef struct ASTNode ASTNode;

#include "ast_node_pointer_vector.h"

struct ASTNode {
	unsigned long byteIndex;
	unsigned long codepointIndex;
	unsigned long tokenIndex;
	ASTNodeType type;
	string *value;
	ASTNode *parent;
	ASTNodePointerVector *children;
};

#endif
