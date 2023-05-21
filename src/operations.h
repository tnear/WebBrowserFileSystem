#ifndef OPERATIONS_MAIN_H
#define OPERATIONS_MAIN_H

#include <stddef.h>
#include <curl/curl.h>
#include <sys/stat.h>

static struct stat regular_file =
{
    .st_mode = S_IFREG | 0444
};

typedef int (*fill_dir_t) (void *buf, const char *name,
				const struct stat *stbuf, off_t off);

struct FuseData;
struct Website;

int operations_getattr(const char *path, struct stat *stbuf, struct FuseData *fuseData);

int operations_readdir(const char *path, void *buf, fill_dir_t filler, off_t offset, struct FuseData *fuseData);

int operations_read(const char *path, char *buf, size_t size,
    off_t offset, struct FuseData *fuseData);

void getUrlFromFusePath(char *url, const char *fusePath, struct FuseData *fuseData);

struct Website* lookupWebsite(struct FuseData *fuseData, const char *url, const char *filename, CURLcode *curlStatus);
struct Website* downloadWebsite(struct FuseData *fuseData, const char *url, const char *filename, CURLcode *curlStatus);

#endif
