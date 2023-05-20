#include "operations.h"
#include "fuseData.h"
#include "linkedList.h"
#include "sqlite.h"
#include "util.h"
#include "website.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <curl/curl.h>

int operations_getattr(const char *path, struct stat *stbuf, FuseData *fuseData)
{
    if (strcmp(path, "/") == 0)
    {
        // for root path, set permissions then exit
        stbuf->st_mode = S_IFDIR | 0400;
        return 0;
    }

    char url[PATH_MAX] = {};
    getURL(url, path, fuseData);

    if (!util_isURL(url))
    {
        // not URL, don't waste time networking
        return 0;
    }

    CURLcode curlStatus = CURLE_OK;
    size_t fileSize = -1;
    Website *website = lookupWebsite(fuseData->db, url);
    if (website)
    {
        fileSize = strlen(website->html);
    }
    else
    {
        char filename[PATH_MAX] = {};
        util_urlToFileName(filename, url);

        curlStatus = util_downloadURL(url, filename);
        if (curlStatus != CURLE_OK)
        {
            // cannot download, return error
            return curlStatus;
        }

        printf(url);
        printf("\n");

        // read into buffer
        char *contents = util_readEntireFile(filename);
        fileSize = strlen(contents);

        // add to database
        website = initWebsite(url, filename, contents);
        insertWebsite(fuseData->db, website);

        // remove temp file
        remove(filename);

        // free memory
        free(contents);
    }

    // todo: save html length in database?
    assert(fileSize != -1);
    stbuf->st_size = fileSize;
    stbuf->st_mode = regular_file.st_mode;
    // set timestamp to current time      
    stbuf->st_mtime = time(NULL);

    // cleanup
    assert(website);
    freeWebsite(website);

    return curlStatus;
}

int operations_readdir(const char *path, void *buf, fill_dir_t filler,
    off_t offset, FuseData *fuseData)
{
    if (strcmp(path, "/") != 0)
    {
        // ignore non-root files
        return 0;
    }

    // Root
    const off_t zeroOffset = 0;

    // fill all files added this session
    Node *current = fuseData->llHead;
    while (current)
    {
        filler(buf, current->filename, &regular_file, zeroOffset);
        current = current->next;
    }

    return 0;
}

int operations_read(const char *fusePath, char *buf, size_t size,
    off_t offset, FuseData *fuseData)
{
    char url[PATH_MAX] = {};
    getURL(url, fusePath, fuseData);

    char filename[PATH_MAX] = {};
    util_urlToFileName(filename, url);
    int len = 0;

    assert(llContainsString(fuseData->llHead, filename));
    util_downloadURL(url, filename);

    // Read file into buffer
    char *contents = util_readEntireFile(filename);
    remove(filename);
    
    // Copy to buffer
    strcpy(buf, contents);
    len = strlen(contents);
    free(contents);

    return len;
}

void getURL(char *url, const char *fusePath, FuseData *fuseData)
{
    char *pathNoSlash = NULL;
    // Website *website = lookupWebsite(fuseData->db, fusePath + 1);
    Node *existingNode = llFindNode(fuseData->llHead, fusePath + 1);
    if (existingNode)
    {
        pathNoSlash = strdup(existingNode->url);
        //pathNoSlash = strdup(website->url);
    }
    else
    {
        // ex: path: "/example.com"
        // ex: pathNoSlash: "example.com"
        char *pathCopy = strdup(fusePath);
        // '/' is not allowed in file name, so have user type in '\'. Replace it here:
        util_replaceChar(pathCopy, '\\', '/');
        pathNoSlash = strdup(pathCopy + 1);
        free(pathCopy);
    }

    int len = strlen(pathNoSlash);
    assert(len <= PATH_MAX);
    memcpy(url, pathNoSlash, len);
}
