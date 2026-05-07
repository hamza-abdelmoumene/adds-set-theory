#include <ctype.h>
#include <string.h>
#include "../include/utils.h"

// Normalizes a word: converts to lowercase and strips punctuation in-place.
void normalize(char *word)
{
    // logic - iterate and override the word in place, simple and effecient.
    int index = 0;
    for(int i = 0; word[i] != '\0'; i++ )
    {
        if(isalnum(word[i]))
        {
            word[index++] = tolower(word[i]);
        }
    }
    word[index] = '\0';
}

/* 
TODO: filter common words, based on user choice!
// Checks if a word is a stopword. Returns 1 if true, 0 otherwise.
int is_stopword(const char *word)
{
    
    return 0;
}
*/

// Checks if a word is empty or consists purely of whitespace. Returns 1 if true, 0 otherwise.
int is_empty(const char *word)
{
    for(int i = 0; word[i] != '\0'; i++)
        if(!isspace(word[i])) return 1;

    return 1;
}
