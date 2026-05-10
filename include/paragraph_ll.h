#ifndef PARAGRAPH_LL_H
#define PARAGRAPH_LL_H

#include <stdlib.h>
#include "sentence_ll.h"

typedef struct ParagraphNode
{
    int id;                     
    SentenceList val;           
    char *original;             
    struct ParagraphNode *addr; 
} ParagraphNode;

typedef struct ParagraphList
{
    ParagraphNode *head;
    ParagraphNode *tail;
    int count;             
    ParagraphNode **index; 
    size_t capacity;       
} ParagraphList;

ParagraphList CreateParagraphList(void);


void AddParagraph(ParagraphList *list, SentenceList sentence_list, const char *original);


ParagraphNode *GetParagraph(ParagraphList list, int id);


void PrintParagraphs(ParagraphList list);


void FreeParagraphList(ParagraphList *list);


void BuildIndex(ParagraphList *list);


ParagraphNode *GetParagraphByIndex(ParagraphList *list, int i);

#endif 
