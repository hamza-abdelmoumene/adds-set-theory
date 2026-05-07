#ifndef UTILS_H
#define UTILS_H

/**
 * @file utils.h
 * @brief Utility functions for text processing.
 */

// Normalizes a word: converts to lowercase and strips non-alpha characters in-place.
void normalize(char *word);

// Checks if a word is empty or consists purely of whitespace. Returns 1 if true, 0 otherwise.
int is_empty(const char *word);

#endif // UTILS_H