#ifndef SETS_OPS_H
#define SETS_OPS_H

#include <stddef.h>

#include "word_bst.h"
#include "sentence_ll.h"
#include "paragraph_ll.h"

WordNode *WordUnion(WordNode *A, WordNode *B);


WordNode *WordIntersection(WordNode *A, WordNode *B);


WordNode *WordDifference(WordNode *A, WordNode *B);

SentenceList SentenceUnion(SentenceList A, SentenceList B);


SentenceList SentenceIntersection(SentenceList A, SentenceList B);


SentenceList SentenceDifference(SentenceList A, SentenceList B);

ParagraphList ParagraphUnion(ParagraphList A, ParagraphList B);


ParagraphList ParagraphIntersection(ParagraphList A, ParagraphList B);


ParagraphList ParagraphDifference(ParagraphList A, ParagraphList B);

size_t WordCardinality(WordNode *A);
size_t SentenceCardinality(SentenceList A);
size_t ParagraphCardinality(ParagraphList A);

bool WordIsSubset(WordNode *A, WordNode *B);
bool SentenceIsSubset(SentenceList A, SentenceList B);
bool ParagraphIsSubset(ParagraphList A, ParagraphList B);

#endif 
