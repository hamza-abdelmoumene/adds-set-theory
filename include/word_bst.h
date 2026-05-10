#ifndef WORD_BST_H
#define WORD_BST_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "utils.h"

#define MAX_WORD 256

typedef struct WordNode
{
    char val[MAX_WORD];
    struct WordNode *left;
    struct WordNode *right;
} WordNode;

#define Ass_LC(p, q) ((p)->left = (q))
#define Ass_RC(p, q) ((p)->right = (q))
#define FreeNode(p) free(p)
#define Ass_Node_Val(p, v) strcpy((p)->val, (v))
#define NodeValue(p) ((p)->val)
#define LC(p) ((p)->left)
#define RC(p) ((p)->right)

static inline WordNode *AllocateNodeImpl(void)
{
    WordNode *node = (WordNode *)CheckedMalloc(sizeof(WordNode), "AllocateNode");

    Ass_LC(node, NULL);
    Ass_RC(node, NULL);

    return node;
}

#define AllocateNode() (AllocateNodeImpl())


void Search(const char *word, WordNode *r, WordNode **p, WordNode **q);


bool Insert(const char *word, WordNode **r);


void Inorder(WordNode *r);


void FreeTree(WordNode **r);


void CopyTree(WordNode *src, WordNode **dest);


void CollectWords(WordNode *root, char ***array, size_t *size, size_t *capacity);
void SortWords(char **array, size_t size);
void MedianInsert(char **array, size_t left, size_t right, WordNode **root);

#endif 
