#include "../src/linkedList.h"
#include "../src/operations.h"
#include "../src/util.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    llInsertNode(&head, "Hello");
    llInsertNode(&head, "World");
    llInsertNode(&head, "Linked");
    llInsertNode(&head, "List");

    assert(llContainsString(head, "Hello"));
    assert(!llContainsString(head, "fake_string"));

    assert(llGetLength(head) == 4);

    // try duplicate insert
    llInsertNodeIfDoesntExist(&head, "Hello");
    assert(llGetLength(head) == 4);

    assert(strcmp(head->data, "Hello") == 0);
    head = head->next;
    assert(strcmp(head->data, "World") == 0);
    head = head->next;
    assert(strcmp(head->data, "Linked") == 0);
    head = head->next;
    assert(strcmp(head->data, "List") == 0);
    head = head->next;
    assert(!head);

    llFreeList(head);
}

void testGetAttrFileExists()
{
    // create file
    char full_filename[] = "/filePath.txt";
    char *filename = full_filename + 1; // ignore leading slash
    FILE *fp = fopen(filename, "w");

    // write data
    char data[] = "test data...";
    fputs(data, fp);
    fclose(fp);

    struct stat st;
    memset(&st, 0, sizeof(st));

    Node *llHead = NULL;

    int ret = operations_getattr(full_filename, &st, &llHead);
    const bool fileExists = access(filename, F_OK) == 0;
    assert(fileExists);

    remove(filename);

    // verify successful call
    assert(ret == 0);

    // verify file attributes
    assert(st.st_size == 12);
    assert(st.st_mode == (S_IFREG | 0400));
}

void testGetAttrURL()
{
    char *full_filename = "/www.example.com";
    char *filename = full_filename + 1; // ignore leading slash

    struct stat st;
    memset(&st, 0, sizeof(st));

    Node *llHead = NULL;
    int ret = operations_getattr(full_filename, &st, &llHead);

    // verify file was created
    const bool fileExists = access(filename, F_OK) == 0;
    assert(fileExists);

    // verify file contents
    char *contents = util_readEntireFile(filename);
    assert(strstr(contents, "<!doctype html>") != 0);
    assert(strstr(contents, "</html>") != 0);
    free(contents);
    remove(filename);
    
    // verify getattr result
    assert(ret == 0);
    assert(st.st_size >= 1000 && st.st_size <= 10000);
    assert(st.st_mode == (S_IFREG | 0400));
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
    llInsertNode(&llHead, "file1.txt");
    llInsertNode(&llHead, "file2.txt");

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
    char *contents = malloc(2048);

    size_t size = 0; // unused
    off_t offset = 0; // unused

    // init linked list with www.example.com
    Node *llHead = NULL;
    llInsertNode(&llHead, filename + 1); // +1 to trim slash

    // read file, return length
    int fileLength = operations_read(filename, contents, size, offset, llHead);
    int strLength = strlen(contents);

    remove(filename);

    // verify length and file contents
    assert(strstr(contents, "<!doctype html>") != 0);
    assert(strstr(contents, "</html>") != 0);
    free(contents);
    assert(strLength == fileLength);
    assert(fileLength >= 1024 && fileLength <= 2048);
}

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

int main()
{
    testDownloadURL();
    testInvalidURL();
    testReadEntireFile();
    testReadInvalidFile();
    testLinkedList();
    testGetAttrFileExists();
    testGetAttrURL();
    testReadDirRoot();
    testReadDirFiles();
    testRead();
    testReadNoFiles();

    printf("Tests passed!\n");
    return 0;
}
