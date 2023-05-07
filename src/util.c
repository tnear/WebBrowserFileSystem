#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

// very simple utility to determine quickly if a string might be a URL
bool util_isURL(const char *url)
{
    if (url[0] == '.')
        return false; // ignore hidden files

    char dot[] = ".";
    char *ret = strstr(url, dot);
    if (!ret)
        return false; // URLs must contain a '.'

    return true;
}

// Ex for '/': "abc//" => "abc"
static void removeTrailingChar(char *str, char charToRemove)
{
    while (1)
    {
        int len = strlen(str);
        // Get last matching character
        char *lastSlashLocation = strrchr(str, charToRemove);
        if (lastSlashLocation == str + len - 1)
        {
            // remove forbidden character
            str[len - 1] = '\0';
        }
        else
            break;
    }
}

void util_urlToFileName(char *filename, const char *inputURL)
{
    // copy string
    char *url = strdup(inputURL);

    // Remove trailing slashes
    removeTrailingChar(url, '/');

    const char *startingPoint = NULL;
    char *lastSlashLocation = strrchr(url, '/');
    if (lastSlashLocation)
    {
        // trim after last slash
        startingPoint = lastSlashLocation + 1;
    }
    else
    {
        startingPoint = url;
    }

    assert(startingPoint);
    memcpy(filename, startingPoint, strlen(startingPoint));

    // cleanup
    free(url);
}
