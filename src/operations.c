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
#include <unistd.h>

int operations_getattr(const char *path, struct stat *stbuf, FuseData *fuseData)
{
    if (strcmp(path, "/") == 0)
    {
        // Root path
        stbuf->st_mode = S_IFDIR | 0400;
        return 0;
    }

    char *url = getURL(path, fuseData);

    if (!util_isURL(url))
    {
        // not URL, don't waste time networking
        return 0;
    }

    char filename[PATH_MAX] = {};
    util_urlToFileName(filename, url);

    const bool isValidURL = util_downloadURL(url, filename);
    if (isValidURL)
    {
        printf(url);
        printf("\n");
        llInsertNodeIfDoesntExist(&fuseData->llHead, filename, url);

        // Read into buffer
        char *contents = util_readEntireFile(filename);

        // copy file attributes to stbuf
        struct stat localBuf;
        int ret = stat(filename, &localBuf);
        assert(ret == 0);
        memcpy(stbuf, &localBuf, sizeof(struct stat));

        // remove file
        remove(filename);

        stbuf->st_size = strlen(contents);
        stbuf->st_mode = regular_file.st_mode;
        free(contents);
    }

    free(url);
    return 0;
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
    char *url = getURL(fusePath, fuseData);

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

    free(url);
    return len;
}

char *getURL(const char *fusePath, FuseData *fuseData)
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

    return pathNoSlash;
}
