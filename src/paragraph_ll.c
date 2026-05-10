#include <stdio.h>
#include <stdlib.h>
#include "../include/paragraph_ll.h"

// Sets up a new, empty list for paragraphs.
ParagraphList CreateParagraphList(void)
{
    ParagraphList list = {NULL, NULL, 0, NULL, 0};
    return list;
}

// Adds a paragraph to the end of the list.
void AddParagraph(ParagraphList *list, SentenceList sentence_list, const char *original)
{
    if (list == NULL) return;

    ParagraphNode *new_node;
    Allocate(new_node); // Allocate using our macro
    new_node->id = list->count;
    new_node->original = CheckedStrDup(original, "AddParagraph");
    Ass_val(new_node, sentence_list);
    Ass_adr(new_node, NULL);

    // standard tail insertion
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

// Walks through the list to find a specific paragraph by its ID.
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

// Prints everything! This calls PrintSentences, which calls Inorder.
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

// Deep cleanup. This is like a waterfall: 
// Free Paragraph -> Free Sentences -> Free Word BSTs.
void FreeParagraphList(ParagraphList *list)
{
    if (list == NULL)
        return;

    ParagraphNode *current = list->head;

    while (current != NULL)
    {
        ParagraphNode *temp = Next(current);
        FreeSentenceList(&current->val); // Cascade down to sentences
        if (current->original)
            free(current->original);
        Free(current);
        current = temp;
    }

    free(list->index); // Free the shortcut array

    list->head = NULL;
    list->tail = NULL;
    list->index = NULL;
    list->count = 0;
}

// Builds the "shortcut" array so we can access any paragraph by index (O(1)).
void BuildIndex(ParagraphList *list)
{
    if (list == NULL || list->count == 0) return;

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

// Uses the shortcut array to get a paragraph instantly.
ParagraphNode *GetParagraphByIndex(ParagraphList *list, int i)
{
    if (list == NULL || i < 0 || i >= list->count)
        return NULL;

    return list->index[i];
}
