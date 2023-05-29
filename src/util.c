#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

CURLcode util_downloadURL(const char *url, const char *filename)
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

    // close file handle
    fclose(file);

    // check for errors
    if (res != CURLE_OK)
    {
        // Delete file for failed downloads
        remove(filename);
    }

    // cleanup curl
    curl_easy_cleanup(curl);

    return res;
}

// Struct to store the content size
typedef struct ContentSizeData
{
    int size;
} ContentSizeData;

// Callback function to handle the header data
size_t _headerCallback(char *buffer, size_t size, size_t nitems, void *userData)
{
    // This function is called once for each line of header data.
    // Only update userData for the Content-Length line
    size_t length = size * nitems;
    const char header[] = "Content-Length:";
    const char header2[] = "content-length:"; // todo: case insensitive compare
    // search substring
    char *start = strstr(buffer, header);
    if (!start)
    {
        // some sites return lower case
        start = strstr(buffer, header2);
    }

    if (start)
    {
        // when finding the "Content-Length" substring,
        // get content length from string, ex: 1234 from "content-length: 1234\r\n"
        ContentSizeData *data = (ContentSizeData*) userData;
        long contentSize = strtol(start + strlen(header), NULL, 10);
        data->size = contentSize;
    }

    return length;
}

int getUrlContentLength(const char *url)
{
    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, _headerCallback);

    // Create a ContentSizeData object to store the size
    ContentSizeData contentSizeData;
    contentSizeData.size = 0;

    // Get header data only
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &contentSizeData);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

    CURLcode res = curl_easy_perform(curl);

    // cleanup
    curl_easy_cleanup(curl);

    // return page size (or 0 if it has no content-length set)
    return contentSizeData.size;
}

size_t _writeCallback(char* buffer, size_t size, size_t nitems, void* userData)
{
    size_t length = size * nitems;
    char *response = (char*)userData;

    // Copy the first BYTE_SIZE_PREVIEW bytes of the response
    strncpy(response, buffer, BYTE_SIZE_PREVIEW);
    response[BYTE_SIZE_PREVIEW] = '\0'; // Null-terminate the string

    return length;
}

CURLcode getFirst100Bytes(char *data, const char *url)
{
    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _writeCallback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
    curl_easy_setopt(curl, CURLOPT_RANGE, "0-99"); // Specify the range to retrieve

    CURLcode res = curl_easy_perform(curl);

    // cleanup
    curl_easy_cleanup(curl);

    return res;
}

// very simple utility to determine quickly if a string might be a URL
bool util_isURL(const char *url)
{
    int len = strlen(url);
    if (len == 0)
        return false;
    else if (url[0] == '.')
        return false; // ignore hidden files
    else if (util_isS3(url))
        return true; // s3 (public) buckets are supported

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
    strncpy(filename, startingPoint, strlen(startingPoint));

    // cleanup
    free(url);
}

// replaces all occurrences of 'orig' with 'rep'
void util_replaceChar(char *str, char orig, char rep)
{
    char *ptr = str;
    while ((ptr = strchr(ptr, orig)))
    {
        *ptr++ = rep;
    }
}

bool util_isS3(const char *url)
{
    char s3prefix[] = "s3://";
    int lenPrefix = strlen(s3prefix);
    if (strlen(url) < lenPrefix)
        return false;

    bool isS3 = strncmp(url, s3prefix, lenPrefix) == 0;
    return isS3;
}

// ex: s3 = "s3://my-bucket/file.html"
// this function assigns first parameter to "my-bucket"
void getBucketFromS3(char *bucket, const char *s3)
{
    assert(util_isS3(s3));
    assert(strlen(bucket) == 0); // ensure bucket is empty

    char s3prefix[] = "s3://";
    int len = strlen(s3prefix);

    // skip past prefix:
    // ex: "s3://my-bucket/file.html" => "my-bucket/file.html"
    const char *bucketStart = s3 + len;
    assert(bucketStart);

    // find first '/'
    const char *slashLocation = strchr(bucketStart, '/');
    if (!slashLocation)
    {
        // no slash, just use end of string
        slashLocation = bucketStart + strlen(bucketStart);
    }

    int dist = slashLocation - bucketStart;

    // copy between s3 prefix and '/' to retrieve bucket name
    strncpy(bucket, bucketStart, dist);
}

// ex: s3 = "s3://my-bucket/dir/file.html"
// => path = "dir/file.html"
void getPathFromS3(char *path, const char *s3)
{
    assert(util_isS3(s3));

    char bucket[FUSE_PATH_MAX] = {};
    getBucketFromS3(bucket, s3);

    // ex: "s3://my-bucket/dir/file.html"
    // pathStart will point to the 'd' in 'dir'
    char *pathStart = strstr(s3, bucket);
    pathStart += strlen(bucket); // consume bucket
    pathStart += 1; // consume slash

    strcpy(path, pathStart);
}

// ex: convert: "s3://bucket/file.html"
// to: "https://bucket.s3.us-east-2.amazonaws.com/file.html"
void convertS3intoURL(char *url, const char *s3)
{
    assert(util_isS3(s3));
    assert(strlen(url) == 0);

    char bucket[FUSE_PATH_MAX] = {};
    char path[FUSE_PATH_MAX] = {};
    getBucketFromS3(bucket, s3);
    getPathFromS3(path, s3);

    sprintf(url, "https://%s.s3.us-east-2.amazonaws.com/%s", bucket, path);
}
