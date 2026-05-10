#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../include/sets_ops.h"

// Compare two BSTs by their sorted word lists.
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

    for (size_t i = 0; i < sA; i++)
        free(wA[i]);
    free(wA);
    for (size_t i = 0; i < sB; i++)
        free(wB[i]);
    free(wB);

    return eq;
}

// Check if a sentence BST exists in a sentence list.
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

// Deep-copy a sentence list with duplicated BSTs.
static SentenceList CopySentenceList(SentenceList src)
{
    SentenceList r = CreateSentenceList();
    SentenceNode *cur = src.head;
    while (cur != NULL)
    {
        WordNode *copy = NULL;
        CopyTree(cur->val, &copy);
        AddSentence(&r, copy, cur->original);
        cur = Next(cur);
    }
    return r;
}

// Check if two sentence lists represent the same set.
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

// Check if a paragraph (sentence list) exists in a paragraph list.
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

// Return the union of two word BSTs.
WordNode *WordUnion(WordNode *A, WordNode *B)
{
    WordNode *result = NULL;
    CopyTree(A, &result);
    CopyTree(B, &result);
    return result;
}

// Return the intersection of two word BSTs.
WordNode *WordIntersection(WordNode *A, WordNode *B)
{

    char **words = NULL;
    size_t size = 0, cap = 0;
    CollectWords(A, &words, &size, &cap);

    size_t keep = 0;
    for (size_t i = 0; i < size; i++)
    {
        WordNode *p = NULL, *q = NULL;
        Search(words[i], B, &p, &q);
        if (p != NULL)
            words[keep++] = words[i];
        else
            free(words[i]);
    }

    WordNode *result = NULL;
    if (keep > 0)
        MedianInsert(words, 0, keep - 1, &result);

    for (size_t i = 0; i < keep; i++)
        free(words[i]);
    free(words);
    return result;
}

// Return the difference of two word BSTs (A \ B).
WordNode *WordDifference(WordNode *A, WordNode *B)
{

    char **words = NULL;
    size_t size = 0, cap = 0;
    CollectWords(A, &words, &size, &cap);

    size_t keep = 0;
    for (size_t i = 0; i < size; i++)
    {
        WordNode *p = NULL, *q = NULL;
        Search(words[i], B, &p, &q);
        if (p == NULL)
            words[keep++] = words[i];
        else
            free(words[i]);
    }

    WordNode *result = NULL;
    if (keep > 0)
        MedianInsert(words, 0, keep - 1, &result);

    for (size_t i = 0; i < keep; i++)
        free(words[i]);
    free(words);
    return result;
}

// Return the union of two sentence lists.
SentenceList SentenceUnion(SentenceList A, SentenceList B)
{

    SentenceList result = CopySentenceList(A);

    SentenceNode *cur = B.head;
    while (cur != NULL)
    {
        if (!SentenceExistsIn(cur->val, result))
        {
            WordNode *copy = NULL;
            CopyTree(cur->val, &copy);
            AddSentence(&result, copy, cur->original);
        }
        cur = Next(cur);
    }

    return result;
}

// Return the intersection of two sentence lists.
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
            AddSentence(&result, copy, cur->original);
        }
        cur = Next(cur);
    }

    return result;
}

// Return the difference of two sentence lists (A \ B).
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
            AddSentence(&result, copy, cur->original);
        }
        cur = Next(cur);
    }

    return result;
}

// Return the union of two paragraph lists.
ParagraphList ParagraphUnion(ParagraphList A, ParagraphList B)
{
    ParagraphList result = CreateParagraphList();

    ParagraphNode *cur = A.head;
    while (cur != NULL)
    {
        SentenceList copy = CopySentenceList(cur->val);
        AddParagraph(&result, copy, cur->original);
        cur = Next(cur);
    }

    cur = B.head;
    while (cur != NULL)
    {
        if (!ParagraphExistsIn(cur->val, result))
        {
            SentenceList copy = CopySentenceList(cur->val);
            AddParagraph(&result, copy, cur->original);
        }
        cur = Next(cur);
    }

    return result;
}

// Return the intersection of two paragraph lists.
ParagraphList ParagraphIntersection(ParagraphList A, ParagraphList B)
{
    ParagraphList result = CreateParagraphList();

    ParagraphNode *cur = A.head;
    while (cur != NULL)
    {
        if (ParagraphExistsIn(cur->val, B))
        {
            SentenceList copy = CopySentenceList(cur->val);
            AddParagraph(&result, copy, cur->original);
        }
        cur = Next(cur);
    }

    return result;
}

// Return the difference of two paragraph lists (A \ B).
ParagraphList ParagraphDifference(ParagraphList A, ParagraphList B)
{
    ParagraphList result = CreateParagraphList();

    ParagraphNode *cur = A.head;
    while (cur != NULL)
    {
        if (!ParagraphExistsIn(cur->val, B))
        {
            SentenceList copy = CopySentenceList(cur->val);
            AddParagraph(&result, copy, cur->original);
        }
        cur = Next(cur);
    }

    return result;
}

// Count nodes in a word BST.
size_t WordCardinality(WordNode *A)
{
    if (A == NULL)
        return 0;

    return 1 + WordCardinality(LC(A)) + WordCardinality(RC(A));
}

// Return the size of a sentence list.
size_t SentenceCardinality(SentenceList A)
{
    return (A.count < 0) ? 0U : (size_t)A.count;
}

// Return the size of a paragraph list.
size_t ParagraphCardinality(ParagraphList A)
{
    return (A.count < 0) ? 0U : (size_t)A.count;
}

// Check whether A is a subset of B at word level.
bool WordIsSubset(WordNode *A, WordNode *B)
{
    WordNode *p = NULL;
    WordNode *q = NULL;

    if (A == NULL)
        return true;

    Search(NodeValue(A), B, &p, &q);
    if (p == NULL)
        return false;

    return WordIsSubset(LC(A), B) && WordIsSubset(RC(A), B);
}

// Check whether A is a subset of B at sentence level.
bool SentenceIsSubset(SentenceList A, SentenceList B)
{
    SentenceNode *cur = A.head;

    while (cur != NULL)
    {
        if (!SentenceExistsIn(cur->val, B))
            return false;
        cur = Next(cur);
    }

    return true;
}

// Check whether A is a subset of B at paragraph level.
bool ParagraphIsSubset(ParagraphList A, ParagraphList B)
{
    ParagraphNode *cur = A.head;

    while (cur != NULL)
    {
        if (!ParagraphExistsIn(cur->val, B))
            return false;
        cur = Next(cur);
    }

    return true;
}
