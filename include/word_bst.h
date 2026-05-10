/*
 * word_bst.h - This is where we handle the words in a sentence.
 * We use a Binary Search Tree (BST) because it automatically keeps words 
 * unique and sorted, which is perfect for set theory!
 */

#ifndef WORD_BST_H
#define WORD_BST_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "utils.h"

#define MAX_WORD 256

/* Each node in our tree holds one word and pointers to its "children" */
typedef struct WordNode
{
    char val[MAX_WORD];       // The word itself
    struct WordNode *left;    // Smaller words go left
    struct WordNode *right;   // Bigger words go right
} WordNode;

/* 
 * These are "Abstract Machine" macros. They make the code look 
 * like the formal logic we learned in class.
 */
#define Ass_LC(p, q) ((p)->left = (q))    // Assign Left Child
#define Ass_RC(p, q) ((p)->right = (q))   // Assign Right Child
#define FreeNode(p) free(p)               // Clean up a node
#define Ass_Node_Val(p, v) strcpy((p)->val, (v)) // Put a word in a node
#define NodeValue(p) ((p)->val)           // Get the word out of a node
#define LC(p) ((p)->left)                 // Look at the left child
#define RC(p) ((p)->right)                // Look at the right child

/* Creates a brand new, empty tree node */
static inline WordNode *AllocateNodeImpl(void)
{
    WordNode *node = (WordNode *)CheckedMalloc(sizeof(WordNode), "AllocateNode");

    Ass_LC(node, NULL);
    Ass_RC(node, NULL);

    return node;
}

#define AllocateNode() (AllocateNodeImpl())

/* Finds where a word is in the tree, and who its parent is */
void Search(const char *word, WordNode *r, WordNode **p, WordNode **q);

/* Adds a new word to the tree. If it's already there, it does nothing. */
bool Insert(const char *word, WordNode **r);

/* Prints all words in the tree from A to Z */
void Inorder(WordNode *r);

/* Wipes out the whole tree and frees the memory */
void FreeTree(WordNode **r);

/* Copies everything from one tree into another */
void CopyTree(WordNode *src, WordNode **dest);

/* 
 * These helpers are for balancing the tree. We put everything in an 
 * array, sort it, and then pick the middle word first.
 */
void CollectWords(WordNode *root, char ***array, size_t *size, size_t *capacity);
void SortWords(char **array, size_t size);
void MedianInsert(char **array, size_t left, size_t right, WordNode **root);

#endif 
