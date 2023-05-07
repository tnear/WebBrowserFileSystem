#include "operations.h"
#include "linkedList.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int operations_getattr(const char *path, struct stat *stbuf, Node **llHead)
{
    if (strcmp(path, "/") == 0)
    {
        // Root path
        stbuf->st_mode = S_IFDIR | 0400;
        return 0;
    }

    Node *existingNode = llFindNode(*llHead, path + 1);
    const char *pathNoSlash = NULL;
    char *pathCopy = NULL;
    if (existingNode)
    {
        pathNoSlash = existingNode->url;
    }
    else
    {
        pathCopy = strdup(path);
        // '/' is not allowed in file name, so have user type in '\'. Replace it here:
        util_replaceChar(pathCopy, '\\', '/');

        pathNoSlash = pathCopy + 1; // Account for leading "/", ex: "/file.html"
    }

    if (!util_isURL(pathNoSlash))
    {
        // not URL, don't waste time networking
        return 0;
    }

    char filename[PATH_MAX] = {};
    util_urlToFileName(filename, pathNoSlash);

    const bool isValidURL = util_downloadURL(pathNoSlash, filename);
    if (isValidURL)
    {
        printf(pathNoSlash);
        printf("\n");
        llInsertNodeIfDoesntExist(llHead, filename, pathNoSlash);

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

    free(pathCopy);
    return 0;
}

int operations_readdir(const char *path, void *buf, fill_dir_t filler,
    off_t offset, struct Node *llHead)
{
    if (strcmp(path, "/") != 0)
    {
        // ignore non-root files
        return 0;
    }

    // Root
    const off_t zeroOffset = 0;

    // fill all files added this session
    Node *current = llHead;
    while (current)
    {
        filler(buf, current->filename, &regular_file, zeroOffset);
        current = current->next;
    }

    return 0;
}

int operations_read(const char *path, char *buf, size_t size,
    off_t offset, struct Node *llHead)
{
    Node *existingNode = llFindNode(llHead, path + 1);
    const char *pathNoSlash = NULL;
    char *pathCopy = NULL;
    if (existingNode)
    {
        pathNoSlash = existingNode->url;
    }
    else
    {
        // ex: path: "/example.com"
        // ex: pathNoSlash: "example.com"
        pathCopy = strdup(path);
        // '/' is not allowed in file name, so have user type in '\'. Replace it here:
        util_replaceChar(pathCopy, '\\', '/');
        pathNoSlash = pathCopy + 1;
    }

    char filename[PATH_MAX] = {};
    util_urlToFileName(filename, pathNoSlash);
    int len = 0;

    if (llContainsString(llHead, filename))
    {
        util_downloadURL(pathNoSlash, filename);

        // Read file into buffer
        char *contents = util_readEntireFile(filename);
        remove(filename);
        
        // Copy to buffer
        strcpy(buf, contents);
        len = strlen(contents);
        free(contents);
    }
    else
    {
        // cannot find this file
        len = -ENOENT;
    }

    free(pathCopy);
    return len;
}
