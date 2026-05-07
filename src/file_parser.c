#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/file_parser.h"
#include "../include/utils.h"

ParagraphList ParseFile(const char *filename)
{
    ParagraphList doc = CreateParagraphList();
    // TODO: Implement two-pass parsing per sentence:
    // TODO: 1) collect words into a dynamic array
    // TODO: 2) sort array (qsort + strcmp) and build BST via MedianInsert
    // TODO: After parsing, call BuildIndex to fill the paragraph index array
    return doc;
}
