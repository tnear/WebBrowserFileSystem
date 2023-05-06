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
        printf("file exists:\n");
        printf(path);
        printf("\n");
    }

    const bool isValidURL = util_downloadURL(pathNoSlash, pathNoSlash);
    if (!isValidURL)
    {
        // nothing to add
        return 0;
    }

    llInsertNodeIfDoesntExist(llHead, pathNoSlash);

    stbuf->st_mode = regular_file.st_mode;

    // Read into buffer
    char *contents = util_readEntireFile(pathNoSlash);

    stbuf->st_size = strlen(contents);
    free(contents);

    return 0;
}
