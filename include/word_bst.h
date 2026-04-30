// Include guards
#ifndef WORD_BST_H
#define WORD_BST_H

// Struct and function declarations for the word binary search tree

struct WordNode {
    char *word;
    struct WordNode *left;
    struct WordNode *right;
};

struct WordBST {
    struct WordNode *root;
};

struct WordBST *createWordBST();
void insertWord(struct WordBST *bst, char *word);

#endif // WORD_BST_H
