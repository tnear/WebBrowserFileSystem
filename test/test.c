#include "../src/fuseData.h"
#include "../src/linkedList.h"
#include "../src/operations.h"
#include "../src/sqlite.h"
#include "../src/util.h"
#include "../src/website.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <curl/curl.h>
#include <sys/stat.h>

#define FUSE_PATH_MAX 4096

bool isFile(const char *filename)
{
    return access(filename, F_OK) == 0;
}

void testDownloadURL()
{
    char url[] = "http://example.com";
    char file[] = "example.html";
    CURLcode curlStatus = util_downloadURL(url, file);
    assert(curlStatus == CURLE_OK);

    char *contents = util_readEntireFile(file);

    // Verify beginning and end of string
    assert(strstr(contents, "<!doctype html>") != 0);
    assert(strstr(contents, "</html>") != 0);

    // Cleanup
    free(contents);
    remove(file);
}

void testInvalidURL()
{
    char url[] = "http://fake_url_site1234.com";
    char file[] = "fake.html";

    // Verify failure
    CURLcode curlStatus = util_downloadURL(url, file);
    assert(curlStatus == CURLE_COULDNT_RESOLVE_HOST);

    // Verify no file was created (isFile)
    assert(!isFile(file));
}

void testReadEntireFile()
{
    // Create file
    char filename[] = "myFile.txt";
    FILE *file = fopen(filename, "w");
    char data[] = "Test data 1234...";
    fputs(data, file);
    fclose(file);

    // Read file
    char *contents = util_readEntireFile(filename);
    // Verify file contents
    assert(strcmp(contents, data) == 0);

    // Cleanup
    free(contents);
    remove(filename);
}

void testReadInvalidFile()
{
    // Invalid files should return NULL
    char filename[] = "fake_file_name.txt";
    char *contents = util_readEntireFile(filename);
    assert(!contents);
}

void testLinkedList()
{
    Node *head = NULL;

    Node *helloNode = llInsertNode(&head, "Hello",  "Hello.html");
    assert(strcmp(helloNode->filename, "Hello") == 0);
    assert(strcmp(helloNode->url, "Hello.html") == 0);
    llInsertNode(&head, "World",  "World.html");
    llInsertNode(&head, "Linked", "Linked.html");
    llInsertNode(&head, "List",   "List.html");

    assert(llContainsString(head, "Hello"));
    assert(!llContainsString(head, "fake_string"));

    assert(llFindNode(head, "Hello") == helloNode);
    assert(llGetLength(head) == 4);

    // try duplicate insert
    llInsertNodeIfDoesntExist(&head, "Hello", "Hello.html");
    assert(llGetLength(head) == 4);

    assert(strcmp(head->filename, "Hello") == 0);
    head = head->next;
    assert(strcmp(head->filename, "World") == 0);
    head = head->next;
    assert(strcmp(head->filename, "Linked") == 0);
    head = head->next;
    assert(strcmp(head->filename, "List") == 0);
    head = head->next;
    assert(!head);

    llFreeList(head);
}

void testGetAttrURL()
{
    char *full_filename = "/www.example.com";
    char *filename = full_filename + 1; // ignore leading slash

    struct stat st;
    memset(&st, 0, sizeof(st));
    FuseData *fuseData = initFuseData();
    assert(getWebsiteCount(fuseData->db) == 0);

    int ret = operations_getattr(full_filename, &st, fuseData);
    assert(ret == CURLE_OK);

    // verify linked list is correct
    assert(getWebsiteCount(fuseData->db) == 1);

    // update: files are now transient
    assert(!isFile(filename));

    // verify getattr result
    assert(st.st_size >= 1000 && st.st_size <= 10000);
    // verify read/write/execute permissions
    assert(st.st_mode == (S_IFREG | 0777));

    // verify timestamp
    time_t currentTime = time(NULL);
    assert(st.st_mtime > 0);
    assert(currentTime - st.st_mtime <= 5);
}

void testGetAttrFakeURL()
{
    char *full_filename = "/www.fake_url_example_123_xyz.net";

    struct stat st;
    memset(&st, 0, sizeof(st));
    FuseData *fuseData = initFuseData();

    int ret = operations_getattr(full_filename, &st, fuseData);
    assert(ret == CURLE_COULDNT_RESOLVE_HOST);
}

int g_testFillerCallCount = 0;

int testFiller(void *buf, const char *name, const struct stat *stbuf, off_t off)
{
    ++g_testFillerCallCount;
}

void testReadDirRoot()
{
    g_testFillerCallCount = 0;
    char filename[] = "/";
    void *buf = NULL;

    int (*filler)(void *buf, const char *name, const struct stat *stbuf, off_t off) = testFiller;

    off_t offset = 0;
    FuseData *fuseData = initFuseData();

    char url1[] = "file1.txt";
    char url2[] = "file2.txt";

    // website 1
    Website *website = initWebsite(url1, url1, url1);
    insertWebsite(fuseData->db, website);
    freeWebsite(website);

    // website 2
    website = initWebsite(url2, url2, url2);
    insertWebsite(fuseData->db, website);

    int ret = operations_readdir(filename, buf, testFiller, offset, fuseData);
    assert(ret == 0);

    assert(g_testFillerCallCount == 2);
    g_testFillerCallCount = 0;

    // cleanup
    freeWebsite(website);
}

void testReadDirFiles()
{
    // Nothing to do for non-root files
    g_testFillerCallCount = 0;
    char filename[] = "my_file.txt";
    void *buf = NULL;

    int (*filler)(void *buf, const char *name, const struct stat *stbuf, off_t off) = testFiller;

    off_t offset = 0;
    FuseData *fuseData = initFuseData();
    int ret = operations_readdir(filename, buf, testFiller, offset, fuseData);

    // verify success and that buf was unchanged
    assert(g_testFillerCallCount == 0);
    assert(ret == 0);

    deleteFuseData(fuseData);
}

void testRead()
{
    char filename[] = "/www.example.com";
    char *filenameNoSlash = filename + 1;

    // allocate sufficient space
    char *contents = calloc(4096, 1);

    // pre-download url b/c we are directly calling read()
    CURLcode curlStatus = util_downloadURL(filenameNoSlash, filenameNoSlash);
    assert(curlStatus == CURLE_OK);
    char *htmlData = util_readEntireFile(filenameNoSlash);

    size_t size = 0; // unused
    off_t offset = 0; // unused

    // init linked list with www.example.com
    FuseData *fuseData = initFuseData();
    Website *website = initWebsite(filenameNoSlash, filenameNoSlash, htmlData);
    insertWebsite(fuseData->db, website);

    // read file, return length
    int fileLength = operations_read(filename, contents, size, offset, fuseData);
    int strLength = strlen(contents);

    // verify length and file contents
    assert(strstr(contents, "<!doctype html>") != 0);
    assert(strstr(contents, "</html>") != 0);
    assert(strLength == fileLength);
    assert(fileLength >= 1024 && fileLength <= 2048);

    // cleanup
    deleteFuseData(fuseData);
    free(htmlData);
    free(contents);
    remove(filenameNoSlash);
    assert(!isFile(filenameNoSlash));
    freeWebsite(website);
}

void testReadBackslash()
{
    // allocate sufficient space
    char *contents = calloc(4096, 1);

    size_t size = 0; // unused
    off_t offset = 0; // unused

    // init linked list with www.example.com
    FuseData *fuseData = initFuseData();
    char url[] = "/www.example.com/path";
    char *urlNoSlash = url + 1;

    // pre-download url b/c we are directly calling read()
    char filename[] = "path";
    assert(!isFile(filename));

    CURLcode curlStatus = util_downloadURL(urlNoSlash, filename);
    assert(curlStatus == CURLE_OK);
    char *htmlData = util_readEntireFile(filename);

    // add to database
    Website *website = initWebsite(urlNoSlash, filename, htmlData);
    insertWebsite(fuseData->db, website);

    // read file, return length
    int fileLength = operations_read(url, contents, size, offset, fuseData);
    int strLength = strlen(contents);

    // verify length and file contents
    assert(strstr(contents, "<!doctype html>") != 0);
    assert(strstr(contents, "</html>") != 0);
    assert(strLength == fileLength);
    assert(fileLength >= 1024 && fileLength <= 2048);

    // cleanup
    free(contents);
    free(htmlData);
    deleteFuseData(fuseData);
    assert(isFile(filename));
    remove(filename);
    assert(!isFile(filename));
    freeWebsite(website);
}

void testIsURL()
{
    // URLs
    assert(util_isURL("google.com"));
    
    // Not URLs
    assert(!util_isURL(".com"));
    assert(!util_isURL("com"));
}

void testUrlToFileName()
{
    {
        char filename[FUSE_PATH_MAX] = {};
        char url[] = "www.google.com";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, url) == 0);
    }
    {
        char filename[FUSE_PATH_MAX] = {};
        char url[] = "maps.google.com";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, url) == 0);
    }
    {
        char filename[FUSE_PATH_MAX] = {};
        char url[] = "example.net";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, url) == 0);
    }
    {
        // slash
        char filename[FUSE_PATH_MAX] = {};
        char url[] = "google.com/index.html";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, "index.html") == 0);
    }
    {
        // ends with slash (ignore)
        char filename[FUSE_PATH_MAX] = {};
        char url[] = "google.com/";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, "google.com") == 0);
    }
    {
        char filename[FUSE_PATH_MAX] = {};
        char url[] = "google.com/maps";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, "maps") == 0);
    }
    {
        // ends with slash (ignore)
        char filename[FUSE_PATH_MAX] = {};
        char url[] = "google.com/maps/";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, "maps") == 0);
    }
    {
        // double slash
        char filename[FUSE_PATH_MAX] = {};
        char url[] = "google.com/maps//";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, "maps") == 0);
    }
    {
        // all slashes
        char filename[FUSE_PATH_MAX] = {};
        char url[] = "///";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, "") == 0);
    }
}

void testReplaceChar()
{
    {
        // Replace '/' with '\'
        char str[] = "a/b";
        util_replaceChar(str, '/', '\\');
        assert(strcmp(str, "a\\b") == 0);
    }

    {
        // Replace '/' with '\'
        char str[] = "//";
        util_replaceChar(str, '/', '\\');
        assert(strcmp(str, "\\\\") == 0);
    }
}

void testWebsite()
{
    char url[] = "www.example.com";
    char path[] = "example";
    char html[] = "<HTML>";

    Website *website = initWebsite(url, path, html);

    assert(strcmp(website->url, url) == 0);
    assert(strcmp(website->path, path) == 0);
    assert(strcmp(website->html, html) == 0);

    freeWebsite(website);
}

void testCreateDatabase()
{
    sqlite3 *db = createDatabase();

    const char url[] = "www.example.com";
    const char path[] = "example";
    const char html[] = "<HTML></HTML>";

    Website *website = initWebsite(url, path, html);

    int ret = insertWebsite(db, website);
    assert(ret == SQLITE_OK);

    // lookup existing website and verify contents
    Website *wLookup = lookupWebsiteByUrl(db, url);
    assert(strcmp(wLookup->url, url) == 0);
    assert(strcmp(wLookup->path, path) == 0);
    assert(strcmp(wLookup->html, html) == 0);

    // fake url (returns NULL)
    Website *fake = lookupWebsiteByUrl(db, "fake_url");
    assert(!fake);

    // cleanup
    sqlite3_close(db);
    freeWebsite(website);
    freeWebsite(wLookup);
}

void testLookupURL()
{
    sqlite3 *db = createDatabase();

    const char url[] = "www.example.com";
    const char path[] = "example";
    const char html[] = "<HTML></HTML>";

    // lookup before inserting
    assert(!lookupURL(db, url));

    Website *website = initWebsite(url, path, html);
    insertWebsite(db, website);

    // lookup after inserting
    assert(lookupURL(db, url));

    // lookup fake url
    assert(!lookupURL(db, "fake_url"));

    // cleanup
    sqlite3_close(db);
    freeWebsite(website);
}

void testGetFileNames()
{
    sqlite3 *db = createDatabase();

    // empty state
    Node *node = getFileNames(db);
    assert(!node);
    const char path1[] = "example";
    const char path2[] = "example2";

    // add two websites
    {
        const char url[] = "www.example.com";
        const char path[] = "example";
        const char html[] = "<HTML></HTML>";

        Website *website = initWebsite(url, path1, html);
        insertWebsite(db, website);
        freeWebsite(website);
    }

    {
        const char url[] = "www.example.com2";
        const char html[] = "<HTML></HTML>2";

        Website *website = initWebsite(url, path2, html);
        insertWebsite(db, website);
        freeWebsite(website);
    }

    node = getFileNames(db);
    assert(llGetLength(node) == 2);
    assert(llContainsString(node, path1));
    assert(llContainsString(node, path2));
}

void testFuseData()
{
    FuseData *fuseData = initFuseData();
    assert(fuseData->db);

    // cleanup
    deleteFuseData(fuseData);
}

void testLookupByFilename()
{
    sqlite3 *db = createDatabase();

    const char url[] = "www.example.com";
    const char path[] = "example";
    const char html[] = "<HTML></HTML>";

    // lookup empty database
    Website *fake = lookupWebsiteByFilename(db, "fake_filename");
    assert(!fake);

    Website *website = initWebsite(url, path, html);
    insertWebsite(db, website);

    // lookup existing website and verify contents
    Website *wLookup = lookupWebsiteByFilename(db, path);
    assert(strcmp(wLookup->url, url) == 0);
    assert(strcmp(wLookup->path, path) == 0);
    assert(strcmp(wLookup->html, html) == 0);

    // fake url (returns NULL)
    fake = lookupWebsiteByFilename(db, "fake_filename");
    assert(!fake);

    // cleanup
    sqlite3_close(db);
    freeWebsite(website);
    freeWebsite(wLookup);
}

void testGetAttrOnPath()
{
    FuseData *fuseData = initFuseData();

    // insert website which has a path ("my_path")
    char *fusePath = "/my_path";
    char *fusePathNoSlash = fusePath + 1;
    char html[] = "data here...";
    Website *website = initWebsite("example.com/my_path", fusePathNoSlash, html);
    insertWebsite(fuseData->db, website);

    struct stat st;
    memset(&st, 0, sizeof(st));

    // verify successful lookup of path (NOT url)
    int ret = operations_getattr(fusePath, &st, fuseData);
    assert(ret == CURLE_OK);

    // verify getattr result
    assert(st.st_size == strlen(html));

    // verify timestamp
    time_t currentTime = time(NULL);
    assert(currentTime - st.st_mtime <= 5);

    // cleanup
    freeWebsite(website);
}

void testGetReadOnPath()
{
    char filename[] = "/www.example.com/path";
    char *filenameNoSlash = filename + 1;
    char path[] = "/path";
    char *pathNoSlash = path + 1;

    // allocate sufficient space
    char *buffer = calloc(4096, 1);

    char htmlData[] = "my data...";

    size_t size = 0; // unused
    off_t offset = 0; // unused

    // add website to database
    FuseData *fuseData = initFuseData();
    Website *website = initWebsite(filenameNoSlash, pathNoSlash, htmlData);
    insertWebsite(fuseData->db, website);

    // read file, verify data
    int fileLength = operations_read(path, buffer, size, offset, fuseData);
    assert(strcmp(buffer, htmlData) == 0);
    assert(strlen(buffer) == strlen(htmlData));

    // cleanup
    deleteFuseData(fuseData);
    free(buffer);
    freeWebsite(website);
}

int main()
{
    testDownloadURL();
    testInvalidURL();
    testReadEntireFile();
    testReadInvalidFile();
    testLinkedList();
    testGetAttrURL();
    testGetAttrFakeURL();
    testReadDirRoot();
    testReadDirFiles();
    testRead();
    testReadBackslash();
    testIsURL();
    testUrlToFileName();
    testReplaceChar();
    testFuseData();
    testWebsite();
    testCreateDatabase();
    testLookupURL();
    testGetFileNames();
    testLookupByFilename();
    testGetAttrOnPath();
    testGetReadOnPath();

    printf("Tests passed!\n");
    return 0;
}
