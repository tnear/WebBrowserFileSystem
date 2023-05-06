#define FUSE_USE_VERSION 31

#include "linkedList.h"
#include "util.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

// https://www.youtube.com/watch?v=LZCILvr5tUk

typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
                const struct stat *stbuf, off_t off);

static char HTML_FILE[] = "example.html";
static char HTML_FILE_PATH[] = "/example.html";

static struct stat regular_file = {
    .st_mode = S_IFREG | 0400
};


// Get file attributes. Similar to stat().
int myfs_getattr(const char *path, struct stat *stbuf)
{
    printf("getattr\n");
    printf(path);
    printf("\n");
    if (strcmp(path, "/") == 0)
    {
        // Root path
        stbuf->st_mode = S_IFDIR | 0400;
        return 0;
    }

    const char *startOfFilePath = path + 1; // Account for leading "/", ex: "/file.html"
    const bool fileExists = access(startOfFilePath, F_OK) == 0;
    if (fileExists)
    {
        printf("file exists:\n");
        printf(path);
        printf("\n");
    }
    
    if (strcmp(path, HTML_FILE_PATH) == 0)
    {
        stbuf->st_mode = regular_file.st_mode;
        // Download URL
        util_downloadURL("http://example.com", HTML_FILE);

        // Read into buffer
        char *contents = util_readEntireFile(HTML_FILE);

        stbuf->st_size = strlen(contents);
        free(contents);
    }

    return 0;
}

// Read directory
int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    if (strcmp(path, "/") == 0)
    {
        // Root
        off_t offset = 0;
        filler(buf, HTML_FILE, &regular_file, offset);
    }

    return 0;
}

// Read data from an open file
static int myfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    printf("read\n");
    printf(path);
    printf("\n");
    if (strcmp(path, HTML_FILE_PATH) == 0)
    {
        // Download URL
        util_downloadURL("http://example.com", HTML_FILE);

        // Read into buffer
        char *contents = util_readEntireFile(HTML_FILE);

        // Copy to buffer
        strcpy(buf, contents);
        size_t len = strlen(contents);
        free(contents);
        return len;
    }

    return -ENOENT;
}

struct fuse_operations urlfsOperations = {
    .getattr = myfs_getattr,
    .read    = myfs_read,
    .readdir = myfs_readdir
};

int main(int argc, char* argv[])
{
    printf("main4\n");
    void *userData = NULL;
    return fuse_main(argc, argv, &urlfsOperations, userData);
}
