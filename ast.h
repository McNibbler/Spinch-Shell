#include "svec.h"

#ifndef AST_H
#define AST_H

// Arbitrary Syntax Tree data structure
typedef struct AstNode {
	char* tok;			// Instruction/data token
	int isData;			// Boolean if the token is data or a special token
	struct AstNode* left;
	struct AstNode* right;
} AstNode;

AstNode* parse_tokens(svec* tokens);
AstNode* make_blank_ast_node();
AstNode* make_ast_node(char* tok);
void add_node(AstNode* root, AstNode* l, AstNode* r);
void set_token(AstNode* node, char* tok);
void free_ast(AstNode* node);

#endif
