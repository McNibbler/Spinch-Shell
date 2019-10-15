// Svec data structure header
// Thomas Kaunzinger

#ifndef SVEC_H
#define SVEC_H

// Type definition of svec
typedef struct svec {
    int size;
    char** data;
	int capacity;
} svec;

// Svec interface functions
svec* make_svec();
void  free_svec(svec* sv);

char* svec_get(svec* sv, int ii);
void  svec_put(svec* sv, int ii, char* item);
int svec_find(svec* sv, char* item);
svec* svec_slice(svec* sv, int startIndex, int endIndex);

void svec_push_back(svec* sv, char* item);
void svec_swap(svec* sv, int ii, int jj);

void svec_print(svec* sv);
void svec_reverse(svec* sv);

// Added a helper function for helping grow when needed
void svec_grow(svec* sv);

#endif
