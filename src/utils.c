#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "../include/utils.h"

/* ─────────────────────────────────────────────────────────────────────────────
 *  ERROR HANDLING
 * ───────────────────────────────────────────────────────────────────────────── */
void PrintError(const char *context, const char *message)
{
    const char *where = (context && context[0]) ? context : "Error";
    const char *what = (message && message[0]) ? message : "unexpected error";
    fprintf(stderr, "%s: %s\n", where, what);
}

void HandleError(const char *context, const char *message, int fatal)
{
    PrintError(context, message);
    if (fatal)
        exit(EXIT_FAILURE);
}

void *CheckedMalloc(size_t size, const char *context)
{
    if (size == 0)
        size = 1;

    void *ptr = malloc(size);
    if (ptr == NULL)
        HandleError(context, "out of memory", 1);

    return ptr;
}

void *CheckedRealloc(void *ptr, size_t size, const char *context)
{
    if (size == 0)
        size = 1;

    void *new_ptr = realloc(ptr, size);
    if (new_ptr == NULL)
        HandleError(context, "out of memory", 1);

    return new_ptr;
}

char *CheckedStrDup(const char *s, const char *context)
{
    if (s == NULL)
        return NULL;

    char *copy = (char *)CheckedMalloc(strlen(s) + 1, context);
    strcpy(copy, s);
    return copy;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  CROSS-PLATFORM LAYER
 * ───────────────────────────────────────────────────────────────────────────── */
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
/* Sleep for a given duration in milliseconds (Windows). */
void SleepMillis(int ms) { Sleep(ms); }
/* Initialize Windows terminal settings for UTF-8 output. */
void InitTerminal(void)
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD m = 0;
    GetConsoleMode(h, &m);
    SetConsoleMode(h, m | 0x0004);
    SetConsoleOutputCP(65001);
}
/* Read a single character (blocking) from the console. */
int ReadChar(void) { return _getch(); }
/* Read a single character (non-blocking) from the console. */
int ReadCharNonBlocking(void) { return _kbhit() ? _getch() : -1; }
/* Retrieve console dimensions. */
void GetTerminalSize(int *w, int *h)
{
    CONSOLE_SCREEN_BUFFER_INFO c;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &c))
    {
        *w = c.srWindow.Right - c.srWindow.Left + 1;
        *h = c.srWindow.Bottom - c.srWindow.Top + 1;
    }
    else
    {
        *w = 120;
        *h = 40;
    }
}
#else
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

/* Sleep for a given duration in milliseconds (POSIX). */
void SleepMillis(int ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}
/* POSIX terminals need no extra initialization here. */
void InitTerminal(void) {}

/* Blocking raw-mode read */
/* Read a single character (blocking) from stdin. */
int ReadChar(void)
{
    struct termios o, n;
    unsigned char c = 0;
    ssize_t got;

    tcgetattr(STDIN_FILENO, &o);
    n = o;
    n.c_lflag &= (unsigned)~(ICANON | ECHO);
    n.c_cc[VMIN] = 1;
    n.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &n);
    got = read(STDIN_FILENO, &c, 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &o);
    return (got == 1) ? (int)c : -1;
}

/* Non-blocking ~100 ms timeout — used only to drain escape sequences */
/* Read a single character with a short timeout (~100 ms). */
int ReadCharNonBlocking(void)
{
    struct termios o, n;
    unsigned char c = 0;
    ssize_t got;

    tcgetattr(STDIN_FILENO, &o);
    n = o;
    n.c_lflag &= (unsigned)~(ICANON | ECHO);
    n.c_cc[VMIN] = 0;
    n.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &n);
    got = read(STDIN_FILENO, &c, 1);
    tcsetattr(STDIN_FILENO, TCSANOW, &o);
    return (got == 1) ? (int)c : -1;
}

/* Retrieve terminal dimensions. */
void GetTerminalSize(int *w, int *h)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
    {
        *w = ws.ws_col;
        *h = ws.ws_row;
    }
    else
    {
        *w = 120;
        *h = 40;
    }
}
#endif

/* ─────────────────────────────────────────────────────────────────────────────
 *  PRIMITIVE HELPERS
 * ───────────────────────────────────────────────────────────────────────────── */
/* Clear the terminal screen and reset the cursor. */
void ClearScreen(void)
{
    printf("\033[2J\033[3J\033[H");
    fflush(stdout);
}
/* Flush stdout to force pending output. */
void FlushOutput(void) { fflush(stdout); }
/* Move the cursor to the given row and column. */
void MoveCursor(int r, int c)
{
    printf("\033[%d;%dH", r + 1, c + 1);
    FlushOutput();
}
/* Print a fixed number of spaces. */
void PadSpaces(int n)
{
    for (int i = 0; i < n; i++)
        putchar(' ');
}

/* Visible character count — skips ANSI escapes & UTF-8 continuation bytes */
int VisibleLength(const char *s)
{
    int len = 0;
    for (int i = 0; s[i]; i++)
    {
        if (s[i] == '\033')
        {
            while (s[i] && s[i] != 'm')
                i++;
            continue;
        }
        unsigned char c = (unsigned char)s[i];
        if (c < 0x80 || c >= 0xC0)
            len++;
    }
    return len;
}

/*
 *  TruncateVisible - copy s into buf, stopping once visible length would exceed max_vis.
 *  Properly handles ANSI escape sequences (copied verbatim, not counted).
 *  Always null-terminates buf. Returns buf.
 */
const char *TruncateVisible(const char *s, int max_vis, char *buf, int buf_sz)
{
    int vlen = 0, i = 0, out = 0;
    while (s[i] && out < buf_sz - 1)
    {
        if (s[i] == '\033')
        {
            /* copy entire escape sequence verbatim */
            while (s[i] && s[i] != 'm' && out < buf_sz - 1)
                buf[out++] = s[i++];
            if (s[i] && out < buf_sz - 1)
                buf[out++] = s[i++];
            continue;
        }
        unsigned char c = (unsigned char)s[i];
        int is_leader = (c < 0x80 || c >= 0xC0);
        if (is_leader)
        {
            if (vlen >= max_vis)
                break;
            vlen++;
        }
        buf[out++] = s[i++];
        /* copy UTF-8 continuation bytes */
        while (s[i] && ((unsigned char)s[i] & 0xC0) == 0x80 && out < buf_sz - 1)
            buf[out++] = s[i++];
    }
    buf[out] = '\0';
    return buf;
}

/* Normalize a word by removing non-alpha characters and lowercasing in place. */
void NormalizeWord(char *word)
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

/* Check whether a word is empty or all whitespace. */
int IsBlank(const char *word)
{
    if (word == NULL)
        return 1;

    for (int i = 0; word[i] != '\0'; i++)
        if (!isspace((unsigned char)word[i]))
            return 0;

    return 1;
}
