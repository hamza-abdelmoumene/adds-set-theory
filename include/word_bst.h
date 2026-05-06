#ifndef WORD_BST_H
#define WORD_BST_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/**
 * @file word_bst.h
 * @brief Binary Search Tree implementation for storing words.
 * Incorporates strict Abstract Machine naming conventions.
 */

#define MAX_WORD 100

/**
 * @struct WordNode
 * @brief Node for the binary search tree containing a single word.
 */
typedef struct WordNode
{
    char val[MAX_WORD];
    struct WordNode *left;
    struct WordNode *right;
} WordNode;

// -----------------------------------------------------------------------------
// Abstract Machine Operations for Binary Tree
// -----------------------------------------------------------------------------

#define Ass_LC(p, q) ((p)->left = (q))
#define Ass_RC(p, q) ((p)->right = (q))
#define FreeNode(p) free(p)
#define Ass_Node_Val(p, v) strcpy((p)->val, (v))
#define NodeValue(p) ((p)->val)
#define LC(p) ((p)->left)
#define RC(p) ((p)->right)

static inline WordNode *AllocateNodeImpl(void)
{
    WordNode *node = (WordNode *)malloc(sizeof(WordNode));
    if (!node)
    {
        fprintf(stderr, "AllocateNode: out of memory\n");
        exit(EXIT_FAILURE);
    }

    Ass_LC(node, NULL);
    Ass_RC(node, NULL);

    return node;
}

#define AllocateNode() (AllocateNodeImpl())


// Search for a word in the BST. Returns the node (p) and its parent (q).
void Search(const char *word, WordNode *r, WordNode **p, WordNode **q);

// Insert a word into the BST. Ignores duplicates.
bool Insert(const char *word, WordNode **r);

// Print the BST in-order (alphabetically).
void Inorder(WordNode *r);

// Free all nodes in the BST and set root to NULL.
void FreeTree(WordNode **r);

// Copy all words from src tree into dest tree.
void CopyTree(WordNode *src, WordNode **dest);

#endif // WORD_BST_H
