#ifndef PARAGRAPH_LL_H
#define PARAGRAPH_LL_H

#include <stdlib.h>
#include "sentence_ll.h"

/**
 * @file paragraph_ll.h
 * @brief Linked list implementation for paragraphs.
 * Incorporates strict Abstract Machine naming conventions.
 */

/**
 * @struct ParagraphNode
 * @brief Represents a paragraph, containing a list of sentences.
 */
typedef struct ParagraphNode
{
    int id;                     // Position of the paragraph
    SentenceList val;           // Sentences contained in this paragraph
    struct ParagraphNode *addr; // Pointer to the next paragraph
} ParagraphNode;

/**
 * @struct ParagraphList
 * @brief A linked list of paragraphs with a parallel index for O(1) access.
 */
typedef struct ParagraphList
{
    ParagraphNode *head;
    ParagraphNode *tail;
    int count;             // Total number of paragraphs
    ParagraphNode **index; // Dynamic array of node pointers (by id)
    size_t capacity;       // Allocated size of index array
} ParagraphList;

// Linked List Abstract Machine Operations are inherited from sentence_ll.h
// Allocate(p), Free(p), Ass_val(p, v), Ass_adr(p, q), Value(p), Next(p)

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

// Build the index array after parsing for O(1) access by index.
void BuildIndex(ParagraphList *list);

// Retrieve a paragraph node by index from the built array.
ParagraphNode *GetParagraphByIndex(ParagraphList *list, int i);

#endif // PARAGRAPH_LL_H
