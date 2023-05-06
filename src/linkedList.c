#include "linkedList.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node *llCreateNode(const char *data)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    assert(newNode);
    // Duplicate the string
    newNode->data = strdup(data);
    newNode->next = NULL;
    return newNode;
}

// Insert a new node at the end of the list
void llInsertNode(Node **head, const char *data)
{
    Node *newNode = llCreateNode(data);
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

void llInsertNodeIfDoesntExist(Node **head, const char *data)
{
    if (llContainsString(*head, data))
    {
        // already added
        return;
    }

    llInsertNode(head, data);
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
void llFreeList(Node *head)
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

bool llContainsString(Node *head, const char *data)
{
    Node *current = head;
    while (current)
    {
        if (strcmp(current->data, data) == 0)
        {
            return true;
        }

        current = current->next;
    }

    return false;
}

int llGetLength(Node *head)
{
    int length = 0;
    Node *current = head;
    while (current)
    {
        ++length;
        current = current->next;
    }

    return length;
}
