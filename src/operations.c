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
#include <sys/mman.h>

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
        assert(!website);
        return curlStatus;
    }

    stbuf->st_size = website->htmlLen;
    stbuf->st_mode = regular_file.st_mode;
    // set timestamp from database
    stbuf->st_mtime = website->time;

    // cleanup
    freeWebsite(website);

    return curlStatus;
}

// Returns list of child file names
int operations_readdir(const char *fusePath, void *buf, fill_dir_t filler,
    off_t offset, FuseData *fuseData)
{
    if (strcmp(fusePath, "/") != 0)
    {
        // ignore non-root files
        return 0;
    }

    // make every file a direct child of root
    assert(strcmp(fusePath, "/") == 0);
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
    // fusePath can be of form: "http:\\\\example.com"
    // use getURL to convert above to: "http://example.com"
    // OR be of path-form, in which case:
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

    struct stat statbuf = {};

    // get data starting point using either sqlite or mmap
    char *stringStartingPoint = _getStringStartingPoint(fuseData, website, &statbuf, offset);
    
    // copy to buffer up to 'size' starting at 'offset'
    strncpy(buf, stringStartingPoint, size);

    // return length written
    int len = strnlen(stringStartingPoint, size);

    // free memory
    freeWebsite(website);
    if (fuseData->useMmap)
    {
        // unmap data
        munmap(stringStartingPoint, statbuf.st_size);
    }

    return len;
}

// used to delete (unlink) websites (files)
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
//  =>  url:  example.com/a
//       s3: /s3:\\\\my-bucket\\index.html
//  =>  url: https://my-bucket.s3.us-east-2.amazonaws.com/index.html
void getUrlFromFusePath(char *url, const char *fusePath, FuseData *fuseData)
{
    // remove leading slash using +1
    const char *fusePathNoSlash = fusePath + 1;
    char pathCopy[FUSE_PATH_MAX] = {};
    strcpy(pathCopy, fusePathNoSlash);

    // '/' is not allowed in file name, so have user type in '\'. Replace it here:
    util_replaceChar(pathCopy, '\\', '/');

    Website *website = lookupWebsiteByUrl(fuseData->db, pathCopy);
    if (website)
    {
        // lookup from database if it already exists
        strcpy(url, website->url);
        freeWebsite(website);
    }
    else
    {
        // copy to url which is returned to user
        strcpy(url, pathCopy);
    }
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

    // check if we've seen this path before with a different url
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
    char urlToUse[FUSE_PATH_MAX] = {};
    if (util_isS3(url))
    {
        // s3 urls require a translation
        convertS3intoURL(urlToUse, url);
    }
    else
    {
        // non-s3 urls do not
        strcpy(urlToUse, url);
    }

    char *contents = NULL;
    int contentLength = getUrlContentLength(urlToUse);
    if (contentLength > BYTE_SIZE_CAP)
    {
        // over limit, return preview data (first 100 bytes)
        contents = _getPreviewData(filename, urlToUse);
    }
    else
    {
        *curlStatus = util_downloadURL(urlToUse, filename);
        if (*curlStatus != CURLE_OK)
        {
            // cannot download, return curl error
            return NULL;
        }

        // read into buffer
        contents = util_readEntireFile(filename);
    }

    assert(contents);
    printf("Downloaded: %s\n", url);

    // get current time
    time_t currentTime = time(NULL);

    // add to database
    Website *website = initWebsite(url, filename, contents, currentTime);
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
    if (website && util_isURL(url) && strcmp(website->url, url) != 0 && strcmp(website->path, url) != 0)
    {
        // found same path, but different url
        // delete this website to prepare to create a new one
        deleteURL(fuseData->db, website->url);
        freeWebsite(website);
        return true;
    }

    return false;
}

char* _getPreviewData(const char *filename, const char *url)
{
    // hit limit
    printf("URL '%s' is over size limit, downloading 100-byte preview instead...\n", url);

    // download first 100 bytes
    char *contents = calloc(BYTE_SIZE_PREVIEW + 1, 1);
    CURLcode curlCode = getFirst100Bytes(contents, url);
    assert(curlCode == CURLE_OK);

    // dump to file to be read by calling function
    FILE *fp = fopen(filename, "w");
    fputs(contents, fp);
    fclose(fp);

    // return data
    // todo: would be better design to use file alone
    return contents;
}

char* _getStringStartingPoint(FuseData *fuseData, Website *website, struct stat *statbuf, off_t offset)
{
    char *stringStartingPoint = NULL;
    if (fuseData->useMmap)
    {
        // use mmap()
        printf("Using mmap...\n");
        // remove temp mmap file if it already exists
        char mmapPath[] = "/tmp/mmap_test.txt";
        remove(mmapPath);

        // write html data to /tmp file
        FILE *fp = fopen(mmapPath, "w");
        fputs(website->html, fp);
        fclose(fp);

        // open file and get attributes
        fp = fopen(mmapPath, "r");
        int fd = fileno(fp);
        int err = fstat(fd, statbuf);
        assert(err == 0);

        // mmap file while honoring offset
        int protection = PROT_READ | PROT_WRITE;
        stringStartingPoint = mmap(NULL, statbuf->st_size, protection, MAP_PRIVATE, fd, offset);
        assert(stringStartingPoint != MAP_FAILED);
        assert(statbuf->st_size - offset > 0);
        assert(strlen(stringStartingPoint) == statbuf->st_size - offset);

        remove(mmapPath);
    }
    else
    {
        // use sqlite
        stringStartingPoint = website->html + offset;
    }
    
    return stringStartingPoint;
}
