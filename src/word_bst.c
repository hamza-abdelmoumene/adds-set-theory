#include <stdio.h>
#include "bst_model.h"


void Search(const char *word, WordNode *r, WordNode **p, WordNode **q) {
    if (r == NULL) {
        *p = NULL;
        *q = NULL;
    } else if (strcmp(Word(r), word) == 0) {
        *p = r;
        *q = NULL;
    } else {
        if (strcmp(word, Word(r)) < 0)
            Search(word, LC(r), p, q);
        else
            Search(word, RC(r), p, q);
        if (*q == NULL)
            *q = r;
    }
}


void Insert(const char *word, WordNode **r) {
    /* TODO */
}


void Inorder(WordNode *r) {
    if (r == NULL)
        return;

    Inorder(LC(r));
    printf("%s ", Word(r));
    Inorder(RC(r));
}


void FreeTree(WordNode **r) {
    /* TODO */
}


void CopyTree(WordNode *src, WordNode **dest) {
    /* TODO */
}
