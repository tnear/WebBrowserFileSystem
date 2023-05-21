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

// Returns file attributes
// For "example.com/a",
// fusePath will be: "/example.com/a" when typing commands
// fusePath will be: "/a" when querying (ex: ls)
int operations_getattr(const char *fusePath, struct stat *stbuf, FuseData *fuseData)
{
    // for root path, set permissions then exit
    if (strcmp(fusePath, "/") == 0)
    {
        stbuf->st_mode = S_IFDIR | 0400;
        return 0;
    }

    // get url from fusePath, ex:
    // fusePath: /example.com\\a
    //      url:  example.com/a
    //  filname:              a
    char url[FUSE_PATH_MAX] = {};
    getUrlFromFusePath(url, fusePath, fuseData);
    CURLcode curlStatus = CURLE_OK; // assume okay until network request

    // get file name from url
    char filename[FUSE_PATH_MAX] = {};
    util_urlToFileName(filename, url);

    // lookup by filename first
    // this is needed because sometimes this function will only be called
    // with the filename (ex: "/a" for above example) with no reference to url
    Website *website = lookupWebsiteByFilename(fuseData->db, filename);
    if (!website)
    {
        if (!util_isURL(url))
        {
            // not URL, don't waste time networking
            return 0;
        }

        // next, try lookup by url
        website = lookupWebsiteByUrl(fuseData->db, url);
    }

    // if this url is not in database, it hasn't been seen before, so download it
    if (!website)
    {
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

        // add to database
        website = initWebsite(url, filename, contents);
        insertWebsite(fuseData->db, website);

        // remove temp file
        remove(filename);

        // free memory
        free(contents);
    }

    assert(website);
    // todo: save html length in database?
    size_t fileSize = strlen(website->html);

    stbuf->st_size = fileSize;
    stbuf->st_mode = regular_file.st_mode;
    // set timestamp to current time      
    stbuf->st_mtime = time(NULL);

    // cleanup
    freeWebsite(website);

    return curlStatus;
}

// Returns list of child file names
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

// reads file data
int operations_read(const char *fusePath, char *buf, size_t size,
    off_t offset, FuseData *fuseData)
{
    // fusePath is of form: "http:\\\\example.com"
    // use getURL to convert above to: "http://example.com"
    char url[FUSE_PATH_MAX] = {};
    getUrlFromFusePath(url, fusePath, fuseData);

    Website *website = lookupWebsiteByUrl(fuseData->db, url);
    assert(website);

    // copy to buffer
    strcpy(buf, website->html);
    int len = strlen(website->html);

    // free memory
    freeWebsite(website);

    return len;
}

// ex:
// fusePath: /example.com\\a
//      url:  example.com/a
void getUrlFromFusePath(char *url, const char *fusePath, FuseData *fuseData)
{
    Website *website = lookupWebsiteByUrl(fuseData->db, fusePath + 1);
    if (website)
    {
        // lookup from database if it already exists
        memcpy(url, website->url, strlen(website->url));
        freeWebsite(website);
        return;
    }

    // remove leading slash using +1
    char *pathCopy = strdup(fusePath + 1);

    // '/' is not allowed in file name, so have user type in '\'. Replace it here:
    util_replaceChar(pathCopy, '\\', '/');

    int len = strlen(pathCopy);
    assert(len <= FUSE_PATH_MAX);

    // copy to url which is returned to user
    memcpy(url, pathCopy, len);

    // cleanup
    free(pathCopy);
}
