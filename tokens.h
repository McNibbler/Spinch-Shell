// Tokenization of user command inputs
// Thomas Kaunzinger

#ifndef TOKENS_H
#define TOKENS_H

// Relevant library imports
#include "svec.h"

// Global buffer size - 4096 chars is probably beyond sufficient for most commands, provided
// -- typical user input. May consider dynamically allocating in future.
extern int bufferSize;

// Parses command char array into a svec of tokens split along spaces and operations
svec* tokenize(char* command);

#endif
