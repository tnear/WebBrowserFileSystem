#ifndef LINKED_LIST_C
#define LINKED_LIST_C

#include <stdbool.h>

// Linked list of strings
typedef struct Node
{
    char *data;
    struct Node *next;
} Node;

Node *llCreateNode(const char *data);
void llInsertNode(Node **head, const char *data);
void llInsertNodeIfDoesntExist(Node **head, const char *data);
void llFreeList(Node *head);
bool llContainsString(Node *head, const char *data);
int llGetLength(Node *head);

#endif
