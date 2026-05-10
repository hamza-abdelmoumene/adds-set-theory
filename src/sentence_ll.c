#include <stdio.h>
#include <stdlib.h>
#include "../include/sentence_ll.h"

// Sets up a new, empty list structure.
SentenceList CreateSentenceList(void)
{
    SentenceList list = {NULL, NULL, 0, NULL, 0};
    return list;
}

// Appends a sentence to the end of the list.
void AddSentence(SentenceList *list, WordNode *bst_root, const char *original)
{
    if (list == NULL) return;

    SentenceNode *new_node;
    Allocate(new_node); // Allocate using our macro
    Ass_val(new_node, bst_root);
    Ass_adr(new_node, NULL);
    new_node->id = list->count; // Sequential ID
    
    // Store a copy of the raw text so we can show it in the UI later
    if (original)
        new_node->original = CheckedStrDup(original, "AddSentence");
    else
        new_node->original = NULL;

    // Standard tail insertion for a linked list
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

// Searches for a sentence by ID by walking from the start of the list.
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

// This function builds an array that points to each node in our list.
// Why? Because jumping to the 100th element in an array is much faster 
// than walking through 100 nodes in a linked list.
void BuildSentenceIndex(SentenceList *list)
{
    if (list == NULL || list->count == 0) return;

    // Allocate an array of pointers
    list->index = (SentenceNode **)CheckedMalloc(list->count * sizeof(SentenceNode *),
                                                 "BuildSentenceIndex");

    list->capacity = list->count;

    // Fill the array by walking the list one last time
    SentenceNode *current = list->head;
    while (current != NULL)
    {
        list->index[current->id] = current;
        current = Next(current);
    }
}

// Uses our "shortcut" index to get a sentence instantly.
SentenceNode *GetSentenceByIndex(SentenceList *list, int i)
{
    if (list == NULL || i < 0 || i >= list->count)
        return NULL;

    return list->index[i];
}

// Simple loop to print all sentences.
void PrintSentences(SentenceList list)
{
    SentenceNode *current = list.head;
    while (current != NULL)
    {
        printf("  Sentence %d: ", current->id);
        Inorder(current->val); // Print the words in order
        printf("\n");
        current = Next(current);
    }
}

// Deep cleanup: frees every node, every word tree, and the raw text strings.
void FreeSentenceList(SentenceList *list)
{
    if (list == NULL)
        return;

    SentenceNode *current = list->head;

    while (current != NULL)
    {
        SentenceNode *temp = Next(current);
        FreeTree(&current->val); // Free the word BST
        free(current->original);  // Free the original text
        Free(current);           // Free the node itself
        current = temp;
    }

    free(list->index); // Free the shortcut array

    // Zero everything out to be safe
    list->head = NULL;
    list->tail = NULL;
    list->index = NULL;
    list->count = 0;
    list->capacity = 0;
}
