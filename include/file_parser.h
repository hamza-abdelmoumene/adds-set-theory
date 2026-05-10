/*
 * file_parser.h - This module reads a text file and breaks it down 
 * into paragraphs, sentences, and words.
 */

#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include "paragraph_ll.h"
#include "sentence_ll.h"

/* 
 * ParserState is like a "scratchpad". While we read the file character 
 * by character, we store partial words and sentences here until 
 * they are finished.
 */
typedef struct {
    char         word[256];          // Buffer for the word we are currently reading
    int          word_len;           // How many letters in the current word
    char       **words;              // A list of words for the current sentence
    int          word_count;         // How many words we have in the current sentence
    int          word_capacity;      // Size of the words array
    SentenceList current_sentences;  // List of sentences for the current paragraph
    char         sentence_buf[4096]; // Raw text of the current sentence
    int          sentence_len;
    char         paragraph_buf[65536]; // Raw text of the whole paragraph
    int          paragraph_len;
} ParserState;

/* The main function that turns a file into a big structure of paragraphs */
ParagraphList ParseFile(const char *filename);

#endif 
 