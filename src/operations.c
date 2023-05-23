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
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_mtime = time(NULL);
        return 0;
    }

    CURLcode curlStatus = CURLE_OK; // assume okay until network request

    // get website
    Website *website = lookupWebsite(fuseData, fusePath, &curlStatus);
    if (curlStatus != CURLE_OK)
    {
        return curlStatus;
    }

    assert(website);

    // todo: save html length in database?
    stbuf->st_size = strlen(website->html);
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

    // fill all websites added this session
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

// reads website data from database to populate file
int operations_read(const char *fusePath, char *buf, size_t size,
    off_t offset, FuseData *fuseData)
{
    // fusePath is of form: "http:\\\\example.com"
    // use getURL to convert above to: "http://example.com"
    // OR
    // fusePath: "/a"
    // url:       "a"
    char url[FUSE_PATH_MAX] = {};
    getUrlFromFusePath(url, fusePath, fuseData);

    // first, try lookup on url
    Website *website = lookupWebsiteByUrl(fuseData->db, url);
    if (!website)
    {
        // assume that url is a filename, so try looking up on that
        website = lookupWebsiteByFilename(fuseData->db, url);
    }

    // the website must exist in database
    // if this assert fails, it means readattr() did not download url correctly
    assert(website);

    // copy to buffer up to 'size' starting at 'offset'
    char *stringStartingPoint = website->html + offset;
    strncpy(buf, stringStartingPoint, size);

    // return length written
    int len = strnlen(stringStartingPoint, size);

    // free memory
    freeWebsite(website);

    return len;
}

// used to delete websites (files)
int operations_unlink(const char *fusePath, FuseData *fuseData)
{
    // get url from fusePath
    char url[FUSE_PATH_MAX] = {};
    getUrlFromFusePath(url, fusePath, fuseData);

    // check if this website exists
    CURLcode curlStatus = CURLE_OK;
    Website *website = lookupWebsite(fuseData, fusePath, &curlStatus);
    if (website)
    {
        // if so, delete it
        assert(curlStatus == CURLE_OK);
        deleteURL(fuseData->db, website->url);
        freeWebsite(website);
    }

    return 0;
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

Website* lookupWebsite(FuseData *fuseData, const char *fusePath, CURLcode *curlStatus)
{
    // get url from fusePath, ex:
    // fusePath: /example.com\\a
    //      url:  example.com/a
    //  filname:              a
    char url[FUSE_PATH_MAX] = {};
    getUrlFromFusePath(url, fusePath, fuseData);

    // get file name from url
    char filename[FUSE_PATH_MAX] = {};
    util_urlToFileName(filename, url);

    // lookup by filename first
    // this is needed because sometimes this function will only be called
    // with the filename (ex: "/a" for above example) with no reference to url
    Website *website = lookupWebsiteByFilename(fuseData->db, filename);

    bool isSame = _checkIfSamePathWithDifferentUrl(fuseData, website, url);

    if (isSame || !website)
    {
        if (!util_isURL(url))
        {
            // not URL, don't waste time networking
            *curlStatus = CURLE_COULDNT_RESOLVE_HOST;
            return 0;
        }

        // next, try lookup by url
        website = lookupWebsiteByUrl(fuseData->db, url);
    }

    if (!website)
    {
        // url has not been seen before, so download it
        website = downloadWebsite(fuseData, url, filename, curlStatus);
    }

    return website;
}

Website* downloadWebsite(FuseData *fuseData, const char *url, const char *filename, CURLcode *curlStatus)
{
    *curlStatus = util_downloadURL(url, filename);
    if (*curlStatus != CURLE_OK)
    {
        // cannot download, return curl error
        return NULL;
    }

    printf("Downloaded: %s\n", url);

    // read into buffer
    char *contents = util_readEntireFile(filename);

    // add to database
    Website *website = initWebsite(url, filename, contents);
    insertWebsite(fuseData->db, website);

    // remove temp file
    remove(filename);

    // free memory
    free(contents);

    return website;
}

bool _checkIfSamePathWithDifferentUrl(FuseData *fuseData, Website *website, const char *url)
{
    // the same path can exist for different urls:
    // ex: example.com/path and example.net/path
    // these both map to files called "path"
    // this function detects this case and deletes the older one
    if (website && util_isURL(url) && strcmp(website->url, url) != 0)
    {
        // found same path, but different url
        // delete this website to prepare to create a new one
        deleteURL(fuseData->db, website->url);
        freeWebsite(website);
        return true;
    }

    return false;
}
