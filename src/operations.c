#include "operations.h"
#include "linkedList.h"
#include "util.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int operations_getattr(const char *path, const char *mountDir, struct stat *stbuf, Node **llHead)
{
    if (strcmp(path, "/") == 0)
    {
        // Root path
        stbuf->st_mode = S_IFDIR | 0777;
        return 0;
    }
    
    // Account for leading "/", ex: "/example.com" => "example.com"
    const char *pathNoSlash = path + 1;

    // mountDir ex: "/home/.../mnt/"

    char absPath[PATH_MAX] = {};
    strcat(absPath, mountDir);
    strcat(absPath, pathNoSlash);

    const bool fileExists = access(absPath, F_OK) == 0;
    if (fileExists)
    {
        // get file metadata
        struct stat localBuf;
	    int ret = stat(absPath, &localBuf);

        // deep copy to stbuf argument
        memcpy(stbuf, &localBuf, sizeof(struct stat));

        stbuf->st_mode = regular_file.st_mode;
        return ret;
    }

    const bool isValidURL = util_downloadURL(pathNoSlash, absPath);
    if (!isValidURL)
    {
        // nothing to add
        return 0;
    }

    printf("Inserting\n");
    llInsertNodeIfDoesntExist(llHead, absPath);

    // Read into buffer
    char *contents = util_readEntireFile(absPath);

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

int operations_read(const char *path, char *buf, size_t size, off_t offset, struct Node *llHead)
{
    const char *pathNoSlash = path + 1;
    if (!llContainsString(llHead, pathNoSlash))
    {
        // cannot find this file
        return -ENOENT;
    }

    util_downloadURL(pathNoSlash, pathNoSlash);
    
    // Read file into buffer
    char *contents = util_readEntireFile(pathNoSlash);

    // Copy to buffer
    strcpy(buf, contents);
    size_t len = strlen(contents);
    free(contents);
    return len;
}
