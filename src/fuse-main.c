#define FUSE_USE_VERSION 31

#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

// https://www.youtube.com/watch?v=LZCILvr5tUk

typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
                const struct stat *stbuf, off_t off);

char MY_FILE[] = "my_file";
char myText[] = "Hello world3!\n";

static struct stat regular_file = {
    .st_mode = S_IFREG | 0400
};

int myfs_getattr(const char *path, struct stat *stbuf)
{

    if (strcmp(path, "/") == 0)
    {
        // Root path
        stbuf->st_mode = S_IFDIR | 0400;
    }
    else if (strcmp(path, "/my_file") == 0)
    {
        stbuf->st_mode = regular_file.st_mode;
        stbuf->st_size = strlen(myText);
    }


    return 0;
}

int myfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    if (strcmp(path, "/") == 0)
    {
        // Root
        filler(buf, MY_FILE, &regular_file, 0);
    }

    return 0;
}

static int myfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    if (strcmp(path, "/my_file") == 0)
    {
        printf("read for my_file\n");
        strcpy(buf, myText);

        return strlen(myText);
    }

    return -ENOENT;
}

struct fuse_operations myOperations = {
    .getattr = myfs_getattr,
    .read    = myfs_read,
    .readdir = myfs_readdir
};

int main(int argc, char* argv[])
{
    printf("main\n");
    return fuse_main(argc, argv, &myOperations, NULL);
}
