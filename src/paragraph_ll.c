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
    // logic - append a paragraph simply by iterating to the tail of the list
    ParagraphNode *new_node;
    Allocate(new_node);
    new_node->id = list->count;
    Ass_val(new_node, sentence_list);
    Ass_adr(new_node, NULL);

    if (list->head == NULL)
    {
        list->head = new_node;
        list->tail = new_node;
        return;
    }

    Ass_adr(list->tail, new_node);
    list->tail = new_node;
    list->count++;
}


ParagraphNode *GetParagraph(ParagraphList list, int id)
{
    // logic - same as Get Sentence function, bounds check and iterate until find a match.
    if(id < 0 || id >= list.count)
        return NULL;

    ParagraphNode *current = list.head;
    while(current != NULL)
    {
        if(current->id == id)
            return current;
        current = Next(current);
    }

    return NULL;
}

// Print all paragraphs, visualizing their sentences and words.
void PrintParagraphs(ParagraphList list)
{
    // logic - iterate over the paragraph nodes and inside the loop call the sentence printer function.
    ParagraphNode *current = list.head;
    while(current != NULL)
    {
        printf("――― Paragraph %d ―――", current->id);
        PrintSentences(current->val);
        printf("\n");
        current = Next(current);
    }
}

// Free all paragraphs, sentences, and BSTs from memory.
void FreeParagraphList(ParagraphList *list)
{
    ParagraphNode *current = list->head;

    while(current != NULL)
    {
        ParagraphNode *temp = Next(current);
        FreeSentenceList(&current->val);
        Free(current);
        current = temp;
    }

    free(list->index);
    list->index = NULL;
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    list->capacity = 0;
}

// Build the index array after parsing for O(1) access by index.
void BuildIndex(ParagraphList *list)
{
    list->index = malloc((list->count) * sizeof(ParagraphNode* ));
    if(list->index == NULL){
        // TODO: Handle malloc fail
    }
    list->capacity = list->count;
    ParagraphNode *current = list->head;
    while(current != NULL)
    {
        list->index[current->id] = current;
        current = Next(current);
    }
}

// Retrieve a paragraph node by index from the built array.
ParagraphNode *GetParagraphByIndex(ParagraphList *list, int i)
{
    if(i < 0 || i >= list->count) return NULL;
    return list->index[i];
    return NULL;
}
