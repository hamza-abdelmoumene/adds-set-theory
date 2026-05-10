/*
 * sets_ops.h - This is the core of the project.
 * It handles the mathematical set operations (Union, Intersection, Difference) 
 * for words, sentences, and paragraphs.
 */

#ifndef SETS_OPS_H
#define SETS_OPS_H

#include <stddef.h>

#include "word_bst.h"
#include "sentence_ll.h"
#include "paragraph_ll.h"

/* Word-level operations (using BSTs) */
WordNode *WordUnion(WordNode *A, WordNode *B);
WordNode *WordIntersection(WordNode *A, WordNode *B);
WordNode *WordDifference(WordNode *A, WordNode *B);

/* Sentence-level operations (using lists of BSTs) */
SentenceList SentenceUnion(SentenceList A, SentenceList B);
SentenceList SentenceIntersection(SentenceList A, SentenceList B);
SentenceList SentenceDifference(SentenceList A, SentenceList B);

/* Paragraph-level operations (using lists of sentence lists) */
ParagraphList ParagraphUnion(ParagraphList A, ParagraphList B);
ParagraphList ParagraphIntersection(ParagraphList A, ParagraphList B);
ParagraphList ParagraphDifference(ParagraphList A, ParagraphList B);

/* Cardinality: how many items are in the set? */
size_t WordCardinality(WordNode *A);
size_t SentenceCardinality(SentenceList A);
size_t ParagraphCardinality(ParagraphList A);

/* Subset: is every item in A also in B? */
bool WordIsSubset(WordNode *A, WordNode *B);
bool SentenceIsSubset(SentenceList A, SentenceList B);
bool ParagraphIsSubset(ParagraphList A, ParagraphList B);

#endif 
