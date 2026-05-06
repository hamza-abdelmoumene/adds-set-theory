#include <stdio.h>
#include <stdlib.h>
#include "../include/paragraph_ll.h"

// Initialize an empty ParagraphList.
ParagraphList CreateParagraphList(void)
{
    ParagraphList list = {NULL, 0};
    // TODO: Implement initialization
    
    return list;
}

// Append a new paragraph (represented by its sentence list) to the list.
void AddParagraph(ParagraphList *list, SentenceList sentence_list)
{
    // TODO: Implement append logic using Allocate(), Ass_val(), Ass_adr(), Next()
}

// Retrieve a paragraph node by its ID.
ParagraphNode *GetParagraph(ParagraphList list, int id)
{
    // TODO: Implement list traversal using Next()
    return NULL;
}

// Print all paragraphs, visualizing their sentences and words.
void PrintParagraphs(ParagraphList list)
{
    // TODO: Implement list traversal using Next() and Value()
}

// Free all paragraphs, sentences, and BSTs from memory.
void FreeParagraphList(ParagraphList *list)
{
    // TODO: Implement iterative freeing using Next() and Free()
}
