#ifndef SENTENCE_LL_H
#define SENTENCE_LL_H

#include <stdlib.h>
#include <stdio.h>
#include "word_bst.h"

/**
 * @file sentence_ll.h
 * @brief Linked list implementation for sentences.
 * Incorporates strict Abstract Machine naming conventions.
 */

/**
 * @struct SentenceNode
 * @brief Represents a sentence, storing its unique words in a BST.
 */
typedef struct SentenceNode
{
    int id;                    // Position of the sentence
    WordNode *val;             // BST of words in this sentence
    struct SentenceNode *addr; // Pointer to the next sentence
} SentenceNode;

/**
 * @struct SentenceList
 * @brief A linked list of sentences.
 */
typedef struct SentenceList
{
    SentenceNode *head;
    SentenceNode *tail;
    int count; // Total number of sentences
} SentenceList;

// -----------------------------------------------------------------------------
// Abstract Machine Operations for Linked List
// -----------------------------------------------------------------------------
#ifndef ABSTRACT_MACHINE_LIST_MACROS
#define ABSTRACT_MACHINE_LIST_MACROS
static inline void *AllocateImpl(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        fprintf(stderr, "Allocate: out of memory\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

#define Allocate(p) ((p) = AllocateImpl(sizeof(*(p))))
#define Free(p) free(p)
#define Ass_val(p, v) ((p)->val = (v))
#define Ass_adr(p, q) ((p)->addr = (q))
#define Value(p) ((p)->val)
#define Next(p) ((p)->addr)
#endif // ABSTRACT_MACHINE_LIST_MACROS

// Initialize an empty SentenceList.
SentenceList CreateSentenceList(void);

// Append a new sentence (represented by its word BST) to the list.
void AddSentence(SentenceList *list, WordNode *bst_root);

// Retrieve a sentence node by its ID.
SentenceNode *GetSentence(SentenceList list, int id);

// Print all sentences in the list and their distinct words.
void PrintSentences(SentenceList list);

// Free all sentences and their associated BSTs from memory.
void FreeSentenceList(SentenceList *list);

#endif // SENTENCE_LL_H
