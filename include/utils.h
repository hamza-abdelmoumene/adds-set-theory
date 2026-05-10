/*
 * utils.h - Common helper functions used everywhere in the project.
 * These handle things like memory, terminal colors, and basic string cleaning.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/* ANSI escape codes for colors and terminal control */
#define RST "\033[0m"
#define BLD "\033[1m"
#define DIM "\033[2m"
#define HCUR "\033[?25l"
#define SCUR "\033[?25h"

/* Ocean-themed blue colors for the UI */
#define C1 "\033[38;2;5;5;15m"
#define C2 "\033[38;2;0;10;40m"
#define C3 "\033[38;2;0;20;80m"
#define C4 "\033[38;2;0;40;130m"
#define C5 "\033[38;2;0;60;180m"
#define C6 "\033[38;2;0;90;210m"
#define C7 "\033[38;2;0;130;220m"
#define C8 "\033[38;2;0;170;230m"
#define C9 "\033[38;2;0;210;240m"
#define C10 "\033[38;2;80;230;255m"
#define C11 "\033[38;2;160;240;255m"
#define C12 "\033[38;2;220;248;255m"
#define WHT "\033[38;2;255;255;255m"
#define GR1 "\033[38;2;80;80;80m"
#define GR2 "\033[38;2;120;130;150m"
#define RED "\033[38;2;220;60;60m"
#define GRN "\033[38;2;60;210;110m"

/* Prints a simple error message to the console */
void PrintError(const char *context, const char *message);

/* Handles errors: prints them and can even stop the program if fatal is true */
void HandleError(const char *context, const char *message, int fatal);

/* Safe malloc: if it fails to get memory, the program stops immediately */
void *CheckedMalloc(size_t size, const char *context);

/* Safe realloc: grows/shrinks memory and stops if it fails */
void *CheckedRealloc(void *ptr, size_t size, const char *context);

/* Safe strdup: copies a string and stops if it fails to get memory */
char *CheckedStrDup(const char *s, const char *context);

/* Checks if a file actually exists and we have permission to read it */
int IsReadableRegularFile(const char *path);

/* Makes the program wait for a few milliseconds */
void SleepMillis(int ms);

/* Sets up the terminal (mostly for Windows to handle colors) */
void InitTerminal(void);

/* Reads one key from the keyboard without waiting for Enter */
int ReadChar(void);

/* Tries to read a key but doesn't wait if nothing is pressed */
int ReadCharNonBlocking(void);

/* Asks the terminal how big it is (width and height) */
void GetTerminalSize(int *w, int *h);

/* Wipes everything from the screen */
void ClearScreen(void);

/* Makes sure everything we printed actually shows up on screen */
void FlushOutput(void);

/* Jumps the cursor to a specific spot on the screen */
void MoveCursor(int r, int c);

/* Just prints a bunch of spaces */
void PadSpaces(int n);

/* Counts how many characters are visible (skips color codes) */
int VisibleLength(const char *s);

/* Cuts a string short but keeps the color codes working */
const char *TruncateVisible(const char *s, int max_vis, char *buf, int buf_sz);

/* Cleans up a word: makes it lowercase and removes punctuation */
void NormalizeWord(char *word);

/* Checks if a string is just empty or full of spaces */
int IsBlank(const char *word);

#endif 
