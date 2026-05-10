#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include "paragraph_ll.h"
#include "sentence_ll.h"

/**
 * @file file_parser.h
 * @brief File loading and parsing functionality.
 */

/**
 * @struct ParserState
 * @brief Holds the internal state of the parser during file reading.
 */
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

/**
 * @brief Parses a text file into a structured paragraph list.
 * 
 * Rules:
 * - spaces/tabs separate words
 * - '.' separates sentences
 * - '\n' separates paragraphs
 * 
 * @param filename Path to the file to parse.
 * @return A constructed ParagraphList representing the document.
 */
ParagraphList ParseFile(const char *filename);

#endif // FILE_PARSER_H