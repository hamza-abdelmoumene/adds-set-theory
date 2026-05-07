#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/word_bst.h"

// Search for a word in the BST. Returns the node (p) and its parent (q).
void Search(const char *word, WordNode *r, WordNode **p, WordNode **q)
{
    // handle edge cases
    if (p == NULL || q == NULL)
    {
        fprintf(stderr, "Search: output pointers must not be NULL\n");
        return;
    }

    if (word == NULL)
    {
        fprintf(stderr, "Search: word must not be NULL\n");
        *p = NULL;
        *q = NULL;
        return;
    }

    if (word[0] == '\0')
    {
        fprintf(stderr, "Search: word must not be empty\n");
        *p = NULL;
        *q = NULL;
        return;
    }

    if (strlen(word) >= MAX_WORD)
    {
        fprintf(stderr, "Search: word exceeds MAX_WORD (%d)\n", MAX_WORD);
        *p = NULL;
        *q = NULL;
        return;
    }

    *p = r;
    *q = NULL;

    // algorithm logic — iterative walk, track parent at each step
    while (*p != NULL)
    {
        int cmp = strcmp(word, NodeValue(*p));
        if (cmp == 0)
            return; // found — p points to the node, q to its parent

        *q = *p;
        *p = (cmp < 0) ? LC(*p) : RC(*p);
    }
    // not found — p is NULL, q is the last visited node (future parent)
}

// Insert a word into the BST. Ignores duplicates.
bool Insert(const char *word, WordNode **r)
{
    // handle edge cases
    if (r == NULL)
        return false;

    if (word == NULL)
    {
        fprintf(stderr, "Insert: word must not be NULL\n");
        return false;
    }

    if (word[0] == '\0')
    {
        fprintf(stderr, "Insert: word must not be empty\n");
        return false;
    }

    if (strlen(word) >= MAX_WORD)
    {
        fprintf(stderr, "Insert: word exceeds MAX_WORD (%d)\n", MAX_WORD);
        return false;
    }

    // algorithm logic
    WordNode *p = NULL;
    WordNode *q = NULL;

    Search(word, *r, &p, &q);
    if (p != NULL)
        return false; // duplicate — do nothing

    // allocate and fill the new node
    WordNode *new_node = AllocateNode();
    Ass_Node_Val(new_node, word);

    if (q == NULL)
    {
        *r = new_node; // tree was empty
        return true;
    }

    // attach to parent on the correct side
    if (strcmp(word, NodeValue(q)) < 0)
        Ass_LC(q, new_node);
    else
        Ass_RC(q, new_node);

    return true;
}

// Print the BST in-order (alphabetically).
void Inorder(WordNode *r)
{
    if (r == NULL)
        return;

    // algorithm logic — Left → Root → Right
    Inorder(LC(r));
    printf("%s ", NodeValue(r));
    Inorder(RC(r));
}

// Free all nodes in the BST and set root to NULL.
void FreeTree(WordNode **r)
{
    // handle edge cases
    if (r == NULL || *r == NULL)
    {
        if (r == NULL)
            fprintf(stderr, "FreeTree: root pointer must not be NULL\n");
        return;
    }

    // algorithm logic — post-order: free children before self
    FreeTree(&((*r)->left));
    FreeTree(&((*r)->right));
    FreeNode(*r);
    *r = NULL;
}

// Copy all words from src tree into dest tree.
void CopyTree(WordNode *src, WordNode **dest)
{
    // handle edge cases
    if (src == NULL)
        return;

    if (dest == NULL)
    {
        fprintf(stderr, "CopyTree: destination pointer must not be NULL\n");
        return;
    }

    // algorithm logic — pre-order traversal of src, insert each word into dest
    Insert(NodeValue(src), dest);
    CopyTree(LC(src), dest);
    CopyTree(RC(src), dest);
}

// Collect all words from the BST into a dynamically growing array (in-order).
void CollectWords(WordNode *root, char ***array, size_t *size, size_t *capacity)
{
    if (array == NULL || size == NULL || capacity == NULL)
    {
        fprintf(stderr, "CollectWords: output parameters must not be NULL\n");
        return;
    }

    if (root == NULL)
        return;

    // traverse left first (in-order)
    CollectWords(LC(root), array, size, capacity);

    // grow the array if needed
    if (*capacity == 0)
    {
        *capacity = 8;
        *array = (char **)malloc((*capacity) * sizeof(char *));
        if (*array == NULL)
        {
            fprintf(stderr, "CollectWords: out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (*size >= *capacity)
    {
        size_t new_cap = (*capacity) * 2;
        char **tmp = (char **)realloc(*array, new_cap * sizeof(char *));
        if (tmp == NULL)
        {
            fprintf(stderr, "CollectWords: out of memory\n");
            exit(EXIT_FAILURE);
        }
        *array = tmp;
        *capacity = new_cap;
    }

    // copy the word string into the array
    char *copy = (char *)malloc(strlen(NodeValue(root)) + 1);
    if (copy == NULL)
    {
        fprintf(stderr, "CollectWords: out of memory\n");
        exit(EXIT_FAILURE);
    }
    strcpy(copy, NodeValue(root));
    (*array)[(*size)++] = copy;

    CollectWords(RC(root), array, size, capacity);
}

// Comparator for qsort: compares two char* strings.
static int comp(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

// Sort a char** array alphabetically using qsort.
void SortWords(char **array, size_t size)
{
    if (array == NULL || size == 0)
        return;

    qsort(array, size, sizeof(char *), comp);
}

// Insert words median-first into a BST to guarantee a balanced tree.
void MedianInsert(char **array, size_t left, size_t right, WordNode **root)
{
    // Guard: stop when the range is empty.
    if (left > right)
        return;

    size_t mid = left + (right - left) / 2;

    Insert(array[mid], root);

    // recurse on left half — guard against underflow before calling
    if (mid > left)
        MedianInsert(array, left, mid - 1, root);

    MedianInsert(array, mid + 1, right, root);
}