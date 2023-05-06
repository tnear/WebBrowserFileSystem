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
