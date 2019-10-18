// Arbitrary syntax tree for building commands and executing in proper operation order

#include "svec.h"

#ifndef AST_H
#define AST_H

// Arbitrary Syntax Tree data structure
typedef struct AstNode {
	// char* tok;			// Instruction/data token
	// int isData;			// Boolean if the token is data or a special token
	
	// Decided to go for a more one-of-either approach instead
	char* operationToken;			// Unix operation token (e.g. < > ; & )
	svec* instructionTokens;		// Argument/Instruction tokens (anything else)
	struct AstNode* left;
	struct AstNode* right;
} AstNode;

AstNode* make_blank_ast_node();
AstNode* make_full_ast_node(char* operationToken, svec* instructionTokens);		// Unused?
AstNode* make_operation_ast_node(char* operationToken);
AstNode* make_instruction_ast_node(svec* instructionTokens);
void add_top_node(AstNode* root, AstNode* l, AstNode* r);
// void set_operation_token(AstNode* node, char* operationToken);
// void set_instruction_tokens(AstNode* node, svec* instructionTokens);
void free_ast(AstNode* node);
AstNode* parse_tokens(svec* tokens);

#endif
