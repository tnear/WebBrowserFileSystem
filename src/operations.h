#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdbool.h>
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

int operations_getattr(const char *fusePath, struct stat *stbuf, struct FuseData *fuseData);

int operations_readdir(const char *fusePath, void *buf, fill_dir_t filler, off_t offset, struct FuseData *fuseData);

int operations_read(const char *fusePath, char *buf, size_t size,
    off_t offset, struct FuseData *fuseData);

int operations_unlink(const char *fusePath, struct FuseData *fuseData);

void getUrlFromFusePath(char *url, const char *fusePath, struct FuseData *fuseData);

struct Website* lookupWebsite(struct FuseData *fuseData, const char *fusePath, CURLcode *curlStatus);
struct Website* downloadWebsite(struct FuseData *fuseData, const char *url, const char *filename, CURLcode *curlStatus);
bool _checkIfSamePathWithDifferentUrl(struct FuseData *fuseData, struct Website *website, const char *url);

#endif
