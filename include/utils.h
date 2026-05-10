#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

/**
 * @file utils.h
 * @brief Utility functions for text processing and terminal helpers.
 */

/* ANSI / styling helpers */
#define RST "\033[0m"
#define BLD "\033[1m"
#define DIM "\033[2m"
#define HCUR "\033[?25l"
#define SCUR "\033[?25h"

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

/* Cross-platform terminal helpers */

/* Error handling helpers */

/**
 * @brief Prints a standardized non-fatal error message to stderr.
 * @param context Name of the function/module where the error happened.
 * @param message Human-readable explanation of the error.
 */
void PrintError(const char *context, const char *message);

/**
 * @brief Handles an error and optionally terminates the program.
 * @param context Name of the function/module where the error happened.
 * @param message Human-readable explanation of the error.
 * @param fatal Non-zero to exit after printing the error.
 */
void HandleError(const char *context, const char *message, int fatal);

/**
 * @brief Allocates memory or terminates with a standardized error.
 * @param size Number of bytes to allocate.
 * @param context Name of the allocation site.
 * @return Allocated memory block.
 */
void *CheckedMalloc(size_t size, const char *context);

/**
 * @brief Reallocates memory or terminates with a standardized error.
 * @param ptr Existing allocation, or NULL.
 * @param size Number of bytes to allocate.
 * @param context Name of the allocation site.
 * @return Reallocated memory block.
 */
void *CheckedRealloc(void *ptr, size_t size, const char *context);

/**
 * @brief Duplicates a string or terminates with a standardized error.
 * @param s String to duplicate.
 * @param context Name of the allocation site.
 * @return Newly allocated string copy, or NULL when s is NULL.
 */
char *CheckedStrDup(const char *s, const char *context);

/**
 * @brief Suspends execution for the specified number of milliseconds.
 * @param ms The sleep duration in milliseconds.
 */
void SleepMillis(int ms);

/**
 * @brief Initializes the terminal for proper UTF-8 and raw input handling.
 */
void InitTerminal(void);

/**
 * @brief Reads a single character from standard input (blocking).
 * @return The ASCII code of the pressed key.
 */
int ReadChar(void);

/**
 * @brief Reads a single character from standard input without blocking.
 * @return The ASCII code of the pressed key, or -1 if no key was pressed.
 */
int ReadCharNonBlocking(void);

/**
 * @brief Retrieves the current terminal dimensions.
 * @param w Pointer to store the terminal width.
 * @param h Pointer to store the terminal height.
 */
void GetTerminalSize(int *w, int *h);

/* Primitive helpers */

/**
 * @brief Clears the terminal screen.
 */
void ClearScreen(void);

/**
 * @brief Flushes the standard output buffer.
 */
void FlushOutput(void);

/**
 * @brief Moves the terminal cursor to the specified row and column.
 * @param r The row index.
 * @param c The column index.
 */
void MoveCursor(int r, int c);

/**
 * @brief Prints 'n' space characters to the standard output.
 * @param n Number of spaces to print.
 */
void PadSpaces(int n);

/**
 * @brief Calculates the visible length of a string (ignoring ANSI escape codes).
 * @param s The string to evaluate.
 * @return The visible length of the string.
 */
int VisibleLength(const char *s);

/**
 * @brief Truncates a string to a maximum visible length.
 * @param s The input string.
 * @param max_vis The maximum visible length.
 * @param buf The buffer to store the truncated string.
 * @param buf_sz The size of the buffer.
 * @return A pointer to the resulting buffer.
 */
const char *TruncateVisible(const char *s, int max_vis, char *buf, int buf_sz);

/**
 * @brief Normalizes a word: converts to lowercase and strips non-alpha characters in-place.
 * @param word The word to normalize.
 */
void NormalizeWord(char *word);

/**
 * @brief Checks if a word is empty or consists purely of whitespace.
 * @param word The word to check.
 * @return 1 if true, 0 otherwise.
 */
int IsBlank(const char *word);

#endif // UTILS_H
