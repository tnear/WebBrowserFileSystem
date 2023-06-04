#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <bits/stdint-uintn.h>

#define MAX_URL_LENGTH 256

bool util_isS3(const char *url)
{
    char s3prefix[] = "s3://";
    int lenPrefix = strlen(s3prefix);
    if (strlen(url) < lenPrefix)
        return false;

    bool isS3 = strncmp(url, s3prefix, lenPrefix) == 0;
    return isS3;
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

    char bucket[MAX_URL_LENGTH] = {};
    getBucketFromS3(bucket, s3);

    // ex: "s3://my-bucket/dir/file.html"
    // pathStart will point to the 'd' in 'dir'
    char *pathStart = strstr(s3, bucket);
    pathStart += strlen(bucket); // consume bucket
    pathStart += 1;              // consume slash

    //strcpy(path, pathStart);
    strncpy(path, pathStart, strlen(s3) - (pathStart - s3) + 1);
}

// ex: convert: "s3://bucket/file.html"
// to: "https://bucket.s3.amazonaws.com/file.html"
void convertS3intoURL(char *url, const char *s3)
{
    assert(util_isS3(s3));
    assert(strlen(url) == 0);

    char bucket[MAX_URL_LENGTH] = {};
    char path[MAX_URL_LENGTH] = {};
    getBucketFromS3(bucket, s3);
    getPathFromS3(path, s3);

    sprintf(url, "https://%s.s3.amazonaws.com/%s", bucket, path);
}

// Fuzzing entry point
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Call the function to be fuzzed
    if (size <= MAX_URL_LENGTH)
    {
        if (size < 60)
            return 0;

        char s3prefix[] = "s3://";
        char urlToUse[MAX_URL_LENGTH + 5] = {};
        char url[MAX_URL_LENGTH + 5];
        strcpy(url, s3prefix);
        strncpy(url + 5, (const char *)data, size);
        url[size + 5] = '\0';

        // Call the function to be fuzzed
        convertS3intoURL(urlToUse, url);
    }
    else
    {
        exit(-1);
    }

    return 0;
}
