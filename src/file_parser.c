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

// Helper function: finalize the current word and add it to the word list.
static void FlushWord(ParserState *s)
{
    if (s->word_len == 0)
        return;

    s->word[s->word_len] = '\0';
    s->word_len = 0;

    NormalizeWord(s->word);

    if (IsBlank(s->word))
        return;

    if (s->word_count >= s->word_capacity)
    {
        s->word_capacity = (s->word_capacity == 0) ? 8 : s->word_capacity * 2;
        s->words = (char **)CheckedRealloc(s->words,
                                           s->word_capacity * sizeof(char *),
                                           "FlushWord");
    }

    s->words[s->word_count++] = StrDup(s->word);
}

// Helper function: build a sentence BST from collected words and reset buffers.
static void FlushSentence(ParserState *s)
{
    if (s->word_count == 0)
        return;

    SortWords(s->words, s->word_count);

    WordNode *root = NULL;
    MedianInsert(s->words, 0, (size_t)(s->word_count - 1), &root);

    s->sentence_buf[s->sentence_len] = '\0';
    AddSentence(&s->current_sentences, root, s->sentence_buf);

    for (int i = 0; i < s->word_count; i++)
        free(s->words[i]);

    free(s->words);
    s->words = NULL;
    s->word_count = 0;
    s->word_capacity = 0;
    s->sentence_len = 0;
    s->sentence_buf[0] = '\0';
}

// Helper function: build a paragraph node from collected sentences and reset buffers.
static void FlushParagraph(ParserState *s, ParagraphList *result)
{
    if (s->current_sentences.count == 0)
        return;

    BuildSentenceIndex(&s->current_sentences);
    s->paragraph_buf[s->paragraph_len] = '\0';
    AddParagraph(result, s->current_sentences, s->paragraph_buf);
    s->current_sentences = CreateSentenceList();
    s->paragraph_len = 0;
    s->paragraph_buf[0] = '\0';
}

// Main function: parse a text file into a ParagraphList structure.
ParagraphList ParseFile(const char *filename)
{
    ParagraphList result = CreateParagraphList();

    if (filename == NULL)
    {
        PrintError("ParseFile", "filename must not be NULL");
        return result;
    }

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        PrintError("ParseFile", "cannot open file");
        return result;
    }

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
    while ((c = fgetc(file)) != EOF)
    {
        if (c == ' ' || c == '\t')
        {
            FlushWord(&s);
            if (s.sentence_len < (int)sizeof(s.sentence_buf) - 1)
                s.sentence_buf[s.sentence_len++] = ' ';
        }
        else if (c == '.')
        {
            FlushWord(&s);
            FlushSentence(&s);
        }
        else if (c == '\n')
        {
            FlushWord(&s);
            FlushSentence(&s);
            FlushParagraph(&s, &result);
        }
        else
        {
            if (s.word_len < (int)sizeof(s.word) - 1)
                s.word[s.word_len++] = (char)c;
            if (s.sentence_len < (int)sizeof(s.sentence_buf) - 1)
                s.sentence_buf[s.sentence_len++] = (char)c;
        }

        if (s.paragraph_len < (int)sizeof(s.paragraph_buf) - 1)
        {
            // Skip leading whitespace when starting a new paragraph.
            if (s.paragraph_len > 0 || !isspace(c))
                s.paragraph_buf[s.paragraph_len++] = (char)c;
        }
    }

    FlushWord(&s);
    FlushSentence(&s);
    FlushParagraph(&s, &result);

    fclose(file);
    BuildIndex(&result);
    return result;
}
