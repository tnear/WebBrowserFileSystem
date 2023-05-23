#define FUSE_USE_VERSION 31

#include "fuseData.h"
#include "operations.h"

#include <fuse.h>

// global variable for fuse data
FuseData *g_fuseData = NULL;

// Get file attributes. Similar to stat().
int urlfs_getattr(const char *path, struct stat *stbuf)
{
    operations_getattr(path, stbuf, g_fuseData);
    return 0; // assume everything is successful to avoid fuse errors
}

// Read directory
int urlfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    return operations_readdir(path, buf, filler, offset, g_fuseData);
}

// Read data from an open file
int urlfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    return operations_read(path, buf, size, offset, g_fuseData);
}

struct fuse_operations urlfsOperations =
{
    .getattr = urlfs_getattr,
    .read    = urlfs_read,
    .readdir = urlfs_readdir
};

int main(int argc, char* argv[])
{
    g_fuseData = initFuseData();
    int ret = fuse_main(argc, argv, &urlfsOperations, NULL);

    // cleanup
    printf("\n\nfuse-main exiting...\n");
    deleteFuseData(g_fuseData);

    return ret;
}
