#include "../src/util.h"
#include "../src/linkedList.h"

#include <assert.h>
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

void testGetAttr()
{
    char path[] = "filePath.txt";
    struct stat st;
    memset(&st, 0, sizeof(st));
    //int ret = urlfs_getattr(path, st);

    printf("here\n");

}

int main()
{
    testDownloadURL();
    testInvalidURL();
    testReadEntireFile();
    testReadInvalidFile();
    testLinkedList();
    testGetAttr();

    printf("Tests passed!\n");
    return 0;
}
