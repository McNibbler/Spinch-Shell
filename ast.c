// Relevant imports
#include "ast.h"
#include <assert.h>

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
		free_ast(AstNode->left);
		free_ast(AstNode->right);
		AstNode->left = NULL;
		AstNode->right = NULL;
	}
}
