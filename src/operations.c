#include "operations.h"
#include "fuseData.h"
#include "linkedList.h"
#include "sqlite.h"
#include "util.h"
#include "website.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <curl/curl.h>

#define FUSE_PATH_MAX 4096

// For example.com/a
// path will be: "/example.com/a" when typing commands
// path will be: "/a" when running querying commands (ex: ls)
int operations_getattr(const char *path, struct stat *stbuf, FuseData *fuseData)
{
    if (strcmp(path, "/") == 0)
    {
        // for root path, set permissions then exit
        stbuf->st_mode = S_IFDIR | 0400;
        return 0;
    }

    printf("path: %s\n", path);

    char url[FUSE_PATH_MAX] = {};
    getURL(url, path, fuseData);

    if (!util_isURL(url))
    {
        // not URL, don't waste time networking
        return 0;
    }

    CURLcode curlStatus = CURLE_OK;
    size_t fileSize = -1;
    Website *website = lookupWebsiteByUrl(fuseData->db, url);
    if (website)
    {
        fileSize = strlen(website->html);
    }
    else
    {
        char filename[FUSE_PATH_MAX] = {};
        util_urlToFileName(filename, url);

        curlStatus = util_downloadURL(url, filename);
        if (curlStatus != CURLE_OK)
        {
            // cannot download, return error
            return curlStatus;
        }

        printf(url);
        printf("\n");

        // read into buffer
        char *contents = util_readEntireFile(filename);
        fileSize = strlen(contents);

        // add to database
        website = initWebsite(url, filename, contents);
        insertWebsite(fuseData->db, website);

        // remove temp file
        remove(filename);

        // free memory
        free(contents);
    }

    // todo: save html length in database?
    assert(fileSize != -1);
    stbuf->st_size = fileSize;
    stbuf->st_mode = regular_file.st_mode;
    // set timestamp to current time      
    stbuf->st_mtime = time(NULL);

    // cleanup
    assert(website);
    freeWebsite(website);

    return curlStatus;
}

int operations_readdir(const char *path, void *buf, fill_dir_t filler,
    off_t offset, FuseData *fuseData)
{
    if (strcmp(path, "/") != 0)
    {
        // ignore non-root files
        return 0;
    }

    // make every file a direct child of root
    assert(strcmp(path, "/") == 0);
    const off_t zeroOffset = 0;

    // fill all files added this session
    Node *current = getFileNames(fuseData->db);
    Node *currentCache = current;
    while (current)
    {
        filler(buf, current->filename, &regular_file, zeroOffset);
        current = current->next;
    }

    // free linked list when done
    llFreeList(currentCache);

    return 0;
}

int operations_read(const char *fusePath, char *buf, size_t size,
    off_t offset, FuseData *fuseData)
{
    // fusePath is of form: "http:\\\\example.com"
    // use getURL to convert above to: "http://example.com"
    char url[FUSE_PATH_MAX] = {};
    getURL(url, fusePath, fuseData);

    Website *website = lookupWebsiteByUrl(fuseData->db, url);
    assert(website);

    // copy to buffer
    strcpy(buf, website->html);
    int len = strlen(website->html);

    // free memory
    freeWebsite(website);

    return len;
}

void getURL(char *url, const char *fusePath, FuseData *fuseData)
{
    char *pathNoSlash = NULL;
    Website *website = NULL;
    website = lookupWebsiteByUrl(fuseData->db, fusePath + 1);
    if (website)
    {
        pathNoSlash = strdup(website->url);
    }
    else
    {
        // ex: path: "/example.com"
        // ex: pathNoSlash: "example.com"
        char *pathCopy = strdup(fusePath);
        // '/' is not allowed in file name, so have user type in '\'. Replace it here:
        util_replaceChar(pathCopy, '\\', '/');
        pathNoSlash = strdup(pathCopy + 1);
        free(pathCopy);
    }

    int len = strlen(pathNoSlash);
    assert(len <= FUSE_PATH_MAX);

    // copy to url which is returned to user
    memcpy(url, pathNoSlash, len);

    // cleanup
    free(pathNoSlash);
    freeWebsite(website);
}
