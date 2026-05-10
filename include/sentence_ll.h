#ifndef SENTENCE_LL_H
#define SENTENCE_LL_H

#include <stdlib.h>
#include <stdio.h>
#include "word_bst.h"
#include "utils.h"

typedef struct SentenceNode
{
    int id;                    
    WordNode *val;             
    char *original;            
    struct SentenceNode *addr; 
} SentenceNode;

typedef struct SentenceList
{
    SentenceNode  *head;
    SentenceNode  *tail;
    int            count;    
    SentenceNode **index;    
    size_t         capacity; 
} SentenceList;

#ifndef ABSTRACT_MACHINE_LIST_MACROS
#define ABSTRACT_MACHINE_LIST_MACROS
static inline void *AllocateImpl(size_t size)
{
    return CheckedMalloc(size, "Allocate");
}

#define Allocate(p)    ((p) = AllocateImpl(sizeof(*(p))))
#define Free(p)        free(p)
#define Ass_val(p, v)  ((p)->val  = (v))
#define Ass_adr(p, q)  ((p)->addr = (q))
#define Value(p)       ((p)->val)
#define Next(p)        ((p)->addr)
#endif 


SentenceList CreateSentenceList(void);


void AddSentence(SentenceList *list, WordNode *bst_root, const char *original);


SentenceNode *GetSentence(SentenceList list, int id);


void BuildSentenceIndex(SentenceList *list);


SentenceNode *GetSentenceByIndex(SentenceList *list, int i);


void PrintSentences(SentenceList list);


void FreeSentenceList(SentenceList *list);

#endif 
