#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/file_list.h"
#include "../include/file_parser.h"
#include "../include/utils.h"

// Initialize an empty FileList.
FileList CreateFileList(void)
{
    FileList list = {NULL, NULL, 0, NULL, 0};
    return list;
}

// Parse a readable file and append it as a new FileNode. Grows the index automatically.
int AddFile(FileList *list, const char *filename)
{
    if (list == NULL)
    {
        PrintError("AddFile", "list must not be NULL");
        return 0;
    }
    if (filename == NULL || filename[0] == '\0')
    {
        PrintError("AddFile", "filename must not be NULL or empty");
        return 0;
    }

    FILE *check = fopen(filename, "r");
    if (check == NULL)
        return 0;
    fclose(check);

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
    list->index                    = (FileNode **)CheckedRealloc(list->index, list->count * sizeof(FileNode *), "AddFile");
    list->capacity                 = list->count;
    list->index[new_node->id]      = new_node;

    return 1;
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
