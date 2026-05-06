#ifndef SENTENCE_LL_H
#define SENTENCE_LL_H

/**
 * @file sentence_ll.h
 * @brief Linked list implementation for sentences.
 */

#include "word_bst.h"

/**
 * @struct SentenceNode
 * @brief Represents a sentence, storing its unique words in a BST.
 */
typedef struct SentenceNode {
    int id;                     // Position of the sentence
    WordNode *root;             // BST of words in this sentence
    struct SentenceNode *next;  // Pointer to the next sentence
} SentenceNode;

/**
 * @struct SentenceList
 * @brief A linked list of sentences.
 */
typedef struct SentenceList {
    SentenceNode *head;
    int count;                  // Total number of sentences
} SentenceList;

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
