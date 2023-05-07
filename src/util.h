#ifndef UTIL_C
#define UTIL_C

#include <stdbool.h>
#include <stddef.h>

bool util_downloadURL(const char *url, const char *filename);

// Read entire file entire buffer
// Note: caller must free(buffer)
char* util_readEntireFile(const char *filename);

bool util_isURL(const char *url);
void util_urlToFileName(char *filename, const char *url);
void util_replaceChar(char *str, char orig, char rep);

#endif
