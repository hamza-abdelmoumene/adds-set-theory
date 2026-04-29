#ifndef BST_MODEL_H
#define BST_MODEL_H

#include <stdlib.h>
#include <string.h>

#define MAX_WORD 100

typedef struct WordNode {
    char word[MAX_WORD];
    struct WordNode *lc;
    struct WordNode *rc;
} WordNode;

/* Abstract Machine */
#define AllocateNode()      ((WordNode *)malloc(sizeof(WordNode)))
#define FreeNode(p)         free((p))
#define Ass_Word(p, w)      strcpy((p)->word, (w))
#define Ass_lc(p, q)        ((p)->lc = (q))
#define Ass_rc(p, q)        ((p)->rc = (q))
#define Word(p)             ((p)->word)
#define LC(p)               ((p)->lc)
#define RC(p)               ((p)->rc)

void Search  (const char *word, WordNode *r, WordNode **p, WordNode **q);
void Insert  (const char *word, WordNode **r);
void Inorder (WordNode *r);
void FreeTree(WordNode **r);
void CopyTree(WordNode *src, WordNode **dest);

#endif