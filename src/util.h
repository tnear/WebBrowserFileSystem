#ifndef UTIL_C
#define UTIL_C

#include <stdbool.h>
#include <stddef.h>

bool util_downloadURL(const char *url, const char *filename);

// Read entire file entire buffer
// Note: caller must free(buffer)
char* util_readEntireFile(const char *filename);

void util_getMountPoint(char *mountPoint, size_t size, int argc, char *argv[]);

#endif
