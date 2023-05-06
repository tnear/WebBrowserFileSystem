#include "operations.h"
#include "linkedList.h"
#include "util.h"
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
    
    const char *pathNoSlash = path + 1; // Account for leading "/", ex: "/file.html"
    const bool fileExists = access(pathNoSlash, F_OK) == 0;
    if (fileExists)
    {
        // get file metadata
        struct stat localBuf;
	    int ret = stat(pathNoSlash, &localBuf);

        // deep copy to stbuf argument
        memcpy(stbuf, &localBuf, sizeof(struct stat));

        stbuf->st_mode = regular_file.st_mode;
        return ret;
    }

    const bool isValidURL = util_downloadURL(pathNoSlash, pathNoSlash);
    if (!isValidURL)
    {
        // nothing to add
        return 0;
    }

    llInsertNodeIfDoesntExist(llHead, pathNoSlash);


    // Read into buffer
    char *contents = util_readEntireFile(pathNoSlash);

    stbuf->st_size = strlen(contents);
    stbuf->st_mode = regular_file.st_mode;
    free(contents);

    return 0;
}

int operations_readdir(const char *path, void *buf, fill_dir_t filler, off_t offset, struct Node *llHead)
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
        filler(buf, current->data, &regular_file, zeroOffset);
        current = current->next;
    }

    return 0;
}
