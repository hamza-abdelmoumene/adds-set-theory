/*
 * file_list.h - This is the top-level list. It stores all the files 
 * that the user has loaded into the program.
 */

#ifndef FILE_LIST_H
#define FILE_LIST_H

#include <stdlib.h>
#include <string.h>
#include "paragraph_ll.h"

#define MAX_FILENAME 256

/* Represents one loaded file */
typedef struct FileNode
{
    int          id;                    // File number (0, 1, 2...)
    char         filename[MAX_FILENAME]; // The path/name of the file
    ParagraphList val;                  // The actual content (list of paragraphs)
    struct FileNode *addr;              // Pointer to the next file in the list
} FileNode;

/* Header for the list of loaded files */
typedef struct FileList
{
    FileNode  *head;      // First file
    FileNode  *tail;      // Last file
    int        count;     // How many files are loaded
    FileNode **index;     // Shortcut array for fast access
    size_t     capacity; 
} FileList;

/* Creates an empty list of files */
FileList CreateFileList(void);

/* Tries to open a file, parse it, and add it to our list */
int AddFile(FileList *list, const char *filename);

/* Gets a file instantly using its ID */
FileNode *GetFileByIndex(FileList *list, int i);

/* Lists all loaded files on the screen */
void PrintFileList(FileList list);

/* Cleans up all files and all their content from memory */
void FreeFileList(FileList *list);

#endif 
