#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

// Note: caller must free(buffer)
char* util_readEntireFile(const char *filename)
{
    char *buffer = NULL;
    int length = 0;
    FILE *file = fopen(filename, "rb");
    if (!file)
        return NULL;

    // Get length
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer
    buffer = malloc(length + 1);

    // Read file
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);

    // Return file data buffer (must call free(buffer))
    return buffer;
}

bool util_downloadURL(const char *url, const char *filename)
{
    // open file for writing
    FILE *file = fopen(filename, "w");
    assert(file);

    // initialize curl
    CURL *curl = curl_easy_init();
    assert(curl);

    // set URL to download
    curl_easy_setopt(curl, CURLOPT_URL, url);

    // set file handle for output
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    // perform the download
    CURLcode res = curl_easy_perform(curl);

    // check for errors
    if (res != CURLE_OK)
    {
        // Delete file for failed downloads
        remove(filename);
        return false;
    }

    // cleanup curl
    curl_easy_cleanup(curl);

    // close file handle
    fclose(file);

    return true;
}