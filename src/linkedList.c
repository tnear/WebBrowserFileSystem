#include "linkedList.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node *llCreateNode(const char *filename, const char *url)
{
    Node *newNode = malloc(sizeof(Node));
    assert(newNode);
    // Duplicate the string
    newNode->filename = strdup(filename);
    newNode->url = strdup(url);
    newNode->next = NULL;
    return newNode;
}

// Insert a new node at the end of the list
Node* llInsertNode(Node **head, const char *filename, const char *url)
{
    Node *newNode = llCreateNode(filename, url);
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

    return newNode;
}

void llInsertNodeIfDoesntExist(Node **head, const char *filename, const char *url)
{
    if (llContainsString(*head, filename))
    {
        // already added
        return;
    }

    llInsertNode(head, filename, url);
}

// Free the memory allocated for the linked list
void llFreeList(Node *head)
{
    Node *current = head;
    while (current)
    {
        Node *next = current->next;
        free(current->filename);
        free(current->url);
        free(current);
        current = next;
    }
}

bool llContainsString(Node *head, const char *filename)
{
    return llFindNode(head, filename) != NULL;
}

Node* llFindNode(Node *head, const char *filename)
{
    Node *current = head;
    while (current)
    {
        // Just check filename because it is filename
        // that must be unique on file system
        if (strcmp(current->filename, filename) == 0)
        {
            return current;
        }

        current = current->next;
    }

    return NULL;
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
