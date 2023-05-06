#ifndef LINKED_LIST_C
#define LINKED_LIST_C

// Linked list of strings
typedef struct Node
{
    char *data;
    struct Node *next;
} Node;

Node *createNode(const char *data);
void insertNode(Node **head, const char *data);
void freeList(Node *head);

#endif
