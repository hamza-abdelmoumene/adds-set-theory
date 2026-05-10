#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/file_list.h"
#include "../include/file_parser.h"
#include "../include/utils.h"

// Starts a fresh, empty list of files.
FileList CreateFileList(void)
{
    FileList list = {NULL, NULL, 0, NULL, 0};
    return list;
}

// Loads a file into the program.
// 1. Checks if the file is readable.
// 2. Calls the parser to break it into paragraphs.
// 3. Adds it to the linked list and the shortcut array.
int AddFile(FileList *list, const char *filename)
{
    if (list == NULL || filename == NULL || filename[0] == '\0')
        return 0;

    // Make sure the file actually exists and we can open it
    if (!IsReadableRegularFile(filename))
        return 0;

    // Create a new file node
    FileNode *new_node;
    Allocate(new_node);
    new_node->id = list->count;
    strncpy(new_node->filename, filename, MAX_FILENAME - 1);
    new_node->filename[MAX_FILENAME - 1] = '\0';
    
    // THE BIG STEP: Parse the file content
    new_node->val = ParseFile(filename);
    Ass_adr(new_node, NULL);

    // Add to the linked list (at the tail)
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

    // Update the shortcut array so we can find this file by its ID later
    list->index = (FileNode **)CheckedRealloc(list->index, list->count * sizeof(FileNode *), "AddFile");
    list->capacity = list->count;
    list->index[new_node->id] = new_node;

    return 1;
}

// Uses the shortcut array to get a file instantly.
FileNode *GetFileByIndex(FileList *list, int i)
{
    if (list == NULL || i < 0 || i >= list->count)
        return NULL;

    return list->index[i];
}

// Shows all loaded files and how many paragraphs they have.
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

// Wipes everything from memory.
void FreeFileList(FileList *list)
{
    if (list == NULL)
        return;

    FileNode *current = list->head;

    while (current != NULL)
    {
        FileNode *temp = Next(current);
        FreeParagraphList(&current->val); // Cascade down to paragraphs
        Free(current);
        current = temp;
    }

    free(list->index);

    list->head = NULL;
    list->tail = NULL;
    list->index = NULL;
    list->count = 0;
}
