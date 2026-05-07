#ifndef FILE_LIST_H
#define FILE_LIST_H

#include <stdlib.h>
#include <string.h>
#include "paragraph_ll.h"

/**
 * @file file_list.h
 * @brief Linked list of loaded files. One extra layer above ParagraphList.
 *        Mirrors the exact same Abstract Machine pattern used for sentences
 *        and paragraphs — same macros, same tail-pointer O(1) append,
 *        same parallel index array for O(1) access by id.
 */

#define MAX_FILENAME 256

/**
 * @struct FileNode
 * @brief Represents one loaded file: its name and its parsed paragraph structure.
 */
typedef struct FileNode
{
    int          id;                    // Position in the file list
    char         filename[MAX_FILENAME]; // Original filename (display / reload)
    ParagraphList val;                  // Parsed content of this file
    struct FileNode *addr;              // Pointer to the next file node
} FileNode;

/**
 * @struct FileList
 * @brief A linked list of file nodes with a parallel index for O(1) access.
 */
typedef struct FileList
{
    FileNode  *head;
    FileNode  *tail;
    int        count;    // Total number of loaded files
    FileNode **index;    // Dynamic array of node pointers (by id)
    size_t     capacity; // Allocated size of index array
} FileList;

// Initialize an empty FileList.
FileList CreateFileList(void);

// Parse a file and append it as a new FileNode. Grows the index automatically.
void AddFile(FileList *list, const char *filename);

// Retrieve a file node in O(1) using the index array.
FileNode *GetFileByIndex(FileList *list, int i);

// Print the list of loaded files with their ids and filenames.
void PrintFileList(FileList list);

// Free all file nodes, their paragraph lists, and the index array.
void FreeFileList(FileList *list);

#endif // FILE_LIST_H