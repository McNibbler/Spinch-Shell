//////////////////////////////////////////////////////////////////////
// Spinch Shell														//
// An ultra-lightweight POSIX-style shell							//
// Thomas Kaunzinger - Fall 2019									//
// 																	//
// This shell implements most of the basic POSIX-style commands		//
// and operations, including piping, file redirection, variable		//
// storing and recalling, backgrounding, logical operators, etc.	//
//																	//
// The program can be launched standalone interactively or can		//
// input a .sh file for one-off script execution.					//
//																	//
// Some features are still TODO and I will eventually update		//
// them at a later date so that I don't drive myself insane, but	//
// likely not for a while due to my own personal time constraints.	//
//																	//
// These missing features include...								//
// - Sub-command in parentheses support								//
// - Commands with inputs in quotation marks						//
// - Escape characters for newline ignoring							//
// - Order of operations might not be 100% correct in parsing		//
// - Might want to use a hashmap instead of vectors for variables	//
// - Show working directory instead of just spinch$					//
// - Improve cd for its different modes of operation				//
// - Autocomplete and command history?								//
//////////////////////////////////////////////////////////////////////

//////////////////////////////
// Relevant library imports //
//////////////////////////////

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

#include "tokens.h"
#include "svec.h"
#include "ast.h"

//////////////////////
// Global variables //
//////////////////////

// User variables stored here
// TODO: Update my hashmap implementation so that it uses arbitrary length string keys so that
// -- I no longer have to use nasty linear time association vectors just to store variables.
// EDIT: In retrospect, association vectors might be better for smaller sizes of data being stored
// -- due to the considerably lower overhead, so they might actually be better for this application
svec* variableNames;
svec* variableValues;

// This is the exit code expected from the syscalls
int exitCode = -1;

///////////////////////////
// Function Declarations //
///////////////////////////

// Handles the special operations of the AST
int execute_operations(AstNode* ast); 

// Interprets and executes the parsed AST
int execute_ast(AstNode* ast);

// Parses and executes the command string
void execute(char* cmd);

////////// OPERATOR HANDLING //////////
int execute_assignment(AstNode* astL, AstNode* astR);		// =
int execute_semicolon(AstNode* astL, AstNode* astR);		// ;
int execute_back_slash(AstNode* astL, AstNode* astR);		// \    // TODO
int execute_background(AstNode* astL, AstNode* astR);		// &
int execute_pipe(AstNode* astL, AstNode* astR);				// |
int execute_left_arrow(AstNode* astL, AstNode* astR);		// <
int execute_right_arrow(AstNode* astL, AstNode* astR);		// >
int execute_and(AstNode* astL, AstNode* astR);				// &&
int execute_or(AstNode* astL, AstNode* astR);				// ||
int execute_quote(AstNode* astL, AstNode* astR);			// "	// TODO

/////////////////////////////
// Function Implementation //
/////////////////////////////

//////////////////// POSIX SPECIAL OPERATOR HANDLING ////////////////////

// Assigns a variable
int execute_assignment(AstNode* astL, AstNode* astR) {
	if (!astL || !astR || !astL->instructionTokens || !astR->instructionTokens
			|| astL->instructionTokens->size != 1 || astR->instructionTokens->size != 1) {
		printf("Error in variable assignment\n");
		return 1;
	}

	// Finds if the variable already exists or not and returns the index if it does
	int index = svec_find(variableNames,
			svec_get(astL->instructionTokens, astL->instructionTokens->size - 1));
	
	// Populates the association vectors
	if (index == -1) {
		svec_push_back(variableNames,
				svec_get(astL->instructionTokens, astL->instructionTokens->size - 1));
		svec_push_back(variableValues, svec_get(astR->instructionTokens, 0));
	}
	else {
		svec_put(variableValues, index, svec_get(astR->instructionTokens, 0));
	}	
	return 0;	
}

// Evaluates the semicolon separation operator
int execute_semicolon(AstNode* astL, AstNode* astR){
	int cpid;
	// Parent process
	if ((cpid = fork())) {
		int status;
		waitpid(cpid, &status, 0);
		// If signal caught to exit
		if (WEXITSTATUS(status) == 0xFF) {
			return exitCode;
		}
		return execute_ast(astR);
	}
	// Child process
	else {
		// I was a silly man and tried returning this instead of exiting for
		// far too long
		exit(execute_ast(astL));
	}
}

// TODO: handle the newline breaks
int execute_back_slash(AstNode* astL, AstNode* astR) {
	return exitCode;
}

// Backgrounds a process
int execute_background(AstNode* astL, AstNode* astR) {
	// Basically just semicolon without the waiting lol	
	int cpid;
	// Parent process
	if ((cpid = fork())) {
		return execute_ast(astR);
	}
	// Child process
	else {
		exit(execute_ast(astL));
	}
}

// Executes piping outputs to inputs
int execute_pipe(AstNode* astL, AstNode* astR) {

	// Checks the input validity
	if (!astR->instructionTokens // || !astR->instructionTokens->size
			|| !astL->instructionTokens) { //|| !astL->instructionTokens->size) {
		printf("error: missing pipe arguments\n");
		exit(1);
	}

	// This forking approach is kind of weird, but I found that I needed to do a fork to connect
	// the out of one pipe to another, and then I needed another fork so that when that piped
	// process ended, it didn't completely quit the program but rather just continue with it.

	int cpid;
	// Parent process
	if ((cpid = fork())) {
		int status;
		waitpid(cpid, &status, 0);
		return WEXITSTATUS(status);
	}
	// Child process
	// Decouples from the actual program to handle all the piping separate from the program
	else {
		// Generates the pipe file descriptors
		// -- This needs to be in the child process to decouple and return correctly
		int pipeFds[2];
		int rv = pipe(pipeFds);
		if (rv) {
			perror("error");
			return exitCode;
		}

		int cpid2;
		// sub-parent
		if ((cpid2 = fork())) {
			int status;
			waitpid(cpid2, &status, 0);
			close(pipeFds[1]);
			// connects the ends of the pipe
			dup2(pipeFds[0], 0);
			exit(execute_ast(astR));
		}
		// sub-child
		else {
			close(pipeFds[0]);
			// duplicates the write end of the pipe to stdout
			dup2(pipeFds[1], 1);
			exit(execute_ast(astL));
		}
	}

	// OLD ATTEMPT WITHOUT DOUBLE FORK
	/*
	int cpid;
	// parent
	if ((cpid = fork())) {
		int status;
		waitpid(cpid, &status, 0);
		close(pipeFds[1]);
		// connects the ends of the pipe
		dup2(pipeFds[0], 0);
		execute_ast(astR);
		WEXITSTATUS(status);
	}
	// child
	else {
		close(pipeFds[0]);
		// duplicates the write end of the pipe to stdout
		dup2(pipeFds[1], 1);
		exit(execute_ast(astL));
	}
*/
}

// Executes left redirection operations 
int execute_left_arrow(AstNode* astL, AstNode* astR) {
	int cpid;
	// Parent process
	if ((cpid = fork())) {
		int status;
		waitpid(cpid, &status, 0);
		return WEXITSTATUS(status);
	}
	// Child process
	else {
		if (!astR->instructionTokens || !astR->instructionTokens->size) {
			printf("error: no file to redirect left\n");
			exit(1);
		}

		// Opens the file as readonly
		int fd = open(svec_get(astR->instructionTokens, 0), O_RDONLY, 0444);
		if (fd == -1) {
			perror(svec_get(astR->instructionTokens, 0));
			exit(1);
		}

		// replaces stdin with the file 
		dup2(fd, 0);
		close(fd);
		exit(execute_ast(astL));
	}
}

// Executes right redirection operations
// (Virtually identical to left redirection)
int execute_right_arrow(AstNode* astL, AstNode* astR) {
	int cpid;
	// Parent process
	if ((cpid = fork())) {
		int status;
		waitpid(cpid, &status, 0);
		return WEXITSTATUS(status);
	}
	// Child process
	else {
		if (!astR->instructionTokens || !astR->instructionTokens->size) {
			printf("error: no file to redirect right\n");
			exit(1);
		}

		// Opens the file as writeonly, but will create if needed
		int fd = open(svec_get(astR->instructionTokens, 0), O_CREAT | O_WRONLY, 0644);
		if (fd == -1) {
			perror(svec_get(astR->instructionTokens, 0));
			exit(1);
		}

		// replaces stdout with the file 
		dup2(fd, 1);
		close(fd);
		exit(execute_ast(astL));
	}
}

// Handles conditional operation AND
int execute_and(AstNode* astL, AstNode* astR) {
	int cpid;
	// Parent process
	if ((cpid = fork())) {
		int status;
		waitpid(cpid, &status, 0);
		// For some godforsaken reason, true is 0 and false is 1 for this syscall
		// I mean I get it makes sense because of the exit codes but this was annoying 
		if (!WEXITSTATUS(status)) {
			return execute_ast(astR);
		}
		return status;
	}
	// Child process
	else {
		exit(execute_ast(astL));
	}
}

// Handles conditional operation OR
// (I definitely copied and pasted AND for this lmao)
int execute_or(AstNode* astL, AstNode* astR) {
	int cpid;
	// Parent process
	if ((cpid = fork())) {
		int status;
		waitpid(cpid, &status, 0);
		// For some godforsaken reason, true is 0 and false is 1 for this syscall
		// I mean I get it makes sense because of the exit codes but this was annoying 
		if (!WEXITSTATUS(status)) {
			// Short circuits because it's already true so this next operation is irrelevant
			return 0;
		}
		return execute_ast(astR);
	}
	// Child process
	else {
		exit(execute_ast(astL));
	}
}

// Handles the special operations of the AST
int execute_operations(AstNode* ast) {
	// General error if empty AST
	if (!ast) {
		return 1;
	}

	// This uhh shouldn't happen but I'm paranoid
	if (!ast->operationToken || *ast->operationToken == '\0') {
		return 1;
	}

	// Operator to compare to
	char* op = strdup(ast->operationToken);

	// Now this is what I call a beautiful, clean finite state machine lmfao
	if (!strcmp(op, "=")) {			// Variable assignment
		free(op);
		op = NULL;
		return execute_assignment(ast->left, ast->right);
	}
	else if (!strcmp(op, ";")) {	// Command separator
		free(op);
		op = NULL;
		return execute_semicolon(ast->left, ast->right);
	}
	else if (!strcmp(op, "\\")) {	// Newline ignorer (TODO)
		free(op);
		op = NULL;
		return execute_back_slash(ast->left, ast->right);
	}
	else if (!strcmp(op, "&")) {	// Backgrounder
		free(op);
		op = NULL;
		return execute_background(ast->left, ast->right);
	}
	else if (!strcmp(op, "|")) {	// Pipe
		free(op);
		op = NULL;
		return execute_pipe(ast->left, ast->right);
	}
	else if (!strcmp(op, "<")) {	// Left redirect
		free(op);
		op = NULL;
		return execute_left_arrow(ast->left, ast->right);
	}
	else if (!strcmp(op, ">")) {	// Right redirect
		free(op);
		op = NULL;
		return execute_right_arrow(ast->left, ast->right);
	}
	else if (!strcmp(op, "&&")) {	// AND operator
		free(op);
		op = NULL;
		return execute_and(ast->left, ast->right);
	}
	else if (!strcmp(op, "||")) {	// OR operator
		free(op);
		op = NULL;
		return execute_or(ast->left, ast->right);
	}
	else if (*op = '"') {			// Quote handler (TODO: probably not the right space for it)
		free(op);
		op = NULL;
		// return execute_quote(ast->left, ast->right);
	}
	else if (*op == '(') {			// Parentheses handler (TODO: also probably not the right spot)
		free(op);
		op = NULL;
		// TODO: I think you can do this by taking the token itself and remove the first and
		// last chars and re-parsing it and re-executing that new ast. I will take care of
		// that if I have time 
	}

	// If you use an incorrect operator somehow
	printf("Error: Invalid operator\n");
	free(op);
	op = NULL;
	return 1;
}

//////////////////// GENERAL EXECUTION HANDLING ////////////////////

// Interprets and executes the parsed AST
int execute_ast(AstNode* ast) {
	
	// General error if empty AST
	if (!ast) {
		return 1;
	}

	//////////////////// BASE INSTRUCTION EXECUTION ////////////////////

	// If no unix operations, execute this instruction
	if (!ast->operationToken || *ast->operationToken == '\0') {
		// Sometimes you'll just get to a case where you don't have to execute anything
		if (!ast->instructionTokens || !ast->instructionTokens->size) {
			// Do nothing successfully
			return 0;
		}

		
		////////// POSIX-SPECIFIC INSTRUCTIONS //////////

		// Manual exit, just so the user can quit this program if they desire 
		if (!strcmp(svec_get(ast->instructionTokens, 0), "exit")) {
			if (ast->instructionTokens->size > 1) {
				perror("exit: too many arguments\n");
				return 1;
			}
			else {
				return exitCode;
			}
		}

		// Looks like I need to take care of CD as well
		if (!strcmp(svec_get(ast->instructionTokens, 0), "cd")) {
			if (ast->instructionTokens->size == 2) {
				if (!chdir(svec_get(ast->instructionTokens, 1))) {
					return 0;
				}
				perror("cd: ");
			}
			// TODO: consider adding support for cd to move to ~ with no args
			// -- also there's a 3 arg version of cd apparently?
			else {
				perror("cd: ");
			}
			return 1;
		}

		// retreives variables 
		for (int ii = 0; ii < ast->instructionTokens->size; ii++) {
			if (*svec_get(ast->instructionTokens, ii) == '$') {
				// Grabs the value from the association vector and swaps
				char* varName = svec_get(ast->instructionTokens, ii) + 1;
				int index = svec_find(variableNames, varName);
				if (index == -1) {
					svec_put(ast->instructionTokens, ii, " \0");
				}
				else {
					svec_put(ast->instructionTokens, ii, svec_get(variableValues, index));
				}
			}
		}
		
		////////// GENERAL PROGRAM INSTRUCTIONS //////////

		// Forks the processes
		int cpid;
		// Parent Process
		if ((cpid = fork())) {
			// Taken from fork sample
			int status;
			waitpid(cpid, &status, 0);
			return WEXITSTATUS(status);
		}
		// Child process
		else{ 
			// I initially did this in its own separate function, but it's probably just better
			// to do it here so I don't have to do a slow malloc operation
			int argc = ast->instructionTokens->size; 
			char* argv[argc + 1];
			for (int ii = 0; ii < argc; ii++) {
				argv[ii] = svec_get(ast->instructionTokens, ii);
			}

			// I don't exactly remember why we needed to terminate the argv like this but we
			// did it in class so I trust you prof Nat <3
			argv[argc] = 0;

			// The child *should* die here
			// free_ast(ast);
			execvp(argv[0], argv);

			// Yell at you if the child is still alive
			printf("error[%d] - %s: %s\n", errno, argv[0], strerror(errno));
			exit(1);
		}
	}
	
	//////////////////// SPECIAL OPERATION EXECUTION ////////////////////

	// Recurs down the AST if there are special operators
	return execute_operations(ast);	

}

// Parses and executes the command string
void execute(char* cmd) {

	// Runs the parsers to turn the command into a list of tokens and then an
	// Arbitrary Syntax Tree (AST) to execute
	svec* tokens = tokenize(cmd);
	AstNode* commandTree = parse_tokens(tokens);

	// The magic happens in here
	int status = execute_ast(commandTree);

	// No leaks pls
	free_ast(commandTree);
	commandTree = NULL;
	
	// Exits when receives -1 signal
	if (status == exitCode) {
		// exits successfully
		exit(0);
	}

}

///////////////////
// Main function //
///////////////////

// Main execution loop yee haw
int main(int argc, char* argv[]) {
	// Instantiates the command (buffer size is 0x1000, that's probably plenty)
	char* cmd = malloc(bufferSize);

	// Initializes the variable names and values association vectors
	variableNames = make_svec();
	variableValues = make_svec();

	// If launching the program standalone (OR USING A REDIRECT OPERATOR)
	if (argc == 1) {
		while (1) {
			// Ensures that the input command is valid
			printf("spinch$ ");
			fflush(stdout);
			fgets(cmd, bufferSize, stdin);
			
			// Detects EOF and bails.
			if (feof(stdin)) {
				// I'm sure there isn't going to be a significant memory leak between EOF and
				// exiting but I mean better to be consistent
				free(cmd);
				cmd = NULL;
				exit(0);
			}	

			// TODO: Take care of \ here and append that to the command otherwise?
			execute(cmd);
			cmd[0] = '\0';
		}
	}
	// args input from command line from file (WITHOUT REDIRECT OPERATOR)
	else {
		// Opens the input file
		svec* inputs = make_svec();
		FILE* inputFile = fopen(argv[1], "r");

		// Reads the lines of the input file
		int size = 0;
		int running = 1;
		while (running) {
			// Gets every line of the file and places them into the multi-inputs vector
			size = getline(&cmd, (size_t*)&bufferSize, inputFile);
			if (size != -1) {
				cmd[size] = '\0';
				svec_push_back(inputs, cmd);
			}
			// When EOF, exit
			else {
				running = 0;
			}
		}
		// Closes and executes the vector of commands
		fclose(inputFile);	
		for (int ii = 0; ii < inputs->size; ii++) {
			execute(svec_get(inputs, ii));
		}

		// Miss me with that leaky memory
		free_svec(inputs);
		free(cmd);
		cmd = NULL;
	}
	// Exited successfully
	return 0;
}
