#include <stdio.h>
#include <stdlib.h>
#include "../include/paragraph_ll.h"

// Initialize an empty ParagraphList.
ParagraphList CreateParagraphList(void)
{
    ParagraphList list = {NULL, NULL, 0, NULL, 0};
    return list;
}

// Append a new paragraph (represented by its sentence list) to the list.
void AddParagraph(ParagraphList *list, SentenceList sentence_list)
{
    ParagraphNode *new_node;
    Allocate(new_node);
    new_node->id = list->count;
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

// Retrieve a paragraph node by its ID (linear scan fallback).
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

// Print all paragraphs, visualizing their sentences and words.
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

// Free all paragraphs, sentences, and BSTs from memory.
void FreeParagraphList(ParagraphList *list)
{
    ParagraphNode *current = list->head;

    while (current != NULL)
    {
        ParagraphNode *temp = Next(current);
        FreeSentenceList(&current->val);
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

// Build the index array after parsing for O(1) access by index.
void BuildIndex(ParagraphList *list)
{
    if (list->count == 0)
    {
        list->index    = NULL;
        list->capacity = 0;
        return;
    }

    list->index = (ParagraphNode **)malloc(list->count * sizeof(ParagraphNode *));
    if (list->index == NULL)
    {
        fprintf(stderr, "BuildIndex: out of memory\n");
        exit(EXIT_FAILURE);
    }

    list->capacity = list->count;

    ParagraphNode *current = list->head;
    while (current != NULL)
    {
        list->index[current->id] = current;
        current = Next(current);
    }
}

// Retrieve a paragraph node in O(1) using the index array.
ParagraphNode *GetParagraphByIndex(ParagraphList *list, int i)
{
    if (i < 0 || i >= list->count)
        return NULL;

    return list->index[i];
}