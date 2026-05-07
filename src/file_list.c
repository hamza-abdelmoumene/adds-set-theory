#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/file_list.h"
#include "../include/file_parser.h"

// Initialize an empty FileList.
FileList CreateFileList(void)
{
    FileList list = {NULL, NULL, 0, NULL, 0};
    return list;
}

// Parse a file and append it as a new FileNode. Grows the index automatically.
void AddFile(FileList *list, const char *filename)
{
    if (list == NULL)
    {
        fprintf(stderr, "AddFile: list must not be NULL\n");
        return;
    }
    if (filename == NULL || filename[0] == '\0')
    {
        fprintf(stderr, "AddFile: filename must not be NULL or empty\n");
        return;
    }

    // allocate and populate the new node
    FileNode *new_node;
    Allocate(new_node);
    new_node->id = list->count;
    strncpy(new_node->filename, filename, MAX_FILENAME - 1);
    new_node->filename[MAX_FILENAME - 1] = '\0';
    new_node->val  = ParseFile(filename);
    Ass_adr(new_node, NULL);

    // append to the linked list via the tail pointer — O(1)
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

    // grow the index array to keep it always up to date
    FileNode **tmp = (FileNode **)realloc(list->index, list->count * sizeof(FileNode *));
    if (tmp == NULL)
    {
        fprintf(stderr, "AddFile: out of memory growing index\n");
        exit(EXIT_FAILURE);
    }
    list->index                    = tmp;
    list->capacity                 = list->count;
    list->index[new_node->id]      = new_node;
}

// Retrieve a file node in O(1) using the index array.
FileNode *GetFileByIndex(FileList *list, int i)
{
    if (list == NULL || i < 0 || i >= list->count)
        return NULL;

    return list->index[i];
}

// Print the list of loaded files with their ids and filenames.
void PrintFileList(FileList list)
{
    FileNode *current = list.head;
    while (current != NULL)
    {
        printf("  [%d] %s  (%d paragraph%s)\n",
               current->id,
               current->filename,
               current->val.count,
               current->val.count == 1 ? "" : "s");
        current = Next(current);
    }
}

// Free all file nodes, their paragraph lists, and the index array.
void FreeFileList(FileList *list)
{
    if (list == NULL)
        return;

    FileNode *current = list->head;

    while (current != NULL)
    {
        FileNode *temp = Next(current);
        FreeParagraphList(&current->val);
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