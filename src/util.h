#ifndef UTIL_C
#define UTIL_C

#include <stdbool.h>

bool util_downloadURL(const char *url, const char *filename);

// Read entire file entire buffer
// Note: caller must free(buffer)
char* util_readEntireFile(const char *filename);

#endif
