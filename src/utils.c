#include <ctype.h>
#include <string.h>
#include "../include/utils.h"

// Normalizes a word: converts to lowercase and strips non-alpha characters in-place.
void normalize(char *word)
{
    if (word == NULL)
        return;

    int index = 0;
    for (int i = 0; word[i] != '\0'; i++)
    {
        if (isalpha((unsigned char)word[i]))
            word[index++] = (char)tolower((unsigned char)word[i]);
    }
    word[index] = '\0';
}

// Checks if a word is empty or consists purely of whitespace. Returns 1 if true, 0 otherwise.
int is_empty(const char *word)
{
    if (word == NULL)
        return 1;

    for (int i = 0; word[i] != '\0'; i++)
        if (!isspace((unsigned char)word[i]))
            return 0;

    return 1;
}