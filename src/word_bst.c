#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/word_bst.h"

// Main function: find a word in the BST and return node and parent.
void Search(const char *word, WordNode *r, WordNode **p, WordNode **q)
{

    if (p == NULL || q == NULL)
    {
        PrintError("Search", "output pointers must not be NULL");
        return;
    }

    if (word == NULL)
    {
        PrintError("Search", "word must not be NULL");
        *p = NULL;
        *q = NULL;
        return;
    }

    if (word[0] == '\0')
    {
        PrintError("Search", "word must not be empty");
        *p = NULL;
        *q = NULL;
        return;
    }

    if (strlen(word) >= MAX_WORD)
    {
        PrintError("Search", "word exceeds MAX_WORD");
        *p = NULL;
        *q = NULL;
        return;
    }

    *p = r;
    *q = NULL;

    while (*p != NULL)
    {
        int cmp = strcmp(word, NodeValue(*p));
        if (cmp == 0)
            return;

        *q = *p;
        *p = (cmp < 0) ? LC(*p) : RC(*p);
    }
}

// Main function: insert a word into the BST, ignoring duplicates.
bool Insert(const char *word, WordNode **r)
{

    if (r == NULL)
        return false;

    if (word == NULL)
    {
        PrintError("Insert", "word must not be NULL");
        return false;
    }

    if (word[0] == '\0')
    {
        PrintError("Insert", "word must not be empty");
        return false;
    }

    if (strlen(word) >= MAX_WORD)
    {
        PrintError("Insert", "word exceeds MAX_WORD");
        return false;
    }

    WordNode *p = NULL;
    WordNode *q = NULL;

    Search(word, *r, &p, &q);
    if (p != NULL)
        return false;

    WordNode *new_node = AllocateNode();
    Ass_Node_Val(new_node, word);

    if (q == NULL)
    {
        *r = new_node;
        return true;
    }

    if (strcmp(word, NodeValue(q)) < 0)
        Ass_LC(q, new_node);
    else
        Ass_RC(q, new_node);

    return true;
}

// Main function: print the BST in sorted order.
void Inorder(WordNode *r)
{
    if (r == NULL)
        return;

    Inorder(LC(r));
    printf("%s ", NodeValue(r));
    Inorder(RC(r));
}

// Main function: free all nodes in a BST and reset the root.
void FreeTree(WordNode **r)
{

    if (r == NULL || *r == NULL)
    {
        if (r == NULL)
            PrintError("FreeTree", "root pointer must not be NULL");
        return;
    }

    FreeTree(&((*r)->left));
    FreeTree(&((*r)->right));
    FreeNode(*r);
    *r = NULL;
}

// Main function: copy all words from one BST into another.
void CopyTree(WordNode *src, WordNode **dest)
{

    if (src == NULL)
        return;

    if (dest == NULL)
    {
        PrintError("CopyTree", "destination pointer must not be NULL");
        return;
    }

    Insert(NodeValue(src), dest);
    CopyTree(LC(src), dest);
    CopyTree(RC(src), dest);
}

// Main function: collect words into a dynamic array using in-order traversal.
void CollectWords(WordNode *root, char ***array, size_t *size, size_t *capacity)
{
    if (array == NULL || size == NULL || capacity == NULL)
    {
        PrintError("CollectWords", "output parameters must not be NULL");
        return;
    }

    if (root == NULL)
        return;

    CollectWords(LC(root), array, size, capacity);

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

    char *copy = CheckedStrDup(NodeValue(root), "CollectWords");
    (*array)[(*size)++] = copy;

    CollectWords(RC(root), array, size, capacity);
}

// Helper function: comparator for sorting words alphabetically.
static int comp(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

// Main function: sort a word array using qsort.
void SortWords(char **array, size_t size)
{
    if (array == NULL || size == 0)
        return;

    qsort(array, size, sizeof(char *), comp);
}

// Main function: insert words median-first to build a balanced BST.
void MedianInsert(char **array, size_t left, size_t right, WordNode **root)
{

    if (left > right)
        return;

    size_t mid = left + (right - left) / 2;

    Insert(array[mid], root);

    if (mid > left)
        MedianInsert(array, left, mid - 1, root);

    MedianInsert(array, mid + 1, right, root);
}
