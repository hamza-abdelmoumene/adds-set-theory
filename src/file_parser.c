#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/file_parser.h"
#include "../include/utils.h"

// Duplicate a string — portable replacement for strdup.
static char *StrDup(const char *s)
{
    char *copy = (char *)malloc(strlen(s) + 1);
    if (copy == NULL)
    {
        fprintf(stderr, "StrDup: out of memory\n");
        exit(EXIT_FAILURE);
    }
    strcpy(copy, s);
    return copy;
}

//-----------------------------------------------------------------------------
//************************* Helper Functions: *********************************
//-----------------------------------------------------------------------------

// Flush the current word buffer into the words array.
// Normalizes, skips empty strings and stopwords, then grows the array if needed.
static void FlushWord(ParserState *s)
{
    if (s->word_len == 0)
        return;

    s->word[s->word_len] = '\0';
    s->word_len = 0; // FIX: reset length so the next word starts fresh

    normalize(s->word);

    // skip empty words and stopwords
    if (is_empty(s->word) || is_stopword(s->word))
        return;

    if (s->word_count >= s->word_capacity)
    {
        s->word_capacity = (s->word_capacity == 0) ? 8 : s->word_capacity * 2;
        char **tmp = (char **)realloc(s->words, s->word_capacity * sizeof(char *));
        if (tmp == NULL)
        {
            fprintf(stderr, "FlushWord: out of memory\n");
            exit(EXIT_FAILURE);
        }
        s->words = tmp;
    }

    s->words[s->word_count++] = StrDup(s->word);
}

// Flush the current words array into a balanced BST and append it as a new sentence.
static void FlushSentence(ParserState *s)
{
    if (s->word_count == 0)
        return;

    SortWords(s->words, s->word_count);

    WordNode *root = NULL;
    MedianInsert(s->words, 0, (size_t)(s->word_count - 1), &root);

    AddSentence(&s->current_sentences, root);

    for (int i = 0; i < s->word_count; i++)
        free(s->words[i]);

    free(s->words);
    s->words = NULL;
    s->word_count = 0;
    s->word_capacity = 0;
}

// Flush the current sentence list into a new paragraph.
static void FlushParagraph(ParserState *s, ParagraphList *result)
{
    if (s->current_sentences.count == 0)
        return;

    // build the sentence index before handing ownership to the paragraph node
    BuildSentenceIndex(&s->current_sentences);

    AddParagraph(result, s->current_sentences);
    s->current_sentences = CreateSentenceList();
}

// ---------------------------------------------------------------------
// ****** Main Function in the File (most important in the lab):  ******
// ---------------------------------------------------------------------

ParagraphList ParseFile(const char *filename)
{
    ParagraphList result = CreateParagraphList();

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "ParseFile: cannot open file '%s'\n", filename);
        return result;
    }

    ParserState s;
    s.word_len = 0;
    s.words = NULL;
    s.word_count = 0;
    s.word_capacity = 0;
    s.current_sentences = CreateSentenceList();

    int c;
    while ((c = fgetc(file)) != EOF)
    {
        if (c == ' ' || c == '\t')
        {
            FlushWord(&s);
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
            if (s.word_len < (int)sizeof(s.word) - 1) // guard against buffer overflow
                s.word[s.word_len++] = (char)c;
        }
    }

    // Sentence must be flushed before paragraph, otherwise the last sentence is lost.
    FlushWord(&s);
    FlushSentence(&s);
    FlushParagraph(&s, &result);

    fclose(file);
    BuildIndex(&result);
    return result;
}