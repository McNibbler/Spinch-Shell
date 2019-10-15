// Tokenization of user command inputs
// Thomas Kaunzinger

// Relevant library imports
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tokens.h"
#include "svec.h"

// Sets the global buffer size.
int bufferSize = 0x1000;

// Parses command char array into a svec of tokens split along spaces and operations
svec* tokenize(char* command) {

	// Running token vector that is filled by parser
	svec* tokens = make_svec();
	int runningTokenSize = 0;

	// The running command buffer
	char commandBuffer[bufferSize];
	
	// Runs through the whole of the command char by char
	for (int i = 0; i < strlen(command); i++) {

		// Moves i bytes from the start of the command to read the running character
		char* currentChar = command + i;

		// Is there anything better than finite state machines?
		switch (*currentChar) {

			//////////////////////////////////
			// SPECIAL CHARACTER PROCESSING //
			//////////////////////////////////

			// Parsing on single character redirections
			case '<' :  // Similar token types fall through. This is intentional and
			case '>' :	// -- follows the design pattern for multiple cases in a switch
			case ';' :	// -- with the same behavior.
			case '=' :
			case '$' :
			case '\\' :
				// Null-terminates the running command buffer
				commandBuffer[runningTokenSize] = '\0';

				// Adds token up to this pointif it actually contains something
				if (runningTokenSize) {
					svec_push_back(tokens, commandBuffer);
				}
				
				// Adds the single character redirection char as its own token 
				commandBuffer[0] = *currentChar;
				commandBuffer[1] = '\0';
				svec_push_back(tokens, commandBuffer);
				runningTokenSize = 0;
				break;


			// Parsing on characters that can have double meanings if used twice 
			case '&' :	// Similar token types fall through
			case '|' :
				// Null-terminates the running command buffer
				commandBuffer[runningTokenSize] = '\0';

				// Adds token up to this pointif it actually contains something
				if (runningTokenSize) {
					svec_push_back(tokens, commandBuffer);
				}

				commandBuffer[0] = *currentChar;
				if (*(currentChar + 1) == *currentChar) {
					commandBuffer[1] = *currentChar;
					commandBuffer[2] = '\0';
					i++;
				}
				else {
					commandBuffer[1] = '\0';
				}
				svec_push_back(tokens, commandBuffer);
				runningTokenSize = 0;
				break;


			// List of tokens found on unix.stackexchange.com by question posted by user ilkkachu
			// Parsing on groupings 
			case '(' :	// Similar token types fall through
			// case ')' :
			case '"' :
				// Null-terminates the running command buffer
				commandBuffer[runningTokenSize] = '\0';

				char* begin = currentChar;
				currentChar++;
				while (*currentChar != *begin && *currentChar != '\0') {
					runningTokenSize++;
					currentChar++;
				}
				memcpy(&commandBuffer[0], begin, runningTokenSize);
				commandBuffer[runningTokenSize] = '\0';
				svec_push_back(tokens, commandBuffer);
				i += runningTokenSize;
			 	runningTokenSize = 0;
				break;

			////////////////////////
			// WHITESPACE PARSING //
			////////////////////////

			// Parsing on whitespace
			case '\0' :	// Similar token types fall through 
			case '\t' :
			case '\n' :
			case '\v' :	// Does anybody actually use these
			case '\f' :	// -- whitespace characters anymore?
			case '\r' :
			case ' ' :
				// Null-terminates the running command buffer
				commandBuffer[runningTokenSize] = '\0';
				// Adds the token if it actually contains something
				if (runningTokenSize) {
					svec_push_back(tokens, commandBuffer);
				}
				runningTokenSize = 0;
				break;
				

			//////////////////////////////
			// NORMAL CHARACTER PARSING //
			//////////////////////////////

			// Base case
			default :
				// Adds the current char to the command and increases the command size
				commandBuffer[runningTokenSize] = *currentChar;
				runningTokenSize++;
				break;
		}
	}

	// Returns the parsed token vector
	return tokens;
}

