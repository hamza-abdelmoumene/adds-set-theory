#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include "../include/utils.h"

#ifndef S_ISREG
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

// Helper function: print a consistent non-fatal error message.
void PrintError(const char *context, const char *message)
{
    const char *where = (context && context[0]) ? context : "Error";
    const char *what = (message && message[0]) ? message : "unexpected error";
    fprintf(stderr, "%s: %s\n", where, what);
}

// Helper function: report an error and optionally abort the program.
void HandleError(const char *context, const char *message, int fatal)
{
    PrintError(context, message);
    if (fatal)
        exit(EXIT_FAILURE);
}

// Helper function: allocate memory safely and abort on failure.
void *CheckedMalloc(size_t size, const char *context)
{
    if (size == 0)
        size = 1; // avoid zero-byte allocation

    void *ptr = malloc(size);
    if (ptr == NULL)
        HandleError(context, "out of memory", 1);

    return ptr;
}

// Helper function: reallocate memory safely and abort on failure.
void *CheckedRealloc(void *ptr, size_t size, const char *context)
{
    if (size == 0)
        size = 1; // avoid zero-byte allocation

    void *new_ptr = realloc(ptr, size);
    if (new_ptr == NULL)
        HandleError(context, "out of memory", 1);

    return new_ptr;
}

// Helper function: duplicate a string with checked allocation.
char *CheckedStrDup(const char *s, const char *context)
{
    if (s == NULL)
        return NULL;

    char *copy = (char *)CheckedMalloc(strlen(s) + 1, context);
    strcpy(copy, s);
    return copy;
}

// Helper function: validate that a path is a readable regular file.
int IsReadableRegularFile(const char *path)
{
    struct stat st;
    FILE *file;

    if (path == NULL || path[0] == '\0')
        return 0;

    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode))
        return 0;

    file = fopen(path, "r");
    if (file == NULL)
        return 0;

    fclose(file);
    return 1;
}

#ifdef _WIN32
#include <windows.h>
#include <conio.h>

// Helper function: sleep for the requested number of milliseconds (Windows).
void SleepMillis(int ms) { Sleep(ms); }

// Helper function: enable ANSI output and UTF-8 on Windows terminals.
void InitTerminal(void)
{
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD m = 0;
    GetConsoleMode(h, &m);
    SetConsoleMode(h, m | 0x0004);
    SetConsoleOutputCP(65001);
}

// Helper function: read one keypress (blocking) on Windows.
int ReadChar(void) { return _getch(); }

// Helper function: read one keypress with no blocking on Windows.
int ReadCharNonBlocking(void) { return _kbhit() ? _getch() : -1; }

// Helper function: get console width/height on Windows.
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

// Helper function: sleep for the requested number of milliseconds (POSIX).
void SleepMillis(int ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

// Helper function: no terminal initialization needed on POSIX.
void InitTerminal(void) {}

// Helper function: read one keypress (blocking) with raw mode on POSIX.
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

// Helper function: read one keypress with a short timeout on POSIX.
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

// Helper function: get terminal width/height on POSIX.
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

// Helper function: clear the terminal screen and scrollback.
void ClearScreen(void)
{
    printf("\033[2J\033[3J\033[H");
    fflush(stdout);
}

// Helper function: flush stdout to force updates on screen.
void FlushOutput(void) { fflush(stdout); }

// Helper function: move the cursor to a specific row/column.
void MoveCursor(int r, int c)
{
    printf("\033[%d;%dH", r + 1, c + 1);
    FlushOutput();
}

// Helper function: print a fixed number of spaces.
void PadSpaces(int n)
{
    for (int i = 0; i < n; i++)
        putchar(' ');
}

// Helper function: compute visible length while skipping ANSI sequences.
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

// Helper function: truncate a string by visible length without breaking ANSI codes.
const char *TruncateVisible(const char *s, int max_vis, char *buf, int buf_sz)
{
    int vlen = 0, i = 0, out = 0;
    while (s[i] && out < buf_sz - 1)
    {
        if (s[i] == '\033')
        {

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

        while (s[i] && ((unsigned char)s[i] & 0xC0) == 0x80 && out < buf_sz - 1)
            buf[out++] = s[i++];
    }
    buf[out] = '\0';
    return buf;
}

// Helper function: normalize a word to lowercase letters only.
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

// Helper function: check whether a string is empty or whitespace-only.
int IsBlank(const char *word)
{
    if (word == NULL)
        return 1;

    for (int i = 0; word[i] != '\0'; i++)
        if (!isspace((unsigned char)word[i]))
            return 0;

    return 1;
}
