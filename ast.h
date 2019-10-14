#ifndef AST_H
#define AST_H

// Arbitrary Syntax Tree data structure
typedef struct AstNode{
	char* tok;			// Instruction/data token
	int isData;			// Boolean if the token is data or a special token
	AstNode* left;
	AstNode* right;
} AstNode;

void add_node(AstNode* root, AstNode* l, AstNode* r);
void set_token(AstNode* node, char* tok);
void free_ast(AstNode* node);

#endif
