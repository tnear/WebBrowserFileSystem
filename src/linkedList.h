#ifndef LINKED_LIST_C
#define LINKED_LIST_C

#include <stdbool.h>

// Linked list of strings
typedef struct Node
{
    char *filename;
    char *url;
    struct Node *next;
} Node;

Node* llCreateNode(const char *filename, const char *url);
Node* llInsertNode(Node **head, const char *filename, const char *url);
void llInsertNodeIfDoesntExist(Node **head, const char *filename, const char *url);
void llFreeList(Node *head);
bool llContainsString(Node *head, const char *filename);
Node* llFindNode(Node *head, const char *filename);
int llGetLength(Node *head);

#endif
