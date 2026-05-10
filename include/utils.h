#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

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

void PrintError(const char *context, const char *message);

void HandleError(const char *context, const char *message, int fatal);

void *CheckedMalloc(size_t size, const char *context);

void *CheckedRealloc(void *ptr, size_t size, const char *context);

char *CheckedStrDup(const char *s, const char *context);

int IsReadableRegularFile(const char *path);

void SleepMillis(int ms);

void InitTerminal(void);

int ReadChar(void);

int ReadCharNonBlocking(void);

void GetTerminalSize(int *w, int *h);

void ClearScreen(void);

void FlushOutput(void);

void MoveCursor(int r, int c);

void PadSpaces(int n);

int VisibleLength(const char *s);

const char *TruncateVisible(const char *s, int max_vis, char *buf, int buf_sz);

void NormalizeWord(char *word);

int IsBlank(const char *word);

#endif 
