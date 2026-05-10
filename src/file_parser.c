#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/file_parser.h"
#include "../include/utils.h"

// Helper function: wrapper around CheckedStrDup for local use.
static char *StrDup(const char *s)
{
    return CheckedStrDup(s, "StrDup");
}

// Finishes the word we were reading and adds it to our list.
static void FlushWord(ParserState *s)
{
    if (s->word_len == 0)
        return;

    s->word[s->word_len] = '\0'; // Null-terminate the string
    s->word_len = 0;             // Reset for the next word

    NormalizeWord(s->word);      // Clean it up (lowercase, no dots)

    if (IsBlank(s->word))        // If it was just punctuation, ignore it
        return;

    // Grow the word array if we need more space
    if (s->word_count >= s->word_capacity)
    {
        s->word_capacity = (s->word_capacity == 0) ? 8 : s->word_capacity * 2;
        s->words = (char **)CheckedRealloc(s->words,
                                           s->word_capacity * sizeof(char *),
                                           "FlushWord");
    }

    s->words[s->word_count++] = StrDup(s->word); // Store a copy
}

// Finishes the current sentence, builds its word BST, and adds it to the list.
static void FlushSentence(ParserState *s)
{
    if (s->word_count == 0)
        return;

    // Sort words first to help build a balanced BST
    SortWords(s->words, s->word_count);

    WordNode *root = NULL;
    MedianInsert(s->words, 0, (size_t)(s->word_count - 1), &root);

    s->sentence_buf[s->sentence_len] = '\0';
    AddSentence(&s->current_sentences, root, s->sentence_buf);

    // Clean up the temporary word array
    for (int i = 0; i < s->word_count; i++)
        free(s->words[i]);

    free(s->words);
    s->words = NULL;
    s->word_count = 0;
    s->word_capacity = 0;
    s->sentence_len = 0;
    s->sentence_buf[0] = '\0';
}

// Finishes the current paragraph and adds it to our final result.
static void FlushParagraph(ParserState *s, ParagraphList *result)
{
    if (s->current_sentences.count == 0)
        return;

    BuildSentenceIndex(&s->current_sentences); // Make the sentences easy to find
    s->paragraph_buf[s->paragraph_len] = '\0';
    AddParagraph(result, s->current_sentences, s->paragraph_buf);
    
    // Reset for the next paragraph
    s->current_sentences = CreateSentenceList();
    s->paragraph_len = 0;
    s->paragraph_buf[0] = '\0';
}

// The master function: opens a file and builds the whole structure.
ParagraphList ParseFile(const char *filename)
{
    ParagraphList result = CreateParagraphList();

    if (filename == NULL) return result;

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        PrintError("ParseFile", "cannot open file");
        return result;
    }

    // Initialize our "scratchpad"
    ParserState s;
    s.word_len = 0;
    s.words = NULL;
    s.word_count = 0;
    s.word_capacity = 0;
    s.current_sentences = CreateSentenceList();
    s.sentence_len = 0;
    s.sentence_buf[0] = '\0';
    s.paragraph_len = 0;
    s.paragraph_buf[0] = '\0';

    int c;
    // Read the file one character at a time
    while ((c = fgetc(file)) != EOF)
    {
        if (c == ' ' || c == '\t') // Space: end of a word
        {
            FlushWord(&s);
            if (s.sentence_len < (int)sizeof(s.sentence_buf) - 1)
                s.sentence_buf[s.sentence_len++] = ' ';
        }
        else if (c == '.') // Dot: end of a word AND end of a sentence
        {
            FlushWord(&s);
            FlushSentence(&s);
        }
        else if (c == '\n') // Newline: end of everything (word, sentence, paragraph)
        {
            FlushWord(&s);
            FlushSentence(&s);
            FlushParagraph(&s, &result);
        }
        else // Just a normal character (letter, digit, etc.)
        {
            if (s.word_len < (int)sizeof(s.word) - 1)
                s.word[s.word_len++] = (char)c;
            if (s.sentence_len < (int)sizeof(s.sentence_buf) - 1)
                s.sentence_buf[s.sentence_len++] = (char)c;
        }

        // Keep track of the raw text for the whole paragraph
        if (s.paragraph_len < (int)sizeof(s.paragraph_buf) - 1)
        {
            if (s.paragraph_len > 0 || !isspace(c)) // Skip leading spaces
                s.paragraph_buf[s.paragraph_len++] = (char)c;
        }
    }

    // Make sure we don't forget the last bit of text if the file doesn't end with a newline
    FlushWord(&s);
    FlushSentence(&s);
    FlushParagraph(&s, &result);

    fclose(file);
    BuildIndex(&result); // Build the final shortcut index
    return result;
}
