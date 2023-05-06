#define FUSE_USE_VERSION 31

#include "operations.h"
#include "linkedList.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// https://www.youtube.com/watch?v=LZCILvr5tUk

// global variable for start of linked list
// Todo: make it non-global
Node *llHead = NULL;

// Get file attributes. Similar to stat().
int urlfs_getattr(const char *path, struct stat *stbuf)
{
    return operations_getattr(path, stbuf, &llHead);
}

// Read directory
int urlfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    return operations_readdir(path, buf, filler, offset, llHead);
}

// Read data from an open file
int urlfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
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

struct fuse_operations urlfsOperations = {
    .getattr = urlfs_getattr,
    .read    = urlfs_read,
    .readdir = urlfs_readdir
};

int main(int argc, char* argv[])
{
    printf("main4\n");
    void *userData = NULL;
    return fuse_main(argc, argv, &urlfsOperations, userData);
}
