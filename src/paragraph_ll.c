#include <stdio.h>
#include <stdlib.h>
#include "../include/paragraph_ll.h"


ParagraphList CreateParagraphList(void)
{
    ParagraphList list = {NULL, NULL, 0, NULL, 0};
    return list;
}


void AddParagraph(ParagraphList *list, SentenceList sentence_list, const char *original)
{
    if (list == NULL)
    {
        PrintError("AddParagraph", "list must not be NULL");
        return;
    }

    ParagraphNode *new_node;
    Allocate(new_node);
    new_node->id = list->count;
    new_node->original = CheckedStrDup(original, "AddParagraph");
    Ass_val(new_node, sentence_list);
    Ass_adr(new_node, NULL);

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


ParagraphNode *GetParagraph(ParagraphList list, int id)
{
    if (id < 0 || id >= list.count)
        return NULL;

    ParagraphNode *current = list.head;
    while (current != NULL)
    {
        if (current->id == id)
            return current;
        current = Next(current);
    }

    return NULL;
}


void PrintParagraphs(ParagraphList list)
{
    ParagraphNode *current = list.head;
    while (current != NULL)
    {
        printf("――― Paragraph %d ―――\n", current->id);
        PrintSentences(current->val);
        printf("\n");
        current = Next(current);
    }
}


void FreeParagraphList(ParagraphList *list)
{
    if (list == NULL)
        return;

    ParagraphNode *current = list->head;

    while (current != NULL)
    {
        ParagraphNode *temp = Next(current);
        FreeSentenceList(&current->val);
        if (current->original)
            free(current->original);
        Free(current);
        current = temp;
    }

    free(list->index);

    list->head     = NULL;
    list->tail     = NULL;
    list->index    = NULL;
    list->count    = 0;
    list->capacity = 0;
}


void BuildIndex(ParagraphList *list)
{
    if (list == NULL)
    {
        PrintError("BuildIndex", "list must not be NULL");
        return;
    }

    if (list->count == 0)
    {
        list->index    = NULL;
        list->capacity = 0;
        return;
    }

    list->index = (ParagraphNode **)CheckedMalloc(list->count * sizeof(ParagraphNode *),
                                                  "BuildIndex");

    list->capacity = list->count;

    ParagraphNode *current = list->head;
    while (current != NULL)
    {
        list->index[current->id] = current;
        current = Next(current);
    }
}


ParagraphNode *GetParagraphByIndex(ParagraphList *list, int i)
{
    if (list == NULL || i < 0 || i >= list->count)
        return NULL;

    return list->index[i];
}
