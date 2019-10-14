// Relevant imports
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "tokens.h"

// Makes an AST given a svec of tokens
AstNode* token_list_to_ast(svec* tokens) {
	AstNode* running = make_ast_node();
	for (int ii = 0; ii < tokens->size - 1; ii++) {
		char* runningToken = svec_get(tokens, ii);
		
		// Handles the asString inputs in quotes
		if (*runningToken == '"') {

		}

		// Recursively parses parenthetes
		else if (*runningToken == '(') {
			if (strlen(runningToken) >= 2) {
				runningToken[strlen(runningToken) - 1] = '\0';
				runningToken++;
				svec* parenTokenized = tokenize(runningToken);
				AstNode* parenAst = token_list_to_ast(parenTokenized);
			}
		}


		
	}

	return running;
}

// Makes an ast node
AstNode* make_ast_node() {
	AstNode* node = malloc(sizeof(AstNode));
	node->tok = '\0';
	node->isData = 0;
	node->left = NULL;
	node->right = NULL;
	return node;
}

// Adds a node to the AST
void add_node(AstNode* root, AstNode* l, AstNode* r) {
	// Null node symbolizes the end of a tree
	assert(root != NULL);
	root->left = l;
	root->right = r;
}

// Sets the token of the node
void set_token(AstNode* node, char* tok) {
	assert (node != NULL && tok != NULL);

	// Sets the token of this node
	strcpy(node->tok, tok);

	// Checks whether it should assign the node as data or a special token
	switch (*tok) {
		case '<' :		// Intentional fall-through for all special tokens
		case '>' :
		case ';' :
		case '=' :
		case '\\' :
		case '&' :
		case '|' :
		case '(' :
		case '"' :
			node->isData = 0;
			break;
		default :
			node->isData = 1;
			break;	
	}
}

// Frees the node and its children
void free_ast(AstNode* node) {
	// Break at null base case
	if (node != NULL) {
		// Free the token
		free(node->tok);
		node->tok = NULL;

		// Frees nodes recursively
		free_ast(node->left);
		free_ast(node->right);
		node->left = NULL;
		node->right = NULL;
	}
}
