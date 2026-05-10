#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include "paragraph_ll.h"
#include "sentence_ll.h"

typedef struct {
    char         word[256];
    int          word_len;
    char       **words;
    int          word_count;
    int          word_capacity;
    SentenceList current_sentences;
    char         sentence_buf[4096];
    int          sentence_len;
    char         paragraph_buf[65536];
    int          paragraph_len;
} ParserState;

ParagraphList ParseFile(const char *filename);

#endif 