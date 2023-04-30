#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <curl/curl.h>

// todo: unify these
static char myText[] = "Hello world3!\n";
static char HTML_FILE[] = "example.html";
static char HTML_FILE_PATH[] = "/example.html";


void util_print()
{
    printf("util_print\n");
}

void util_downloadURL(char *url)
{
    // open file for writing
    FILE *file = fopen(HTML_FILE, "w");
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
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // cleanup curl
    curl_easy_cleanup(curl);

    // close file handle
    fclose(file);
}