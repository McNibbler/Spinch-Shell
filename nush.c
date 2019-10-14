// Nush Shell (To be rebranded soon)
// An ultra-lightweight POSIX-style shell
// Thomas Kaunzinger - Fall 2019

// Relevant library imports
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tokens.h"
#include "svec.h"
#include "ast.h"

// User variables stored here
// TODO: Update my hashmap implementation so that it uses arbitrary length string keys so that
// I no longer have to use nasty linear time association vectors just to store variables.
svec* variableNames;
svec* variableValues;


// Executest the received command gamer style
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
		if (cmd[strlen(cmd) - 1] == '\\') {
			continue;
		}

		// TODO: Take care of \ here and append that to the command otherwise?
		fflush(stdout);
		execute(cmd);
		cmd[0] = '\0';
	}

	// I guess this is technically unreachable
	return 0;
}
