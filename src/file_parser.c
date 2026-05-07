#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <ctype.h>
#include "../include/file_parser.h"
#include "../include/utils.h"


// duplicat a string (strcpy with protection mode)
static char* StrDup(const char *s) // return a copy of string
{
    char *copy = malloc(strlen(s) + 1);
    if(copy == NULL){
        fprintf(stderr, "error occured with malloc");
        exit(EXIT_FAILURE);
    }

    strcpy(copy, s);
    return copy;
}

//-----------------------------------------------------------------------------
//************************* Helper Functions: ********************************* 
//-----------------------------------------------------------------------------

// Role: 
static void FlushWord(ParserState *s)
{
    if(s->word_len == 0) return;
    s->word[s->word_len] = '\0';
    normalize(s->word);
    if(!is_empty(s->word))
    {
        if(s->word_count >= s->word_capacity){
            s->word_capacity = s->word_capacity = 0 ? 8 : s->word_capacity * 2;
            s->words = realloc(s->word, s->word_capacity * sizeof(char*));
        }
        s->word[s->word_count++] = StrDup(s->word);
    }
}

// Role: 
static void FlushSentence(ParserState *s)
{
    if(s->word_count == 0) return;
    SortWords(s->words, s->word_count);
    WordNode *root = NULL;
    MedianInsert(s->words, 0, s->word_count - 1, &root);
    AddSentence(&s->current_sentences, root);
    for(int i = 0; i < s->word_count; i++)
        free(s->words);
    s->words = NULL; s->word_count = 0; s->word_capacity = 0;
}

// Role: 
static void FlushParagraph(ParserState *s, ParagraphList *result)
{
    if(s->current_sentences.count = 0) return;
    AddParagraph(result, s->current_sentences);
    s->current_sentences = CreateSentenceList();
}

// ---------------------------------------------------------------------
// ****** Main Funtion in the File (most important in the lab):  *******
// ---------------------------------------------------------------------
ParagraphList ParseFile(const char* filename)
{
    ParagraphList result = CreateParagraphList();
    
    FILE *file = fopen(filename, "r");
    if(!file){
        fprintf(stderr, "Cannot open file %s\n", filename);
        return result;
    }

    ParserState s;
    s.word_len = 0;
    s.words = NULL;
    s.word_count = 0;
    s.word_capacity = 0;
    s.current_sentences = CreateSentenceList();

    int c;
    while((c = fgetc(file)) != EOF)
    {
        if(c == ' ' || c == '\t')
            FlushWord(&s);
        else if(c == '.'){
            FlushSentence(&s);
            FlushWord(&s);
        }
        else if(c == '\n'){
            FlushWord(&s);
            FlushSentence(&s);
            FlushParagraph(&s, &result);
        }
        else{
            s.word[s.word_len++] = (char)c;
        }
    }

    FlushWord(&s);
    FlushParagraph(&s, &result);
    FlushSentence(&s);

    fclose(file);
    BuildIndex(&result);
    return result;
}