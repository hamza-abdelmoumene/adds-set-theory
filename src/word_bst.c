#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/word_bst.h"

// Walks through the tree to find a word.
// It returns two things: *p is the node if found, and *q is its parent.
void Search(const char *word, WordNode *r, WordNode **p, WordNode **q)
{
    // Basic checks to make sure we aren't searching for nothing
    if (p == NULL || q == NULL || word == NULL || word[0] == '\0' || strlen(word) >= MAX_WORD)
    {
        if (p) *p = NULL;
        if (q) *q = NULL;
        return;
    }

    *p = r;     // Start at the root
    *q = NULL;  // Root has no parent

    while (*p != NULL)
    {
        int cmp = strcmp(word, NodeValue(*p));
        if (cmp == 0)
            return; // Found it!

        *q = *p; // Keep track of parent before moving down
        *p = (cmp < 0) ? LC(*p) : RC(*p); // Go left if smaller, right if bigger
    }
}

// Inserts a new word into the tree.
// If the word is already there, it skips it (set theory rule!).
bool Insert(const char *word, WordNode **r)
{
    if (r == NULL || word == NULL || word[0] == '\0' || strlen(word) >= MAX_WORD)
        return false;

    WordNode *p = NULL;
    WordNode *q = NULL;

    // See if it already exists
    Search(word, *r, &p, &q);
    if (p != NULL)
        return false; // Already in the tree, skip it.

    // Create a new node for the word
    WordNode *new_node = AllocateNode();
    Ass_Node_Val(new_node, word);

    // If the tree was empty, this is the new root
    if (q == NULL)
    {
        *r = new_node;
        return true;
    }

    // Otherwise, attach it to the parent we found during Search
    if (strcmp(word, NodeValue(q)) < 0)
        Ass_LC(q, new_node);
    else
        Ass_RC(q, new_node);

    return true;
}

// Prints everything in the tree from A to Z.
// We go: left child -> current node -> right child.
void Inorder(WordNode *r)
{
    if (r == NULL)
        return;

    Inorder(LC(r));
    printf("%s ", NodeValue(r));
    Inorder(RC(r));
}

// Cleans up the whole tree to save memory.
// We must free the children BEFORE the parent, or we'll lose them!
void FreeTree(WordNode **r)
{
    if (r == NULL || *r == NULL)
        return;

    FreeTree(&((*r)->left));
    FreeTree(&((*r)->right));
    FreeNode(*r);
    *r = NULL;
}

// Copies all words from the 'src' tree into the 'dest' tree.
void CopyTree(WordNode *src, WordNode **dest)
{
    if (src == NULL)
        return;

    Insert(NodeValue(src), dest); // Add the current word
    CopyTree(LC(src), dest);      // Copy left side
    CopyTree(RC(src), dest);      // Copy right side
}

// Puts all words from the tree into a simple list (array).
// This is useful for sorting or balancing the tree later.
void CollectWords(WordNode *root, char ***array, size_t *size, size_t *capacity)
{
    if (array == NULL || size == NULL || capacity == NULL || root == NULL)
        return;

    CollectWords(LC(root), array, size, capacity);

    // Grow the array if it's full
    if (*capacity == 0)
    {
        *capacity = 8;
        *array = (char **)CheckedMalloc((*capacity) * sizeof(char *), "CollectWords");
    }
    else if (*size >= *capacity)
    {
        size_t new_cap = (*capacity) * 2;
        *array = (char **)CheckedRealloc(*array, new_cap * sizeof(char *), "CollectWords");
        *capacity = new_cap;
    }

    // Add a copy of the word to our list
    char *copy = CheckedStrDup(NodeValue(root), "CollectWords");
    (*array)[(*size)++] = copy;

    CollectWords(RC(root), array, size, capacity);
}

// Simple alphabet sorter for our word list.
static int comp(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

void SortWords(char **array, size_t size)
{
    if (array == NULL || size == 0)
        return;

    qsort(array, size, sizeof(char *), comp);
}

// This builds a "perfect" tree by always picking the middle word first.
// This stops the tree from becoming one long line and keeps it fast!
void MedianInsert(char **array, size_t left, size_t right, WordNode **root)
{
    if (left > right)
        return;

    size_t mid = left + (right - left) / 2;

    Insert(array[mid], root); // Insert the middle element

    // Recurse on the left half and right half
    if (mid > left)
        MedianInsert(array, left, mid - 1, root);

    MedianInsert(array, mid + 1, right, root);
}
