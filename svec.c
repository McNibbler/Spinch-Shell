// Svec data structure src
// Thomas Kaunzinger	

// Imports
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "svec.h"

// Makes an empty svec
svec* make_svec() {
    svec* sv = malloc(sizeof(svec));
    sv->data = malloc(2 * sizeof(char*));
    sv->size = 0;
	sv->capacity = 2;
    return sv;
}

// Frees the memory of the svec
void free_svec(svec* sv) {
	free(sv->data);
	sv->data = NULL;
	free(sv);
	sv = NULL;
}

// Returns the value at the desired svec index
char* svec_get(svec* sv, int ii) {
    assert(ii >= 0 && ii < sv->size);
    return sv->data[ii];
}

// Inserts a duplicated char* into the svec
void svec_put(svec* sv, int ii, char* item) {
    assert(ii >= 0 && ii < sv->size);
    sv->data[ii] = strdup(item);
}

// Returns the index of the item if it exists and -1 otherwise
int svec_find(svec* sv, char* item) {
	for (int ii = 0; ii < sv->size; ii++) {
		if (!strcmp(item, sv->data[ii])) {
			return ii;
		}
	}
	return -1;
}

// Creates a sliced svec along the desired indicies
svec* svec_slice(svec* sv, int startIndex, int endIndex) {
	svec* subSvec = make_svec();
	for (int ii = startIndex; ii < endIndex; ii++) {
		svec_push_back(subSvec, svec_get(sv, ii));
	}
	return subSvec;
}

// Pushes an element to the end of the svec
void svec_push_back(svec* sv, char* item) {
    int ii = sv->size;
	// Grows the vector if needed
	if (sv->capacity <= ii) {
		svec_grow(sv);
	}
    sv->size = ii + 1;
    svec_put(sv, ii, item);
}

// Doubles the vector capacity
void svec_grow(svec* sv) {
	int newCap = sv->capacity * 2;
	char** newData = realloc(sv->data, sizeof(char*) * newCap);
	sv->data = newData;
	sv->capacity = newCap;
}

// Swaps two indicies in a svec
void svec_swap(svec* sv, int ii, int jj) {
    // Ensures that the data isn't OOB
	assert(ii >= 0 && ii < sv->size && jj >= 0 && jj < sv->size);
	char* temp = sv->data[jj];
	sv->data[jj] = sv->data[ii];
	sv->data[ii] = temp;
}

// Prints the contents of the svec
void svec_print(svec* sv) {
	for (int ii = 0; ii < sv->size; ii++) {
		puts(sv->data[ii]);
	}
}

// Reverses the svec
void svec_reverse(svec* sv) {
	// Swaps everything until the middle is reached
	for (int ii = 0; ii < sv->size / 2; ii++) {
		svec_swap(sv, ii, sv->size - ii - 1);
	}
}

// Returns the svec as just an array... well actually a char**
// char** svec_as_array(svec* sv) {
// 	char* running[sv->size];
// 	for (int ii = 0; ii < sv->size; ii++) {
// 		running[ii] = svec_get(sv, ii);
// 	}
// 	return running;
// }

