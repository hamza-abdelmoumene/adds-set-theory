#ifndef WORD_BST_H
#define WORD_BST_H

/**
 * @file word_bst.h
 * @brief Binary Search Tree implementation for storing words.
 */

#define MAX_WORD 100

/**
 * @struct WordNode
 * @brief Node for the binary search tree containing a single word.
 */
typedef struct WordNode {
    char word[MAX_WORD];
    struct WordNode *lc; // Left child
    struct WordNode *rc; // Right child
} WordNode;

// Search for a word in the BST. Returns the node (p) and its parent (q).
void Search(const char *word, WordNode *r, WordNode **p, WordNode **q);

// Insert a word into the BST. Ignores duplicates.
void Insert(const char *word, WordNode **r);

// Print the BST in-order (alphabetically).
void Inorder(WordNode *r);

// Free all nodes in the BST and set root to NULL.
void FreeTree(WordNode **r);

// Copy all words from src tree into dest tree.
void CopyTree(WordNode *src, WordNode **dest);

#endif // WORD_BST_H
