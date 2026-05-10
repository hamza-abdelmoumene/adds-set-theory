#include <stdio.h>
#include <stdlib.h>
#include "../include/sentence_ll.h"


SentenceList CreateSentenceList(void)
{
    SentenceList list = {NULL, NULL, 0, NULL, 0};
    return list;
}


void AddSentence(SentenceList *list, WordNode *bst_root, const char *original)
{
    if (list == NULL)
    {
        PrintError("AddSentence", "list must not be NULL");
        return;
    }

    SentenceNode *new_node;
    Allocate(new_node);
    Ass_val(new_node, bst_root);
    Ass_adr(new_node, NULL);
    new_node->id = list->count;
    if (original)
        new_node->original = CheckedStrDup(original, "AddSentence");
    else
        new_node->original = NULL;

    if (list->head == NULL)
    {
        list->head = new_node;
        list->tail = new_node;
    }
    else
    {
        Ass_adr(list->tail, new_node);
        list->tail = new_node;
    }

    list->count++;
}


SentenceNode *GetSentence(SentenceList list, int id)
{
    if (id < 0 || id >= list.count)
        return NULL;

    SentenceNode *current = list.head;
    while (current != NULL)
    {
        if (current->id == id)
            return current;
        current = Next(current);
    }

    return NULL;
}


void BuildSentenceIndex(SentenceList *list)
{
    if (list == NULL)
    {
        PrintError("BuildSentenceIndex", "list must not be NULL");
        return;
    }

    
    if (list->count == 0)
    {
        list->index = NULL;
        list->capacity = 0;
        return;
    }

    list->index = (SentenceNode **)CheckedMalloc(list->count * sizeof(SentenceNode *),
                                                 "BuildSentenceIndex");

    list->capacity = list->count;

    SentenceNode *current = list->head;
    while (current != NULL)
    {
        list->index[current->id] = current;
        current = Next(current);
    }
}


SentenceNode *GetSentenceByIndex(SentenceList *list, int i)
{
    if (list == NULL || i < 0 || i >= list->count)
        return NULL;

    return list->index[i];
}


void PrintSentences(SentenceList list)
{
    SentenceNode *current = list.head;
    while (current != NULL)
    {
        printf("  Sentence %d: ", current->id);
        Inorder(current->val);
        printf("\n");
        current = Next(current);
    }
}


void FreeSentenceList(SentenceList *list)
{
    if (list == NULL)
        return;

    SentenceNode *current = list->head;

    while (current != NULL)
    {
        SentenceNode *temp = Next(current);
        FreeTree(&current->val);
        free(current->original);
        Free(current);
        current = temp;
    }

    
    free(list->index);

    list->head = NULL;
    list->tail = NULL;
    list->index = NULL;
    list->count = 0;
    list->capacity = 0;
}
