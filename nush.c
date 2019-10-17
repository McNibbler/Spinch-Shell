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

int exitCode = -1;	// I think this is right but I have to check

///////////////////////////
// Function Declarations //
///////////////////////////

// Interprets and executes the parsed AST
int execute_ast(AstNode* ast);

// Parses and executes the command string
void execute(char* cmd);

/////////////////////////////
// Function Implementation //
/////////////////////////////

// Interprets and executes the parsed AST
int execute_ast(AstNode* ast) {
	// General error if empty AST
	if (!ast) {
		return 1;
	}

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
				return -1;
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
			if (WIFEXITED(status)) {
				return WEXITSTATUS(status);
			}
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
		
		
		return 0;
	}
	return 1;
}

// Parses and executes the command string
void execute(char* cmd) {
	// if (!strcmp(cmd, "exit\n")) {
	// 	exit(0);
	// }
	// printf("command: %s\n", cmd);

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

// Executes the received command gamer style
/*
void execute(char* cmd) {

	svec* tokens = tokenize(cmd);

	int cpid;
	int exitCode = 0;

	// Parent process
	if ((cpid = fork())) {
		// parent process
		// printf("Parent pid: %d\n", getpid());
		// printf("Parent knows child pid: %d\n", cpid);

		// Waits for the child process to terminate
		int status;
		waitpid(cpid, &status, 0);

		if (WIFEXITED(status)) {
			printf("child exited with exit code (or main returned) %d\n", WEXITSTATUS(status));
		}
	}
	// TODO: Child process
	else {
		// child process
		// printf("Child pid: %d\n", getpid());
		// printf("Child knows parent pid: %d\n", getppid());

		// Executes the individual tokens
		for (int ii = 0; ii < tokens->size; ii++) {
			char* currentToken = tokens->data[ii];
			
			// Exit command
			if (!strcmp(currentToken, "exit")) {
				exit(0);
			}
			else if (!strcmp(currentToken, ";")) {

			}
			else if (!strcmp(currentToken, "<")) {

			}
			else if (!strcmp(currentToken, ">")) {

			}
			else if (!strcmp(currentToken, "|")) {

			}
			else if (!strcmp(currentToken, "&")) {

			}
			else if (!strcmp(currentToken, "||")) {

			}
			else if (!strcmp(currentToken, "&&")) {

			}
			else if (*currentToken == '(') {

			}
			else if (*currentToken = '"') {

			}
			else {

			}
		}

		// The argv array for the child.
		// Terminated by a null pointer.
		char* args[] = {cmd, "one", 0};

		printf("== executed program's output: ==\n");

		execvp(cmd, args);
		printf("Can't get here, exec only returns on error.");
	}
}
*/

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
