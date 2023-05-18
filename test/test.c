#include "../src/linkedList.h"
#include "../src/operations.h"
#include "../src/sqlite.h"
#include "../src/util.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

void testDownloadURL()
{
    char url[] = "http://example.com";
    char file[] = "example.html";
    bool success = util_downloadURL(url, file);
    assert(success);

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
    bool success = util_downloadURL(url, file);
    assert(!success);

    // Verify no file was created
    assert(access(file, F_OK) != 0);
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
    Node *llHead = NULL;

    int ret = operations_getattr(full_filename, &st, &llHead);
    assert(ret == 0);

    // verify linked list is correct
    assert(llGetLength(llHead) == 1);
    llContainsString(llHead, filename);

    // update: files are now transient
    const bool fileExists = access(filename, F_OK) == 0;
    assert(!fileExists);

    // verify getattr result
    assert(st.st_size >= 1000 && st.st_size <= 10000);
    // verify read/write/execute permissions
    assert(st.st_mode == (S_IFREG | 0777));

    // verify timestamp
    time_t currentTime = time(NULL);
    assert(st.st_mtime > 0);
    assert(currentTime - st.st_mtime <= 5);
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
    Node *llHead = NULL;
    llInsertNode(&llHead, "file1.txt", "file1.txt");
    llInsertNode(&llHead, "file2.txt", "file2.txt");

    int ret = operations_readdir(filename, buf, testFiller, offset, llHead);

    assert(g_testFillerCallCount == 2);
    assert(ret == 0);
    g_testFillerCallCount = 0;
}

void testReadDirFiles()
{
    // Nothing to do for non-root files
    g_testFillerCallCount = 0;
    char filename[] = "my_file.txt";
    void *buf = NULL;

    int (*filler)(void *buf, const char *name, const struct stat *stbuf, off_t off) = testFiller;

    off_t offset = 0;
    Node *llHead = NULL;
    int ret = operations_readdir(filename, buf, testFiller, offset, llHead);

    // verify success and that buf was unchanged
    assert(g_testFillerCallCount == 0);
    assert(ret == 0);
}

void testRead()
{
    char filename[] = "/www.example.com";
    char *filenameNoSlash = filename + 1;

    // allocate sufficient space
    char *contents = calloc(4096, 1);

    size_t size = 0; // unused
    off_t offset = 0; // unused

    // init linked list with www.example.com
    Node *llHead = NULL;
    llInsertNode(&llHead, filenameNoSlash, filenameNoSlash);

    // read file, return length
    int fileLength = operations_read(filename, contents, size, offset, llHead);
    int strLength = strlen(contents);

    // verify length and file contents
    assert(strstr(contents, "<!doctype html>") != 0);
    assert(strstr(contents, "</html>") != 0);
    free(contents);
    assert(strLength == fileLength);
    assert(fileLength >= 1024 && fileLength <= 2048);
}

void testReadBackslash()
{
    // allocate sufficient space
    char *contents = calloc(4096, 1);

    size_t size = 0; // unused
    off_t offset = 0; // unused

    // init linked list with www.example.com
    Node *llHead = NULL;
    llInsertNode(&llHead, "path", "www.example.com/path");

    // read file, return length
    int fileLength = operations_read("/path", contents, size, offset, llHead);
    int strLength = strlen(contents);

    // verify length and file contents
    assert(strstr(contents, "<!doctype html>") != 0);
    assert(strstr(contents, "</html>") != 0);
    assert(strLength == fileLength);
    assert(fileLength >= 1024 && fileLength <= 2048);

    // cleanup
    free(contents);
}

/*
Unreachable?
void testReadNoFiles()
{
    char filename[] = "/www.example.com";
    char *contents = NULL;

    size_t size = 0; // unused
    off_t offset = 0; // unused

    // init linked list with www.example.com
    Node *llHead = NULL;

    int ret = operations_read(filename, contents, size, offset, llHead);

    // verify result
    //free(contents);
    assert(ret == -ENOENT);
}
*/

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
        char filename[PATH_MAX] = {};
        char url[] = "www.google.com";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, url) == 0);
    }
    {
        char filename[PATH_MAX] = {};
        char url[] = "maps.google.com";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, url) == 0);
    }
    {
        char filename[PATH_MAX] = {};
        char url[] = "example.net";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, url) == 0);
    }
    {
        // slash
        char filename[PATH_MAX] = {};
        char url[] = "google.com/index.html";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, "index.html") == 0);
    }
    {
        // ends with slash (ignore)
        char filename[PATH_MAX] = {};
        char url[] = "google.com/";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, "google.com") == 0);
    }
    {
        char filename[PATH_MAX] = {};
        char url[] = "google.com/maps";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, "maps") == 0);
    }
    {
        // ends with slash (ignore)
        char filename[PATH_MAX] = {};
        char url[] = "google.com/maps/";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, "maps") == 0);
    }
    {
        // double slash
        char filename[PATH_MAX] = {};
        char url[] = "google.com/maps//";
        util_urlToFileName(filename, url);
        assert(strcmp(filename, "maps") == 0);
    }
    {
        // all slashes
        char filename[PATH_MAX] = {};
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

void testCreateDatabase()
{
    sqlite3 *db = createDatabase();
    assert(db);

    int ret = _createWebsiteTable(db);
    assert(ret == SQLITE_OK);

    ret = insertRow(db);
    assert(ret == SQLITE_OK);

    sqlite3_close(db);
    printf("here\n");
}

int main()
{
    testDownloadURL();
    testInvalidURL();
    testReadEntireFile();
    testReadInvalidFile();
    testLinkedList();
    testGetAttrURL();
    testReadDirRoot();
    testReadDirFiles();
    testRead();
    testReadBackslash();
    testIsURL();
    testUrlToFileName();
    testReplaceChar();

    // sqlite tests
    testCreateDatabase();

    printf("Tests passed!\n");
    return 0;
}
