#include "linkedList.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node *createNode(const char *data)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    assert(newNode);
    // Duplicate the string
    newNode->data = strdup(data);
    newNode->next = NULL;
    return newNode;
}

// Insert a new node at the end of the list
void insertNode(Node **head, const char *data)
{
    Node *newNode = createNode(data);
    if (*head == NULL)
    {
        *head = newNode;
    }
    else
    {
        Node *current = *head;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = newNode;
    }
}

/*
void displayList(const Node *head)
{
    const Node *current = head;
    while (current != NULL)
    {
        printf("%s ", current->data);
        current = current->next;
    }
    printf("\n");
}
*/

// Free the memory allocated for the linked list
void freeList(Node *head)
{
    Node *current = head;
    while (current)
    {
        Node *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
}
