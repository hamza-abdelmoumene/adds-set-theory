#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/word_bst.h"

// Search for a word in the BST using Abstract Machine. Returns the node (p) and its parent (q).
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

    // algorithm logic (we can use either recursive or iterative aproach)
    while (*p != NULL)
    {
        int cmp = strcmp(word, NodeValue(*p));
        if (cmp == 0)
        {
            return;
        }

        *q = *p;
        if (cmp < 0)
        {
            *p = LC(*p);
        }
        else
        {
            *p = RC(*p);
        }
    }
}

// Insert a word into the BST. Ignores duplicates.
bool Insert(const char *word, WordNode **r)
{
    // handle edge cases
    if (r == NULL)
    {
        return false;
    }

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
    {
        return false;
    }

    if (q == NULL)
    {
        *r = AllocateNode();
        Ass_Node_Val(*r, word);
        return true;
    }

    if (strcmp(word, NodeValue(q)) < 0)
    {
        Ass_LC(q, AllocateNode());
        Ass_Node_Val(LC(q), word);
    }
    else
    {
        Ass_RC(q, AllocateNode());
        Ass_Node_Val(RC(q), word);
    }

    return true;
}

// Print the BST in-order (alphabetically).
void Inorder(WordNode *r)
{
    // handle edge cases
    if (r == NULL)
    {
        return;
    }

    if (NodeValue(r)[0] == '\0')
    {
        fprintf(stderr, "Inorder: encountered empty word node\n");
    }

    if (strlen(NodeValue(r)) >= MAX_WORD)
    {
        fprintf(stderr, "Inorder: encountered oversized word node\n");
    }

    // algorithm logic
    if (r != NULL)
    {
        Inorder(LC(r));
        printf("%s", NodeValue(r));
        Inorder(RC(r));
    }
}

// Free all nodes in the BST and set root to NULL.
void FreeTree(WordNode **r)
{
    // handle edge cases
    if (r == NULL || *r == NULL)
    {
        if (r == NULL)
        {
            fprintf(stderr, "FreeTree: root pointer must not be NULL\n");
        }
        return;
    }

    // algorithm logic
    FreeTree(&((*r)->left));
    FreeTree(&((*r)->right));
    FreeNode(*r);
    *r = NULL;
}

// Copy all words from src tree into dest tree.
void CopyTree(WordNode *src, WordNode **dest)
{
    // handle edge cases
    if (src == NULL || dest == NULL)
    {
        if (dest == NULL)
        {
            fprintf(stderr, "CopyTree: destination pointer must not be NULL\n");
        }
        return;
    }

    if (NodeValue(src)[0] == '\0')
    {
        fprintf(stderr, "CopyTree: encountered empty word node\n");
    }

    if (strlen(NodeValue(src)) >= MAX_WORD)
    {
        fprintf(stderr, "CopyTree: encountered oversized word node\n");
    }

    // algorithm logic
    Insert(NodeValue(src), dest);
    CopyTree(LC(src), dest);
    CopyTree(RC(src), dest);
}

// Balanced BST helpers (array -> sort -> median-first insertion)
void CollectWords(WordNode *root, char ***array, size_t *size, size_t *capacity)
{
    if (array == NULL || size == NULL || capacity == NULL)
    {
        fprintf(stderr, "CollectWords: output parameters must not be NULL\n");
        return;
    }

    if (root == NULL)
    {
        return;
    }

    CollectWords(LC(root), array, size, capacity);

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

    if (*size >= *capacity)
    {
        size_t new_capacity = (*capacity) * 2;
        char **tmp = (char **)realloc(*array, new_capacity * sizeof(char *));
        if (tmp == NULL)
        {
            fprintf(stderr, "CollectWords: out of memory\n");
            exit(EXIT_FAILURE);
        }
        *array = tmp;
        *capacity = new_capacity;
    }

    size_t len = strlen(NodeValue(root));
    char *copy = (char *)malloc(len + 1);
    if (copy == NULL)
    {
        fprintf(stderr, "CollectWords: out of memory\n");
        exit(EXIT_FAILURE);
    }
    strcpy(copy, NodeValue(root));

    (*array)[*size] = copy;
    (*size)++;

    CollectWords(RC(root), array, size, capacity);
}

// helper function to use in SortWords function (used in qsort function, for more infos visit: https://www.geeksforgeeks.org/c/qsort-function-in-c/)
static int comp(const void *a, const void *b) {
    return (*(char *)a - *(char *)b);
}

void SortWords(char **array, size_t size)
{
    // directly use qsort built-in function
    qsort(*array, size, sizeof((*array)[0]), comp);
}

void MedianInsert(char **array, size_t left, size_t right, WordNode **root)
{
    if(left > right)
        return;
    
    int mid = left + (right - left) / 2; // use this math-correct formula to avoid overflow in large numbers.

    Insert(root, array[mid]);

    // logic - insert the median of the array at the bst recursively to ensure a balanced bst
    MedianInsert(array, left, mid - 1, root);
    MedianInsert(array, mid + 1, right, root);
}

