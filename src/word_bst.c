#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/word_bst.h"

// Search for a word in the BST using Abstract Machine. Returns the node (p) and its parent (q).
void Search(const char *word, WordNode *r, WordNode **p, WordNode **q)
{
    // edge cases traiting
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
    // edge cases traiting
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
    if (*r == NULL)
    {
        *r = AllocateNode();
        Ass_Node_Val(*r, word);
        return true;
    }

    if (strcmp(word, NodeValue(*r)) == 0)
        return false;

    if (strcmp(word, NodeValue(*r)) < 0)
        return Insert(word, &((*r)->left));
    else
        return Insert(word, &((*r)->right));
}

// Print the BST in-order (alphabetically).
void Inorder(WordNode *r)
{
    // edge cases traiting
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
    // edge cases traiting
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
    // edge cases traiting
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
