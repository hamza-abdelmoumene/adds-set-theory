#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/file_list.h"
#include "../include/file_parser.h"
#include "../include/utils.h"


FileList CreateFileList(void)
{
    FileList list = {NULL, NULL, 0, NULL, 0};
    return list;
}


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

    if (!IsReadableRegularFile(filename))
        return 0;

    
    FileNode *new_node;
    Allocate(new_node);
    new_node->id = list->count;
    strncpy(new_node->filename, filename, MAX_FILENAME - 1);
    new_node->filename[MAX_FILENAME - 1] = '\0';
    new_node->val  = ParseFile(filename);
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

    
    list->index                    = (FileNode **)CheckedRealloc(list->index, list->count * sizeof(FileNode *), "AddFile");
    list->capacity                 = list->count;
    list->index[new_node->id]      = new_node;

    return 1;
}


FileNode *GetFileByIndex(FileList *list, int i)
{
    if (list == NULL || i < 0 || i >= list->count)
        return NULL;

    return list->index[i];
}


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
