#ifndef OPERATIONS_MAIN_H
#define OPERATIONS_MAIN_H

#include <stddef.h>
#include <sys/stat.h>

static struct stat regular_file =
{
    .st_mode = S_IFREG | 0777
};

typedef int (*fill_dir_t) (void *buf, const char *name,
				const struct stat *stbuf, off_t off);

struct Node;
int operations_getattr(const char *path, const char *mountDir, struct stat *stbuf, struct Node **llHead);

int operations_readdir(const char *path, void *buf, fill_dir_t filler, off_t offset, struct Node *llHead);

int operations_read(const char *path, const char *mountDir, char *buf, size_t size,
    off_t offset, struct Node *llHead);

#endif
