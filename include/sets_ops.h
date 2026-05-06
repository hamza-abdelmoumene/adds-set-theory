#ifndef SETS_OPS_H
#define SETS_OPS_H

/**
 * @file sets_ops.h
 * @brief Set theory operations across Words, Sentences, and Paragraphs.
 */

#include "word_bst.h"
#include "sentence_ll.h"
#include "paragraph_ll.h"

// -----------------------------------------------------------------------------
// Word Level Set Operations (Operates on BSTs)
// -----------------------------------------------------------------------------

// Union: Elements in either A or B
WordNode *WordUnion(WordNode *A, WordNode *B);

// Intersection: Elements in both A and B
WordNode *WordIntersection(WordNode *A, WordNode *B);

// Difference: Elements in A but NOT in B
WordNode *WordDifference(WordNode *A, WordNode *B);

// -----------------------------------------------------------------------------
// Sentence Level Set Operations (Operates on SentenceLists)
// -----------------------------------------------------------------------------

// Union: Merge sentences from A and B into a new list
SentenceList SentenceUnion(SentenceList A, SentenceList B);

// Intersection: Sentences from A and B whose word sets intersect
SentenceList SentenceIntersection(SentenceList A, SentenceList B);

// Difference: Sentences in A whose content does not appear in B
SentenceList SentenceDifference(SentenceList A, SentenceList B);

// -----------------------------------------------------------------------------
// Paragraph Level Set Operations (Operates on ParagraphLists)
// -----------------------------------------------------------------------------

// Union: All paragraphs from both lists combined
ParagraphList ParagraphUnion(ParagraphList A, ParagraphList B);

// Intersection: Paragraphs from A and B whose sentence sets intersect
ParagraphList ParagraphIntersection(ParagraphList A, ParagraphList B);

// Difference: Paragraphs in A not present in B
ParagraphList ParagraphDifference(ParagraphList A, ParagraphList B);

#endif // SETS_OPS_H
