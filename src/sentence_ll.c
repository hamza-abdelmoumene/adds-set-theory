#include <stdio.h>
#include <stdlib.h>
#include "../include/sentence_ll.h"

// Initialize an empty SentenceList.
SentenceList CreateSentenceList(void)
{
    SentenceList list = {NULL, NULL, 0};

    return list;
}

// Append a new sentence (represented by its word BST) to the list using abstract machine operations.
void AddSentence(SentenceList *list, WordNode *bst_root)
{
    SentenceNode *new_node;
    Allocate(new_node);
    Ass_val(new_node, bst_root);
    Ass_adr(new_node, NULL);
    new_node->id = list->count;

    if(list->head == NULL)
    {
        list->head = new_node;
        list->tail = new_node;
        return; 
    }
    

    
    Ass_adr(list->tail, new_node);
    list->tail = new_node;
    list->count++;

}

// Retrieve a sentence node by its ID.
SentenceNode *GetSentence(SentenceList list, int id)
{
    if(id >= list.count || id < 0)
        return NULL;

    SentenceNode *current = list.head;
    while(current != NULL)
    {
        if(current->id == id)
            return current;
        current = Next(current);
    }

    return NULL;
}

// Print all sentences in the list and their distinct words.
void PrintSentences(SentenceList list)
{
    SentenceNode *current = list.head;
    while(current != NULL)
    {
        printf("Sentence %d: ", current->id);
        Inorder(current->val);
        printf("\n");
        current = Next(current);
    }
    return;
}

// Free all sentences and their associated BSTs from memory.
void FreeSentenceList(SentenceList *list)
{
    SentenceNode *current = list->head;

    while(current != NULL)
    {
        SentenceNode *temp = Next(current);
        FreeTree(&current->val);
        Free(current);
        current = temp;
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}
