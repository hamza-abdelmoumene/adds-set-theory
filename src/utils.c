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

// Prints an error message in a nice format: "Where: What happened"
void PrintError(const char *context, const char *message)
{
    const char *where = (context && context[0]) ? context : "Error";
    const char *what = (message && message[0]) ? message : "unexpected error";
    fprintf(stderr, "%s: %s\n", where, what);
}

// Shows the error and exits the program if it's a big problem (fatal)
void HandleError(const char *context, const char *message, int fatal)
{
    PrintError(context, message);
    if (fatal)
        exit(EXIT_FAILURE);
}

// Like normal malloc, but if it fails, the program just quits instead of crashing later
void *CheckedMalloc(size_t size, const char *context)
{
    if (size == 0)
        size = 1; // 0-byte malloc can be weird, so we ask for 1 byte at least

    void *ptr = malloc(size);
    if (ptr == NULL)
        HandleError(context, "out of memory", 1); // Fatal error: we can't continue without memory

    return ptr;
}

// Like normal realloc, but safe. Used when we need to grow our word lists or arrays.
void *CheckedRealloc(void *ptr, size_t size, const char *context)
{
    if (size == 0)
        size = 1; 

    void *new_ptr = realloc(ptr, size);
    if (new_ptr == NULL)
        HandleError(context, "out of memory", 1);

    return new_ptr;
}

// Copies a string into new memory and makes sure the allocation worked
char *CheckedStrDup(const char *s, const char *context)
{
    if (s == NULL)
        return NULL;

    char *copy = (char *)CheckedMalloc(strlen(s) + 1, context);
    strcpy(copy, s);
    return copy;
}

// Checks if the file path is valid, if it's a real file (not a folder), and if we can read it.
int IsReadableRegularFile(const char *path)
{
    struct stat st;
    FILE *file;

    if (path == NULL || path[0] == '\0')
        return 0;

    // stat() tells us info about the file from the OS
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode))
        return 0;

    // Final check: actually try to open it
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

// Pauses the program for a bit. ms is in milliseconds.
void SleepMillis(int ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

// Nothing needed for Linux/Mac terminals to start up.
void InitTerminal(void) {}

// Reads a single key from the keyboard. We turn off "echo" and "canonical mode" 
// so the user doesn't have to press Enter to send the key to us.
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

// Tries to read a key but gives up quickly (100ms) if nothing is pressed.
// Useful for detecting special keys like arrows.
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

// Asks the OS for the current terminal window size.
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
        *w = 120; // Default if we can't find out
        *h = 40;
    }
}
#endif

// Clears the whole screen using ANSI escape codes.
void ClearScreen(void)
{
    printf("\033[2J\033[3J\033[H");
    fflush(stdout);
}

// Forces the terminal to show everything we've printed so far.
void FlushOutput(void) { fflush(stdout); }

// Moves the text cursor to row r and column c.
void MoveCursor(int r, int c)
{
    printf("\033[%d;%dH", r + 1, c + 1);
    FlushOutput();
}

// Prints a specific number of spaces to the screen.
void PadSpaces(int n)
{
    for (int i = 0; i < n; i++)
        putchar(' ');
}

// Counts how many characters you'd actually see on screen.
// This is important because color codes (like \033[31m) shouldn't be counted as "visible".
int VisibleLength(const char *s)
{
    int len = 0;
    for (int i = 0; s[i]; i++)
    {
        if (s[i] == '\033') // Skip escape sequences
        {
            while (s[i] && s[i] != 'm')
                i++;
            continue;
        }
        unsigned char c = (unsigned char)s[i];
        if (c < 0x80 || c >= 0xC0) // UTF-8 logic to count only the start of characters
            len++;
    }
    return len;
}

// Truncates a string to a certain visible length, but doesn't break the color codes.
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

// Takes a word and makes it clean: lowercase letters only, no punctuation.
// "Hello!" becomes "hello".
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

// Returns true if the string is empty or just has spaces/tabs.
int IsBlank(const char *word)
{
    if (word == NULL)
        return 1;

    for (int i = 0; word[i] != '\0'; i++)
        if (!isspace((unsigned char)word[i]))
            return 0;

    return 1;
}
