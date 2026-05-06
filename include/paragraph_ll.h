#ifndef PARAGRAPH_LL_H
#define PARAGRAPH_LL_H

/**
 * @file paragraph_ll.h
 * @brief Linked list implementation for paragraphs.
 */

#include "sentence_ll.h"

/**
 * @struct ParagraphNode
 * @brief Represents a paragraph, containing a list of sentences.
 */
typedef struct ParagraphNode {
    int id;                     // Position of the paragraph
    SentenceList sentences;     // Sentences contained in this paragraph
    struct ParagraphNode *next; // Pointer to the next paragraph
} ParagraphNode;

/**
 * @struct ParagraphList
 * @brief A linked list of paragraphs.
 */
typedef struct ParagraphList {
    ParagraphNode *head;
    int count;                  // Total number of paragraphs
} ParagraphList;

// Initialize an empty ParagraphList.
ParagraphList CreateParagraphList(void);

// Append a new paragraph (represented by its sentence list) to the list.
void AddParagraph(ParagraphList *list, SentenceList sentence_list);

// Retrieve a paragraph node by its ID.
ParagraphNode *GetParagraph(ParagraphList list, int id);

// Print all paragraphs, visualizing their sentences and words.
void PrintParagraphs(ParagraphList list);

// Free all paragraphs, sentences, and BSTs from memory.
void FreeParagraphList(ParagraphList *list);

#endif // PARAGRAPH_LL_H
