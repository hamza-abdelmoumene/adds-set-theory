#include <stdio.h>
#include <stdlib.h>
#include "../include/sentence_ll.h"

// Initialize an empty SentenceList.
SentenceList CreateSentenceList(void)
{
    SentenceList list = {NULL, 0};
    // TODO: Implement initialization
    return list;
}

// Append a new sentence (represented by its word BST) to the list using abstract machine operations.
void AddSentence(SentenceList *list, WordNode *bst_root)
{
    // TODO: Implement append logic using Allocate(), Ass_val(), Ass_adr(), Next()
}

// Retrieve a sentence node by its ID.
SentenceNode *GetSentence(SentenceList list, int id)
{
    // TODO: Implement list traversal using Next()
    return NULL;
}

// Print all sentences in the list and their distinct words.
void PrintSentences(SentenceList list)
{
    // TODO: Implement list traversal using Next() and Value()
}

// Free all sentences and their associated BSTs from memory.
void FreeSentenceList(SentenceList *list)
{
    // TODO: Implement iterative freeing using Next() and Free()
}
