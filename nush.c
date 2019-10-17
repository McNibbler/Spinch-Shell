//////////////////////////////////////////////
// Nush Shell (To be rebranded soon)		//
// An ultra-lightweight POSIX-style shell	//
// Thomas Kaunzinger - Fall 2019			//
//////////////////////////////////////////////

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
// I no longer have to use nasty linear time association vectors just to store variables.
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
int execute_var_call(AstNode* astL, AstNode* astR);			// $
int execute_semicolon(AstNode* astL, AstNode* astR);		// ;
int execute_forward_slash(AstNode* astL, AstNode* astR);	// \     //
int execute_background(AstNode* astL, AstNode* astR);		// &
int execute_pipe(AstNode* astL, AstNode* astR);				// |
int execute_left_arrow(AstNode* astL, AstNode* astR);		// <
int execute_right_arrow(AstNode* astL, AstNode* astR);		// >
int execute_and(AstNode* astL, AstNode* astR);				// &&
int execute_or(AstNode* astL, AstNode* astR);				// ||
int execute_quote(AstNode* astL, AstNode* astR);			// "

/////////////////////////////
// Function Implementation //
/////////////////////////////

//////////////////// POSIX SPECIAL OPERATOR HANDLING ////////////////////

// Evaluates the semicolon operator
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

// TODO: this one seems important later
int execute_forward_slash(AstNode* astL, AstNode* astR) {
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

// TODO: FINISH THIS SHIT DAWWWWWGGG IT'S 7:30AM AND I NEED TO SLEEEEEEEEEEEEEEEEEEEEP
int execute_pipe(AstNode* astL, AstNode* astR) {

	return exitCode;

	// Generates the pipe file descriptors
	int pipeFds[2];
	int rv = pipe(pipeFds);
	if (rv != 0) {
		perror("error");
		return exitCode;
	}

	int cpid;
	// Parent process
	if ((cpid = fork())) {
		int status;
		waitpid(cpid, &status, 0);
		return WEXITSTATUS(status);
	}
	// Child process
	else {
		if (!astR->instructionTokens || !astR->instructionTokens->size
				|| !astL->instructionTokens || !astL->instructionTokens->size) {
			printf("error: missing pipe arguments\n");
			exit(1);
		}

		// replaces stdin with the file 
		// dup2(fd, 0);
		// close(fd);
		exit(execute_ast(astL));
	}
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

	char* op = strdup(ast->operationToken);

	if (!strcmp(op, "=")) {
		// return execute_assignment(ast->left, ast->right);
	}
	else if (!strcmp(op, "$")) {
		// return execute_var_call(ast->left, ast->right);
	}
	else if (!strcmp(op, ";")) {
		return execute_semicolon(ast->left, ast->right);
	}
	else if (!strcmp(op, "\\")) {
		return execute_forward_slash(ast->left, ast->right);
	}
	else if (!strcmp(op, "&")) {
		return execute_background(ast->left, ast->right);
	}
	else if (!strcmp(op, "|")) {
		return execute_pipe(ast->left, ast->right);
	}
	else if (!strcmp(op, "<")) {
		return execute_left_arrow(ast->left, ast->right);
	}
	else if (!strcmp(op, ">")) {
		return execute_right_arrow(ast->left, ast->right);
	}
	else if (!strcmp(op, "&&")) {
		return execute_and(ast->left, ast->right);
	}
	else if (!strcmp(op, "||")) {
		return execute_or(ast->left, ast->right);
	}
	else if (*op = '"') {
		// return execute_quote(ast->left, ast->right);
	}
	else if (*op == '(') {
		// TODO: I think you can do this by taking the token itself and remove the first and
		// last chars and re-parsing it and re-executing that new ast. I will take care of
		// that if I have time 
	}

	printf("Error: Invalid operator\n");
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
			execvp(argv[0], argv);

			// Yell at you if the child is still alive
			printf("error[%d] - %s: %s\n", errno, argv[0], strerror(errno));
			exit(1);
		}
	}
	
	//////////////////// SPECIAL OPERATION EXECUTION ////////////////////

	return execute_operations(ast);	

}

// Parses and executes the command string
void execute(char* cmd) {

	// Runs the parsers to turn the command into a list of tokens and then an
	// Arbitrary Syntax Tree (AST) to execute
	svec* tokens = tokenize(cmd);
	AstNode* commandTree = parse_tokens(tokens);
	int status = execute_ast(commandTree);
	// free_svec(tokens);
	// free_ast(commandTree);
	
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
	char cmd[bufferSize];
	variableNames = make_svec();
	variableValues = make_svec();

	while (1) {
		// Ensures that the input command is valid
		if (argc == 1) {
			printf("nush$ ");
			fflush(stdout);
			fgets(cmd, bufferSize, stdin);

		}
		// Execute... virtually nothing
		else {
			memcpy(cmd, "echo", 5);
		}
		
		// Keeps growing that command if goes to next line
		// if (cmd[strlen(cmd) - 1] == '\\') {
		// 	continue;
		// }

		// TODO: Take care of \ here and append that to the command otherwise?
		fflush(stdout);
		execute(cmd);
		cmd[0] = '\0';
	}

	// I guess this is technically unreachable
	return 0;
}
