#ifndef UTIL_C
#define UTIL_C

#include <stdbool.h>
#include <stddef.h>
#include <curl/curl.h>

#define BYTE_SIZE_CAP 50000
#define BYTE_SIZE_PREVIEW 100

CURLcode util_downloadURL(const char *url, const char *filename);
int getUrlContentLength(const char *url);
CURLcode getFirst100Bytes(char *data, const char *url);

// Read entire file entire buffer
// Note: caller must free(buffer)
char* util_readEntireFile(const char *filename);

bool util_isURL(const char *url);
void util_urlToFileName(char *filename, const char *url);
void util_replaceChar(char *str, char orig, char rep);

#endif
