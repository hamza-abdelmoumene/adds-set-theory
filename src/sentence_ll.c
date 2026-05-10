#include <stdio.h>
#include <stdlib.h>
#include "../include/sentence_ll.h"

// Initialize an empty SentenceList.
SentenceList CreateSentenceList(void)
{
    SentenceList list = {NULL, NULL, 0, NULL, 0};
    return list;
}

// Append a new sentence (represented by its word BST) to the list.
void AddSentence(SentenceList *list, WordNode *bst_root, const char *original)
{
    if (list == NULL)
    {
        PrintError("AddSentence", "list must not be NULL");
        return;
    }

    SentenceNode *new_node;
    Allocate(new_node);
    Ass_val(new_node, bst_root);
    Ass_adr(new_node, NULL);
    new_node->id = list->count;
    if (original)
        new_node->original = CheckedStrDup(original, "AddSentence");
    else
        new_node->original = NULL;

    if (list->head == NULL)
    {
        list->head = new_node;
        list->tail = new_node;
    }
    else
    {
        Ass_adr(list->tail, new_node);
        list->tail = new_node;
    }

    list->count++;
}

// Retrieve a sentence node by its ID (linear scan fallback).
SentenceNode *GetSentence(SentenceList list, int id)
{
    if (id < 0 || id >= list.count)
        return NULL;

    SentenceNode *current = list.head;
    while (current != NULL)
    {
        if (current->id == id)
            return current;
        current = Next(current);
    }

    return NULL;
}

// Build the index array after all sentences are added for O(1) access by id.
void BuildSentenceIndex(SentenceList *list)
{
    if (list == NULL)
    {
        PrintError("BuildSentenceIndex", "list must not be NULL");
        return;
    }

    // guard against empty list — malloc(0) is UB on some platforms
    if (list->count == 0)
    {
        list->index = NULL;
        list->capacity = 0;
        return;
    }

    list->index = (SentenceNode **)CheckedMalloc(list->count * sizeof(SentenceNode *),
                                                 "BuildSentenceIndex");

    list->capacity = list->count;

    SentenceNode *current = list->head;
    while (current != NULL)
    {
        list->index[current->id] = current;
        current = Next(current);
    }
}

// Retrieve a sentence node in O(1) using the index array.
SentenceNode *GetSentenceByIndex(SentenceList *list, int i)
{
    if (list == NULL || i < 0 || i >= list->count)
        return NULL;

    return list->index[i];
}

// Print all sentences in the list and their distinct words.
void PrintSentences(SentenceList list)
{
    SentenceNode *current = list.head;
    while (current != NULL)
    {
        printf("  Sentence %d: ", current->id);
        Inorder(current->val);
        printf("\n");
        current = Next(current);
    }
}

// Free all sentences, their associated BSTs, and the index array from memory.
void FreeSentenceList(SentenceList *list)
{
    if (list == NULL)
        return;

    SentenceNode *current = list->head;

    while (current != NULL)
    {
        SentenceNode *temp = Next(current);
        FreeTree(&current->val);
        free(current->original);
        Free(current);
        current = temp;
    }

    // free the index array itself (the nodes it pointed to are already freed above)
    free(list->index);

    list->head = NULL;
    list->tail = NULL;
    list->index = NULL;
    list->count = 0;
    list->capacity = 0;
}
