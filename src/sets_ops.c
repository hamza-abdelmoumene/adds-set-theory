#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "../include/sets_ops.h"

// Tells us if two BSTs have exactly the same words.
// It works by putting all words into sorted lists and comparing them.
static bool BSTsEqual(WordNode *A, WordNode *B)
{
    char **wA = NULL, **wB = NULL;
    size_t sA = 0, sB = 0, cA = 0, cB = 0;

    CollectWords(A, &wA, &sA, &cA); // Turn tree A into a sorted list
    CollectWords(B, &wB, &sB, &cB); // Turn tree B into a sorted list

    bool eq = (sA == sB); // Must have the same number of words
    if (eq)
    {
        for (size_t i = 0; i < sA; i++)
        {
            if (strcmp(wA[i], wB[i]) != 0) // Words must match exactly
            {
                eq = false;
                break;
            }
        }
    }

    // Clean up the temporary lists
    for (size_t i = 0; i < sA; i++) free(wA[i]);
    free(wA);
    for (size_t i = 0; i < sB; i++) free(wB[i]);
    free(wB);

    return eq;
}

// Checks if a sentence (BST) is already inside a sentence list.
static bool SentenceExistsIn(WordNode *bst, SentenceList list)
{
    SentenceNode *cur = list.head;
    while (cur != NULL)
    {
        if (BSTsEqual(bst, cur->val)) // Use our equality helper
            return true;
        cur = Next(cur);
    }
    return false;
}

// Creates an exact duplicate of a sentence list.
static SentenceList CopySentenceList(SentenceList src)
{
    SentenceList r = CreateSentenceList();
    SentenceNode *cur = src.head;
    while (cur != NULL)
    {
        WordNode *copy = NULL;
        CopyTree(cur->val, &copy); // Duplicate the word tree too
        AddSentence(&r, copy, cur->original);
        cur = Next(cur);
    }
    return r;
}

// Checks if two sentence lists have the same sentences (order doesn't matter).
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

// Checks if a paragraph is already inside a paragraph list.
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

// UNION of two word sets: everything in A plus everything in B.
WordNode *WordUnion(WordNode *A, WordNode *B)
{
    WordNode *result = NULL;
    CopyTree(A, &result); // Put all of A into result
    CopyTree(B, &result); // Put all of B into result (duplicates are ignored automatically)
    return result;
}

// INTERSECTION of two word sets: only words that appear in BOTH A and B.
WordNode *WordIntersection(WordNode *A, WordNode *B)
{
    char **words = NULL;
    size_t size = 0, cap = 0;
    CollectWords(A, &words, &size, &cap); // Get all words from A

    size_t keep = 0;
    for (size_t i = 0; i < size; i++)
    {
        WordNode *p = NULL, *q = NULL;
        Search(words[i], B, &p, &q); // Check if word[i] is also in B
        if (p != NULL)
            words[keep++] = words[i]; // Found in both, keep it!
        else
            free(words[i]); // Not in B, throw it away
    }

    WordNode *result = NULL;
    if (keep > 0)
        MedianInsert(words, 0, keep - 1, &result); // Build a balanced tree from kept words

    for (size_t i = 0; i < keep; i++) free(words[i]);
    free(words);
    return result;
}

// DIFFERENCE of two word sets (A \ B): words that are in A but NOT in B.
WordNode *WordDifference(WordNode *A, WordNode *B)
{
    char **words = NULL;
    size_t size = 0, cap = 0;
    CollectWords(A, &words, &size, &cap); // Get all words from A

    size_t keep = 0;
    for (size_t i = 0; i < size; i++)
    {
        WordNode *p = NULL, *q = NULL;
        Search(words[i], B, &p, &q);
        if (p == NULL)
            words[keep++] = words[i]; // Not in B, so we keep it!
        else
            free(words[i]); // Found in B, so we discard it
    }

    WordNode *result = NULL;
    if (keep > 0)
        MedianInsert(words, 0, keep - 1, &result);

    for (size_t i = 0; i < keep; i++) free(words[i]);
    free(words);
    return result;
}

// UNION of two sentence sets.
SentenceList SentenceUnion(SentenceList A, SentenceList B)
{
    SentenceList result = CopySentenceList(A); // Start with everything in A

    SentenceNode *cur = B.head;
    while (cur != NULL)
    {
        if (!SentenceExistsIn(cur->val, result)) // If it's not already in result, add it
        {
            WordNode *copy = NULL;
            CopyTree(cur->val, &copy);
            AddSentence(&result, copy, cur->original);
        }
        cur = Next(cur);
    }

    return result;
}

// INTERSECTION of two sentence sets.
SentenceList SentenceIntersection(SentenceList A, SentenceList B)
{
    SentenceList result = CreateSentenceList();

    SentenceNode *cur = A.head;
    while (cur != NULL)
    {
        if (SentenceExistsIn(cur->val, B)) // Keep only if it exists in both
        {
            WordNode *copy = NULL;
            CopyTree(cur->val, &copy);
            AddSentence(&result, copy, cur->original);
        }
        cur = Next(cur);
    }

    return result;
}

// DIFFERENCE of two sentence sets (A \ B).
SentenceList SentenceDifference(SentenceList A, SentenceList B)
{
    SentenceList result = CreateSentenceList();

    SentenceNode *cur = A.head;
    while (cur != NULL)
    {
        if (!SentenceExistsIn(cur->val, B)) // Keep only if NOT in B
        {
            WordNode *copy = NULL;
            CopyTree(cur->val, &copy);
            AddSentence(&result, copy, cur->original);
        }
        cur = Next(cur);
    }

    return result;
}

// UNION of two paragraph sets.
ParagraphList ParagraphUnion(ParagraphList A, ParagraphList B)
{
    ParagraphList result = CreateParagraphList();

    // Add everything from A
    ParagraphNode *cur = A.head;
    while (cur != NULL)
    {
        SentenceList copy = CopySentenceList(cur->val);
        AddParagraph(&result, copy, cur->original);
        cur = Next(cur);
    }

    // Add everything from B that isn't already in A
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

// INTERSECTION of two paragraph sets.
ParagraphList ParagraphIntersection(ParagraphList A, ParagraphList B)
{
    ParagraphList result = CreateParagraphList();

    ParagraphNode *cur = A.head;
    while (cur != NULL)
    {
        if (ParagraphExistsIn(cur->val, B)) // Keep only if it exists in both
        {
            SentenceList copy = CopySentenceList(cur->val);
            AddParagraph(&result, copy, cur->original);
        }
        cur = Next(cur);
    }

    return result;
}

// DIFFERENCE of two paragraph sets (A \ B).
ParagraphList ParagraphDifference(ParagraphList A, ParagraphList B)
{
    ParagraphList result = CreateParagraphList();

    ParagraphNode *cur = A.head;
    while (cur != NULL)
    {
        if (!ParagraphExistsIn(cur->val, B)) // Keep only if NOT in B
        {
            SentenceList copy = CopySentenceList(cur->val);
            AddParagraph(&result, copy, cur->original);
        }
        cur = Next(cur);
    }

    return result;
}

// Counts nodes in a word BST.
size_t WordCardinality(WordNode *A)
{
    if (A == NULL)
        return 0;

    // The size is 1 (the current node) + size of left side + size of right side.
    return 1 + WordCardinality(LC(A)) + WordCardinality(RC(A));
}

// Returns the number of sentences in the list.
size_t SentenceCardinality(SentenceList A)
{
    return (A.count < 0) ? 0U : (size_t)A.count;
}

// Returns the number of paragraphs in the list.
size_t ParagraphCardinality(ParagraphList A)
{
    return (A.count < 0) ? 0U : (size_t)A.count;
}

// Check whether set A is a subset of set B (every word in A must be in B).
bool WordIsSubset(WordNode *A, WordNode *B)
{
    WordNode *p = NULL;
    WordNode *q = NULL;

    if (A == NULL)
        return true; // Empty set is always a subset!

    Search(NodeValue(A), B, &p, &q); // Check if the current word of A is in B
    if (p == NULL)
        return false; // Found a word in A that isn't in B!

    // Check all other words in the left and right sides of the tree
    return WordIsSubset(LC(A), B) && WordIsSubset(RC(A), B);
}

// Check whether every sentence in A is also in B.
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

// Check whether every paragraph in A is also in B.
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
