#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../include/sets_ops.h"

 // Internal Helpers
 


//Compare two BSTs by their sorted word content.

static bool BSTsEqual(WordNode *A, WordNode *B)
{
    char **wA = NULL, **wB = NULL;
    size_t sA = 0, sB = 0, cA = 0, cB = 0;

    CollectWords(A, &wA, &sA, &cA);
    CollectWords(B, &wB, &sB, &cB);

    bool eq = (sA == sB);
    if (eq)
    {
        for (size_t i = 0; i < sA; i++)
        {
            if (strcmp(wA[i], wB[i]) != 0)
            {
                eq = false;
                break;
            }
        }
    }

    for (size_t i = 0; i < sA; i++) free(wA[i]);
    free(wA);
    for (size_t i = 0; i < sB; i++) free(wB[i]);
    free(wB);

    return eq;
}

 // Check if a sentence (identified by its word BST) has an equivalent in the given SentenceList.
static bool SentenceExistsIn(WordNode *bst, SentenceList list)
{
    SentenceNode *cur = list.head;
    while (cur != NULL)
    {
        if (BSTsEqual(bst, cur->val))
            return true;
        cur = Next(cur);
    }
    return false;
}


 // Deep-copy an entire SentenceList (every BST is duplicated via CopyTree).
 
static SentenceList CopySentenceList(SentenceList src)
{
    SentenceList r = CreateSentenceList();
    SentenceNode *cur = src.head;
    while (cur != NULL)
    {
        WordNode *copy = NULL;
        CopyTree(cur->val, &copy);
        AddSentence(&r, copy);
        cur = Next(cur);
    }
    return r;
}

//Check if two SentenceLists represent the same set of sentences.
static bool SentenceListsEqual(SentenceList A, SentenceList B)
{
    if (A.count != B.count)
        return false;

    SentenceNode *cur = A.head;
    while (cur != NULL)
    {
        if (!SentenceExistsIn(cur->val, B))
            return false;
        cur = Next(cur);
    }
    return true;
}

//Check if a paragraph (identified by its SentenceList) has an equivalent in the given ParagraphList.
static bool ParagraphExistsIn(SentenceList sentences, ParagraphList list)
{
    ParagraphNode *cur = list.head;
    while (cur != NULL)
    {
        if (SentenceListsEqual(sentences, cur->val))
            return true;
        cur = Next(cur);
    }
    return false;
}


 //  Word Level Set Operations  
 

// Union
WordNode *WordUnion(WordNode *A, WordNode *B)
{
    WordNode *result = NULL;
    CopyTree(A, &result);
    CopyTree(B, &result); // Insert silently ignores duplicates
    return result;
}

// Intersection
WordNode *WordIntersection(WordNode *A, WordNode *B)
{
    // Collect words from A (in-order → already sorted)
    char **words = NULL;
    size_t size = 0, cap = 0;
    CollectWords(A, &words, &size, &cap);

    // Filter: keep only words that also exist in B
    size_t keep = 0;
    for (size_t i = 0; i < size; i++)
    {
        WordNode *p = NULL, *q = NULL;
        Search(words[i], B, &p, &q);
        if (p != NULL)
            words[keep++] = words[i]; // reuse slot
        else
            free(words[i]);
    }

    // Build a balanced BST from the filtered (still sorted) array
    WordNode *result = NULL;
    if (keep > 0)
        MedianInsert(words, 0, keep - 1, &result);

    for (size_t i = 0; i < keep; i++) free(words[i]);
    free(words);
    return result;
}

// Difference:
WordNode *WordDifference(WordNode *A, WordNode *B)
{
    // Collect words from A (in-order → already sorted)
    char **words = NULL;
    size_t size = 0, cap = 0;
    CollectWords(A, &words, &size, &cap);

    // Filter: keep only words that do NOT exist in B
    size_t keep = 0;
    for (size_t i = 0; i < size; i++)
    {
        WordNode *p = NULL, *q = NULL;
        Search(words[i], B, &p, &q);
        if (p == NULL)
            words[keep++] = words[i]; // reuse slot
        else
            free(words[i]);
    }

    // Build a balanced BST from the filtered (still sorted) array
    WordNode *result = NULL;
    if (keep > 0)
        MedianInsert(words, 0, keep - 1, &result);

    for (size_t i = 0; i < keep; i++) free(words[i]);
    free(words);
    return result;
}


 // Sentence Level Set Operations  
 

// Union
SentenceList SentenceUnion(SentenceList A, SentenceList B)
{
    // Start with a deep copy of all sentences in A
    SentenceList result = CopySentenceList(A);

    // Append sentences from B that are not already present
    SentenceNode *cur = B.head;
    while (cur != NULL)
    {
        if (!SentenceExistsIn(cur->val, result))
        {
            WordNode *copy = NULL;
            CopyTree(cur->val, &copy);
            AddSentence(&result, copy);
        }
        cur = Next(cur);
    }

    return result;
}

// Intersection
SentenceList SentenceIntersection(SentenceList A, SentenceList B)
{
    SentenceList result = CreateSentenceList();

    SentenceNode *cur = A.head;
    while (cur != NULL)
    {
        if (SentenceExistsIn(cur->val, B))
        {
            WordNode *copy = NULL;
            CopyTree(cur->val, &copy);
            AddSentence(&result, copy);
        }
        cur = Next(cur);
    }

    return result;
}

// Difference
SentenceList SentenceDifference(SentenceList A, SentenceList B)
{
    SentenceList result = CreateSentenceList();

    SentenceNode *cur = A.head;
    while (cur != NULL)
    {
        if (!SentenceExistsIn(cur->val, B))
        {
            WordNode *copy = NULL;
            CopyTree(cur->val, &copy);
            AddSentence(&result, copy);
        }
        cur = Next(cur);
    }

    return result;
}


 // Paragraph Level Set Operations 
 

// Union
ParagraphList ParagraphUnion(ParagraphList A, ParagraphList B)
{
    ParagraphList result = CreateParagraphList();

    // Copy all paragraphs from A
    ParagraphNode *cur = A.head;
    while (cur != NULL)
    {
        SentenceList copy = CopySentenceList(cur->val);
        AddParagraph(&result, copy);
        cur = Next(cur);
    }

    // Append paragraphs from B that are not already present
    cur = B.head;
    while (cur != NULL)
    {
        if (!ParagraphExistsIn(cur->val, result))
        {
            SentenceList copy = CopySentenceList(cur->val);
            AddParagraph(&result, copy);
        }
        cur = Next(cur);
    }

    return result;
}

// Intersection
ParagraphList ParagraphIntersection(ParagraphList A, ParagraphList B)
{
    ParagraphList result = CreateParagraphList();

    ParagraphNode *cur = A.head;
    while (cur != NULL)
    {
        if (ParagraphExistsIn(cur->val, B))
        {
            SentenceList copy = CopySentenceList(cur->val);
            AddParagraph(&result, copy);
        }
        cur = Next(cur);
    }

    return result;
}

// Difference
ParagraphList ParagraphDifference(ParagraphList A, ParagraphList B)
{
    ParagraphList result = CreateParagraphList();

    ParagraphNode *cur = A.head;
    while (cur != NULL)
    {
        if (!ParagraphExistsIn(cur->val, B))
        {
            SentenceList copy = CopySentenceList(cur->val);
            AddParagraph(&result, copy);
        }
        cur = Next(cur);
    }

    return result;
}
