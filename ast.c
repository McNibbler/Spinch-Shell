// Relevant import
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "tokens.h"
#include <stdio.h>

// Makes an AST given a svec of tokens
AstNode* parse_tokens(svec* tokens) {
	// Ensure tokens exist and are not empty
	if (tokens == NULL || !tokens->size) {
		return make_blank_ast_node();
	}

	// Operations
	char* ops[] = {"\"", "(", "=", ";", "\\", "&", "|", "<", ">", "&&", "||"}; //, "$"};
	int opsSize = 11;

	// Cycles through the operations and parses based on that
	int hasOper = 0;
	for (int ii = 0; ii < opsSize; ii++) {
		int findIndex = svec_find(tokens, ops[ii]);
		if (findIndex > -1) {
			svec* left = svec_slice(tokens, 0, findIndex);
			svec* right = svec_slice(tokens, findIndex + 1, tokens->size);
			AstNode* result = make_operation_ast_node(ops[ii]);
			add_top_node(result, parse_tokens(left), parse_tokens(right));
			return result;
		}
	}

	// Returns an ast with no children and just raw instructions if there is no operations
	return make_instruction_ast_node(tokens);
	

	// A relic of shell past... hope i dont need this
	
	// AstNode* running = make_ast_node();
	// for (int ii = 0;  ii < tokens->size - 1; ii++) {
	// 	char* runningToken = svec_get(tokens, ii);
		
	//	// Recursively parses parenthetes
	//	else if (*runningToken == '(') {
	//		if (strlen(runningToken) >= 2) {
	//			runningToken[strlen(runningToken) - 1] = '\0';
	//			runningToken++;
	//			svec* parenTokenized = tokenize(runningToken);
	//			AstNode* parenAst = parse_tokens(parenTokenized);
	//			return parenAst;
	//		}
	//	}
	
	//	// 
	//	//else if
		
	//	}

}

// Makes an ast node
AstNode* make_blank_ast_node() {
	AstNode* node = malloc(sizeof(AstNode));
	node->operationToken = "\0";
	node->instructionTokens = NULL;
	node->left = NULL;
	node->right = NULL;
	return node;
}

// Makes an ast node with a specified operation token and instructions
AstNode* make_full_ast_node(char* operationToken, svec* instructionTokens) {
	assert(operationToken);
	int isOper = 0;
	// Checks whether it should assign the node as data or a special token
	switch (*operationToken) {
		case '<' :		// Intentional fall-through for all special tokens
		case '>' :
		case ';' :
		case '=' :
		case '\\' :
		case '&' :
		case '|' :
		case '(' :
		case '"' :
			isOper = 1;
			break;
		default :
			isOper = 0;
			break;	
	}
	assert(isOper);

	AstNode* node = malloc(sizeof(AstNode));
	node->operationToken = strdup(operationToken);
	node->instructionTokens = instructionTokens;
	node->left = NULL;
	node->right = NULL;
	return node;

}

// Makes an ast node with a specified operation token
AstNode* make_operation_ast_node(char* operationToken) {
	assert(operationToken);
	int isOper = 0;
	// Checks whether it should assign the node as data or a special token
	switch (*operationToken) {
		case '<' :		// Intentional fall-through for all special tokens
		case '>' :
		case ';' :
		case '=' :
		case '$' :
		case '\\' :
		case '&' :
		case '|' :
		case '(' :
		case '"' :
			isOper = 1;
			break;
		default :
			isOper = 0;
			break;	
	}
	assert(isOper);
	AstNode* node = malloc(sizeof(AstNode));
	node->operationToken = strdup(operationToken);
	node->instructionTokens = make_svec();
	node->left = NULL;
	node->right = NULL;
	return node;
}

// Makes an ast node with specified instructions
AstNode* make_instruction_ast_node(svec* instructionTokens) {
	AstNode* node = malloc(sizeof(AstNode));
	node->operationToken = "\0";
	node->instructionTokens = instructionTokens;
	node->left = NULL;
	node->right = NULL;
	return node;
}

// Adds a node to the AST
void add_top_node(AstNode* root, AstNode* l, AstNode* r) {
	// Null node symbolizes the end of a tree
	assert(root != NULL);
	root->left = l;
	root->right = r;
}

// Frees the node and its children
void free_ast(AstNode* node) {
	// Break at null base case
	if (node != NULL) {
		// Free the tokens
		free(node->operationToken);
		node->operationToken = NULL;
		free(node->instructionTokens);
		node->instructionTokens = NULL;

		// Frees nodes recursively
		free_ast(node->left);
		free_ast(node->right);
		node->left = NULL;
		node->right = NULL;
	}
}


// Decided to get rid of the below functionality because they weren't really ever used
// -- and I couldn't be assed to figure out their memory misalignment problems right now
// -- because it's 5AM and segfaults suck

/*

// Sets the operation token of the node
void set_operation_token(AstNode* node, char* operationToken) {
	assert(node != NULL && operationToken != NULL);

	// Sets the token of this node
	node->operationToken = strdup(operationToken);

	int isOper = 0;
	
	// Checks whether it should assign the node as data or a special token
	switch (*operationToken) {
		case '<' :		// Intentional fall-through for all special tokens
		case '>' :
		case ';' :
		case '=' :
		case '\\' :
		case '&' :
		case '|' :
		case '(' :
		case '"' :
			isOper = 1;
			break;
		default :
			isOper = 0;
			break;	
	}

	assert(isOper);
}

// Sets the instruction tokens of the node
void set_instruction_tokens(AstNode* node, svec* instructionTokens) {
	assert(node != NULL && instructionTokens != NULL);
	// I prolly should copy this but whatever I think in this case it's okay
	node->instructionTokens = instructionTokens;
}


*/
