#ifndef UTIL_C
#define UTIL_C

#include <stdbool.h>

bool util_downloadURL(char *url, char *filename);

// Read entire file entire buffer
// Note: caller must free(buffer)
char* util_readEntireFile(char *filename);

#endif
