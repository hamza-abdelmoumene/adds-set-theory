/*
 * sentence_ll.h - A linked list to store sentences.
 * Each sentence has its own BST of words and the original text.
 */

#ifndef SENTENCE_LL_H
#define SENTENCE_LL_H

#include <stdlib.h>
#include <stdio.h>
#include "word_bst.h"
#include "utils.h"

/* A single item in our sentence list */
typedef struct SentenceNode
{
    int id;                    // The position of the sentence (0, 1, 2...)
    WordNode *val;             // The set of words (stored in a BST)
    char *original;            // The original sentence text as it was in the file
    struct SentenceNode *addr; // Pointer to the next sentence
} SentenceNode;

/* The header that keeps track of the whole list of sentences */
typedef struct SentenceList
{
    SentenceNode  *head;      // First sentence
    SentenceNode  *tail;      // Last sentence (for fast appending)
    int            count;     // Total number of sentences
    SentenceNode **index;     // An array for "shortcut" access (O(1))
    size_t         capacity; 
} SentenceList;

/* 
 * Abstract Machine Macros for Linked Lists.
 * These match the "Machine Abstraite" notation used in class.
 */
#ifndef ABSTRACT_MACHINE_LIST_MACROS
#define ABSTRACT_MACHINE_LIST_MACROS
static inline void *AllocateImpl(size_t size)
{
    return CheckedMalloc(size, "Allocate");
}

#define Allocate(p)    ((p) = AllocateImpl(sizeof(*(p)))) // Allocate memory for a node
#define Free(p)        free(p)                             // Free memory
#define Ass_val(p, v)  ((p)->val  = (v))                   // Assign value field
#define Ass_adr(p, q)  ((p)->addr = (q))                   // Assign address (next) field
#define Value(p)       ((p)->val)                          // Get value field
#define Next(p)        ((p)->addr)                         // Get address field
#endif 

/* Start a fresh, empty list of sentences */
SentenceList CreateSentenceList(void);

/* Adds a sentence to the end of the list */
void AddSentence(SentenceList *list, WordNode *bst_root, const char *original);

/* Finds a sentence by searching one by one (slow) */
SentenceNode *GetSentence(SentenceList list, int id);

/* Creates the shortcut array (index) so we can find sentences instantly */
void BuildSentenceIndex(SentenceList *list);

/* Uses the shortcut array to find a sentence (very fast) */
SentenceNode *GetSentenceByIndex(SentenceList *list, int i);

/* Shows all sentences on the screen (mostly for debugging) */
void PrintSentences(SentenceList list);

/* Wipes the whole list and its BSTs from memory */
void FreeSentenceList(SentenceList *list);

#endif 
