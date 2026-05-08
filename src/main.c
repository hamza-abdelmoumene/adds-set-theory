#include <stdio.h>
#include <stdlib.h>
#include "../include/sets_ops.h"
#include "../include/file_parser.h"

static void print_separator(const char *title)
{
    printf("\n══════════════════════════════════════════\n");
    printf("  %s\n", title);
    printf("══════════════════════════════════════════\n");
}

int main(void)
{
    // --- Parse two test files ---
    ParagraphList file1 = ParseFile("tests/test1.txt");
    ParagraphList file2 = ParseFile("tests/test2.txt");

    printf("File 1 loaded: %d paragraph(s)\n", file1.count);
    PrintParagraphs(file1);

    printf("File 2 loaded: %d paragraph(s)\n", file2.count);
    PrintParagraphs(file2);

    // --- Word Level Operations ---
    // Use first sentence of first paragraph from each file
    if (file1.count > 0 && file2.count > 0)
    {
        ParagraphNode *p1 = GetParagraphByIndex(&file1, 0);
        ParagraphNode *p2 = GetParagraphByIndex(&file2, 0);

        if (p1->val.count > 0 && p2->val.count > 0)
        {
            SentenceNode *s1 = GetSentenceByIndex(&p1->val, 0);
            SentenceNode *s2 = GetSentenceByIndex(&p2->val, 0);

            printf("\nSentence A words: ");
            Inorder(s1->val);
            printf("\nSentence B words: ");
            Inorder(s2->val);
            printf("\n");

            print_separator("WORD UNION");
            WordNode *wu = WordUnion(s1->val, s2->val);
            Inorder(wu);
            printf("\n");
            FreeTree(&wu);

            print_separator("WORD INTERSECTION");
            WordNode *wi = WordIntersection(s1->val, s2->val);
            Inorder(wi);
            printf("\n");
            FreeTree(&wi);

            print_separator("WORD DIFFERENCE (A - B)");
            WordNode *wd = WordDifference(s1->val, s2->val);
            Inorder(wd);
            printf("\n");
            FreeTree(&wd);
        }

        // --- Sentence Level Operations ---
        print_separator("SENTENCE UNION");
        SentenceList su = SentenceUnion(p1->val, p2->val);
        PrintSentences(su);
        FreeSentenceList(&su);

        print_separator("SENTENCE INTERSECTION");
        SentenceList si = SentenceIntersection(p1->val, p2->val);
        PrintSentences(si);
        FreeSentenceList(&si);

        print_separator("SENTENCE DIFFERENCE (A - B)");
        SentenceList sd = SentenceDifference(p1->val, p2->val);
        PrintSentences(sd);
        FreeSentenceList(&sd);
    }

    // --- Paragraph Level Operations ---
    print_separator("PARAGRAPH UNION");
    ParagraphList pu = ParagraphUnion(file1, file2);
    PrintParagraphs(pu);
    FreeParagraphList(&pu);

    print_separator("PARAGRAPH INTERSECTION");
    ParagraphList pi = ParagraphIntersection(file1, file2);
    PrintParagraphs(pi);
    FreeParagraphList(&pi);

    print_separator("PARAGRAPH DIFFERENCE (A - B)");
    ParagraphList pd = ParagraphDifference(file1, file2);
    PrintParagraphs(pd);
    FreeParagraphList(&pd);

    // --- Cleanup ---
    FreeParagraphList(&file1);
    FreeParagraphList(&file2);

    printf("\nAll operations completed successfully.\n");
    return 0;
}
