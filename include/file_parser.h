#ifndef FILE_PARSER_H
#define FILE_PARSER_H

/**
 * @file file_parser.h
 * @brief File loading and parsing functionality.
 */

#include "paragraph_ll.h"

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
