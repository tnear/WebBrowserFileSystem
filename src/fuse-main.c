#define FUSE_USE_VERSION 31

#include "operations.h"
#include "linkedList.h"
#include "util.h"

#include <fuse.h>
#include <limits.h>
#include <stdio.h>

// https://www.youtube.com/watch?v=LZCILvr5tUk

// global variable for start of linked list
// Todo: make it non-global
Node *llHead = NULL;
char g_mountDir[PATH_MAX] = {};

// Get file attributes. Similar to stat().
int urlfs_getattr(const char *path, struct stat *stbuf)
{
    return operations_getattr(path, g_mountDir, stbuf, &llHead);
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
    return operations_read(path, g_mountDir, buf, size, offset, llHead);
}

struct fuse_operations urlfsOperations = {
    .getattr = urlfs_getattr,
    .read    = urlfs_read,
    .readdir = urlfs_readdir
};

int main(int argc, char* argv[])
{
    util_getMountPoint(g_mountDir, sizeof(g_mountDir), argc, argv);
    printf("Mount dir: %s\n", g_mountDir);

    void *userData = NULL;
    return fuse_main(argc, argv, &urlfsOperations, userData);
}
