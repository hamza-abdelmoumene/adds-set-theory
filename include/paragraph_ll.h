/*
 * paragraph_ll.h - A linked list to store paragraphs.
 * Each paragraph contains a list of sentences and its own raw text.
 */

#ifndef PARAGRAPH_LL_H
#define PARAGRAPH_LL_H

#include <stdlib.h>
#include "sentence_ll.h"

/* One paragraph in our document */
typedef struct ParagraphNode
{
    int id;                     // Paragraph number (0, 1, 2...)
    SentenceList val;           // The list of sentences in this paragraph
    char *original;             // The raw text of the whole paragraph
    struct ParagraphNode *addr; // Pointer to the next paragraph
} ParagraphNode;

/* The header for the list of paragraphs */
typedef struct ParagraphList
{
    ParagraphNode *head;       // First paragraph
    ParagraphNode *tail;       // Last paragraph
    int count;                 // Total count
    ParagraphNode **index;     // Shortcut array (O(1) access)
    size_t capacity;       
} ParagraphList;

/* Starts a new, empty list of paragraphs */
ParagraphList CreateParagraphList(void);

/* Adds a paragraph to the end of the list */
void AddParagraph(ParagraphList *list, SentenceList sentence_list, const char *original);

/* Finds a paragraph by walking the list (slow) */
ParagraphNode *GetParagraph(ParagraphList list, int id);

/* Prints all paragraphs and their sentences to the console */
void PrintParagraphs(ParagraphList list);

/* Wipes everything: paragraphs, sentences, and words! */
void FreeParagraphList(ParagraphList *list);

/* Builds the shortcut array for fast access */
void BuildIndex(ParagraphList *list);

/* Uses the shortcut array to find a paragraph instantly */
ParagraphNode *GetParagraphByIndex(ParagraphList *list, int i);

#endif 
