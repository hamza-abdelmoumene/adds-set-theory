#ifndef FILE_LIST_H
#define FILE_LIST_H

#include <stdlib.h>
#include <string.h>
#include "paragraph_ll.h"

#define MAX_FILENAME 256

typedef struct FileNode
{
    int          id;                    
    char         filename[MAX_FILENAME]; 
    ParagraphList val;                  
    struct FileNode *addr;              
} FileNode;

typedef struct FileList
{
    FileNode  *head;
    FileNode  *tail;
    int        count;    
    FileNode **index;    
    size_t     capacity; 
} FileList;


FileList CreateFileList(void);


int AddFile(FileList *list, const char *filename);


FileNode *GetFileByIndex(FileList *list, int i);


void PrintFileList(FileList list);


void FreeFileList(FileList *list);

#endif 
