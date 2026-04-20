/*******************************************************************************
 * intro.c — ESI Intro Animation (library implementation)
 *
 * This file contains the full implementation of the intro animation module.
 * It is compiled as part of the project alongside main.c and the other modules.
 * Nothing here is exposed to the outside world except animation(), which is
 * declared in intro.h.
 *
 * Build (handled automatically by the Makefile):
 *     gcc src/*.c -Iinclude -o adds-set-theory
 ******************************************************************************/

/*
 * _POSIX_C_SOURCE 199309L — Feature-test macro for POSIX compliance.
 *
 * This must be defined BEFORE any #include. It tells the C standard library
 * to expose POSIX-specific functions like nanosleep(), which is used for
 * millisecond-precision sleeping on Linux/macOS. Without this line, the
 * compiler would not see nanosleep() and would throw an "implicit declaration"
 * warning or error.
 *
 * 199309L corresponds to POSIX.1b (1993), the revision that introduced
 * real-time extensions including nanosleep.
 */
#define _POSIX_C_SOURCE 199309L

/*
 * Standard library includes. Each one provides a set of functions and types
 * that we use somewhere in this file.
 */
#include <stdio.h>    /* printf, putchar, fflush, fgets — terminal I/O        */
#include <stdlib.h>   /* General utilities; included as good practice          */
#include <stdbool.h>  /* Provides the bool type and true/false constants       */
#include <string.h>   /* strlen, snprintf — string manipulation functions      */
#include <time.h>     /* struct timespec and nanosleep() on Linux/macOS        */
#include <ctype.h>    /* Character classification (isdigit, etc.)              */
#include <math.h>     /* abs() — absolute value, used in the separator effect  */
#include <stdint.h>   /* Fixed-width integer types like uint8_t                */

/*
 * Include our own header last. This ensures that if intro.h accidentally
 * depends on something from the standard headers above, it will find it.
 * It also lets the compiler verify that our definitions here match what
 * intro.h declares.
 */
#include "intro.h"


/* =============================================================================
 * CROSS-PLATFORM LAYER
 *
 * Windows and Linux/macOS expose completely different APIs for:
 *   - sleeping (Sleep vs usleep/nanosleep)
 *   - reading a single keypress without Enter (conio vs termios)
 *   - querying the terminal size (Win32 API vs ioctl)
 *   - enabling ANSI escape codes (Windows needs extra setup; Linux does not)
 *
 * We solve this by wrapping each platform's API in a common function name
 * (p_sleep, p_init, p_getch, p_term_size). The rest of the code only ever
 * calls these wrappers and works correctly on both platforms.
 *
 * The #ifdef _WIN32 block is compiled ONLY when building on Windows.
 * The #else block is compiled on Linux, macOS, and other POSIX systems.
 * ============================================================================= */

#ifdef _WIN32
    /*
     * Windows-specific headers:
     *   windows.h — the main Win32 API: Sleep(), GetConsoleScreenBufferInfo(), etc.
     *   conio.h   — console I/O: _getch() reads a keypress without waiting for Enter.
     *               This header only exists on Windows; it has no POSIX equivalent.
     */
    #include <windows.h>
    #include <conio.h>

    /*
     * p_sleep(ms) — sleep for 'ms' milliseconds.
     * Windows Sleep() already takes milliseconds, so this is a direct pass-through.
     */
    void p_sleep(int ms) { Sleep(ms); }

    /*
     * p_init() — platform initialisation.
     *
     * On Windows, the default console does NOT understand ANSI escape codes
     * (the sequences we use for colors and cursor movement). We must explicitly
     * enable them by adding the ENABLE_VIRTUAL_TERMINAL_PROCESSING flag (0x0004)
     * to the console's mode flags.
     *
     * We also call SetConsoleOutputCP(65001) to switch the code page to UTF-8,
     * which allows box-drawing characters (┌, ─, │, etc.) to render correctly.
     */
    void p_init(void) {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); /* get handle to stdout       */
        DWORD  m = 0;
        GetConsoleMode(h, &m);                      /* read current mode flags    */
        SetConsoleMode(h, m | 0x0004);              /* add ANSI processing flag   */
        SetConsoleOutputCP(65001);                  /* switch console to UTF-8    */
    }

    /*
     * p_getch() — read one keypress immediately, without requiring Enter.
     * _getch() is provided by conio.h and does exactly this on Windows.
     */
    int p_getch(void) { return _getch(); }

    /*
     * p_term_size(w, h) — fill *w and *h with the terminal's column and row count.
     *
     * CONSOLE_SCREEN_BUFFER_INFO holds the screen buffer geometry.
     * srWindow gives the visible window region; we compute its width and height
     * from the right/left and bottom/top edges respectively.
     * If the query fails we fall back to 120×40, a safe default size.
     */
    void p_term_size(int *w, int *h) {
        CONSOLE_SCREEN_BUFFER_INFO c;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &c)) {
            *w = c.srWindow.Right  - c.srWindow.Left + 1;
            *h = c.srWindow.Bottom - c.srWindow.Top  + 1;
        } else {
            *w = 120; *h = 40;
        }
    }

#else
    /*
     * POSIX-specific headers:
     *   unistd.h    — POSIX standard: usleep(), STDOUT_FILENO, STDIN_FILENO
     *   termios.h   — terminal attribute control: tcgetattr/tcsetattr
     *                 needed to switch the terminal into raw mode for p_getch()
     *   sys/ioctl.h — ioctl() + TIOCGWINSZ: query the terminal window size
     */
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>

    /*
     * p_sleep(ms) — sleep for 'ms' milliseconds.
     * usleep() takes microseconds, so we multiply by 1000.
     */
    void p_sleep(int ms) { usleep(ms * 1000); }

    /*
     * p_init() — no special setup needed on Linux/macOS.
     * ANSI escape codes work in any modern terminal emulator out of the box.
     * The function exists only so call sites compile on all platforms.
     */
    void p_init(void) {}

    /*
     * p_getch() — read one keypress without waiting for Enter.
     *
     * By default, the terminal operates in "canonical" (cooked) mode: input is
     * line-buffered and getchar() only returns after the user presses Enter.
     * To read a single keystroke immediately, we temporarily switch to "raw" mode:
     *
     *   1. Save current terminal settings in 'o'.
     *   2. Copy them into 'n' and clear two flags:
     *        ICANON — disables line buffering (input is now character-by-character)
     *        ECHO   — disables echoing typed characters back to the screen
     *   3. Apply the raw settings with tcsetattr (TCSANOW = take effect right now).
     *   4. Read one character with getchar() — returns immediately.
     *   5. Restore the original settings so the shell works normally after exit.
     */
    int p_getch(void) {
        struct termios o, n;
        int c;
        tcgetattr(STDIN_FILENO, &o);        /* save original settings            */
        n = o;                               /* copy into 'n' to modify           */
        n.c_lflag &= ~(ICANON | ECHO);      /* disable line buffering and echo   */
        tcsetattr(STDIN_FILENO, TCSANOW, &n); /* apply raw mode immediately      */
        c = getchar();                        /* read one char — no Enter needed  */
        tcsetattr(STDIN_FILENO, TCSANOW, &o); /* restore original settings       */
        return c;
    }

    /*
     * p_term_size(w, h) — fill *w and *h with the terminal's column and row count.
     *
     * ioctl() with TIOCGWINSZ asks the kernel for the terminal window size.
     * The result is stored in a struct winsize: ws_col = columns, ws_row = rows.
     * If ioctl fails (e.g., output is piped to a file) we fall back to 120×40.
     */
    void p_term_size(int *w, int *h) {
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
            *w = ws.ws_col;
            *h = ws.ws_row;
        } else {
            *w = 120; *h = 40;
        }
    }

#endif /* end of cross-platform layer */


/* =============================================================================
 * ANSI ESCAPE CODE MACROS
 *
 * ANSI escape sequences are byte strings interpreted by the terminal emulator
 * as commands rather than printable text. They all begin with ESC (\033),
 * followed by '[', optional parameters, and a letter that identifies the command.
 *
 * Color sequences end in 'm' and follow this format:
 *   \033[<code>m       — standard 16-color mode
 *   \033[38;2;R;G;Bm   — 24-bit true-color foreground (text color)
 *   \033[48;2;R;G;Bm   — 24-bit true-color background (cell color)
 * ============================================================================= */

/* Text attribute macros */
#define RST  "\033[0m"    /* Reset: clear ALL styling (color, bold, dim) to default */
#define BLD  "\033[1m"    /* Bold: heavier/brighter text                             */
#define DIM  "\033[2m"    /* Dim: slightly faded text                                */
#define HCUR "\033[?25l"  /* Hide cursor: suppresses blinking cursor during animation */
#define SCUR "\033[?25h"  /* Show cursor: restores cursor visibility when done        */

/*
 * 12-step blue-to-cyan color gradient for the logo and ASCII art.
 * Each macro sets the FOREGROUND (text) color using 24-bit RGB.
 * Applied to consecutive rows, they create a smooth dark-to-bright ramp:
 *   C1  = near-black  (darkest)
 *   C12 = near-white-cyan (brightest)
 */
#define C1  "\033[38;2;5;5;15m"
#define C2  "\033[38;2;0;10;40m"
#define C3  "\033[38;2;0;20;80m"
#define C4  "\033[38;2;0;40;130m"
#define C5  "\033[38;2;0;60;180m"
#define C6  "\033[38;2;0;90;210m"
#define C7  "\033[38;2;0;130;220m"
#define C8  "\033[38;2;0;170;230m"
#define C9  "\033[38;2;0;210;240m"
#define C10 "\033[38;2;80;230;255m"
#define C11 "\033[38;2;160;240;255m"
#define C12 "\033[38;2;220;248;255m"

/* Utility colors */
#define WHT "\033[38;2;255;255;255m"  /* Pure white text                        */
#define GR1 "\033[38;2;80;80;80m"    /* Medium gray (used in subtle footer text) */


/* =============================================================================
 * UTILITY FUNCTIONS
 *
 * Small helpers used throughout the animation. All are declared static so
 * they are private to this file and do not pollute the global namespace.
 * ============================================================================= */

/*
 * cls() — clear the terminal screen and move the cursor to the top-left.
 *
 * \033[2J — erase the entire visible screen content
 * \033[H  — move cursor to row 1, column 1 (home position)
 * fflush forces the output buffer to be written to the terminal immediately.
 */
static void cls(void) { printf("\033[2J\033[H"); fflush(stdout); }

/*
 * fl() — flush the output buffer.
 *
 * Terminal output is line-buffered by default: characters sit in a buffer
 * and are only sent to the screen when a newline is printed or the buffer
 * is full. During animations we print without newlines, so we call fl()
 * after every character to make it appear on screen immediately.
 */
static void fl(void) { fflush(stdout); }

/*
 * cur(r, c) — move the cursor to row r, column c (0-based).
 *
 * The ANSI sequence \033[row;colH moves the cursor to the given position.
 * ANSI uses 1-based indexing, so we add 1 to both r and c before printing.
 * This lets us position text anywhere without printing newlines.
 */
static void cur(int r, int c) { printf("\033[%d;%dH", r+1, c+1); fl(); }

/*
 * vis_len(s) — count the VISIBLE characters in a string, ignoring ANSI codes.
 *
 * The problem: strlen() counts every byte including ANSI escape sequences,
 * which are invisible. If we use strlen() to center a colored string we get
 * the wrong offset because the invisible bytes inflate the count.
 *
 * This function skips ANSI sequences (which start with \033 and end with 'm')
 * and only counts bytes that represent visible characters. It also handles
 * multi-byte UTF-8: continuation bytes (0x80–0xBF) are skipped so that one
 * multi-byte character is counted as one visible cell.
 */
static int vis_len(const char *s) {
    int len = 0;
    for (int i = 0; s[i]; i++) {
        if (s[i] == '\033') {
            /* skip the entire escape sequence up to and including 'm' */
            while (s[i] && s[i] != 'm') i++;
            continue;
        }
        unsigned char c = (unsigned char)s[i];
        /* count only the leading byte of each UTF-8 character */
        if (c < 0x80 || c >= 0xC0) len++;
    }
    return len;
}

/*
 * cprint(row, color, text) — print text centered on the screen at the given row.
 *
 * Steps:
 *   1. Get the terminal width.
 *   2. Compute the starting column = (terminal width - visible text length) / 2.
 *   3. Move the cursor there with cur().
 *   4. Print: color code, text, RST to reset styling.
 */
static void cprint(int row, const char *color, const char *text) {
    int tw, th; p_term_size(&tw, &th);
    int col = (tw - vis_len(text)) / 2;
    if (col < 0) col = 0;
    cur(row, col);
    printf("%s%s" RST, color, text);
    fl();
}

/*
 * type_at(row, color, text, delay) — typewriter effect: print text one character
 * at a time, centered at 'row', with 'delay' ms between each visible character.
 *
 * ANSI escape sequences inside 'text' are printed instantly (no delay) because
 * they are invisible — adding a delay for them would make the effect look uneven.
 * Spaces and newlines also skip the delay so only printable glyphs feel "typed".
 */
static void type_at(int row, const char *color, const char *text, int delay) {
    int tw, th; p_term_size(&tw, &th);
    int col = (tw - vis_len(text)) / 2;
    if (col < 0) col = 0;
    cur(row, col);
    printf("%s", color);   /* set color before we start printing characters */
    for (int i = 0; text[i]; i++) {
        if (text[i] == '\033') {
            /* print escape sequence bytes immediately with no delay */
            while (text[i] && text[i] != 'm') { putchar(text[i]); i++; }
            if (text[i]) putchar(text[i]);
            fl(); continue;
        }
        putchar(text[i]); fl();
        /* only delay after printable characters — not spaces or newlines */
        if (text[i] != ' ' && text[i] != '\n') p_sleep(delay);
    }
    printf(RST); fl();
}


/* =============================================================================
 * ESI LOGO — PIXEL ART RENDERER
 *
 * The logo is painted onto a virtual pixel canvas (CW × CH) as colored
 * rectangles, then converted to terminal characters. Two canvas rows are
 * merged into one terminal row using the Unicode "upper half block" character
 * ▀ (U+2580): its foreground color is the top pixel and its background color
 * is the bottom pixel, achieving double the vertical resolution.
 * ============================================================================= */

#define CW      80          /* canvas width  in pixels (= terminal columns used) */
#define CH      42          /* canvas height in pixels (21 terminal rows)        */
#define TROWS   (CH / 2)    /* terminal rows occupied by the logo image          */
#define INFO_ROWS 4         /* rows below the logo: separator + info lines       */
#define TOTAL_ROWS (TROWS + INFO_ROWS) /* total terminal rows for the whole block */

/* RGB color struct: one unsigned byte per channel (0–255) */
typedef struct { unsigned char r, g, b; } RGB;

/* The pixel grid: CH rows × CW columns */
static RGB canvas[CH][CW];

/* The three colors used in the logo */
static const RGB L_WHITE = {255, 255, 255}; /* canvas background                */
static const RGB L_BLACK = {  0,   0,   0}; /* letter strokes                   */
static const RGB L_BLUE  = { 30, 105, 180}; /* dot above the "i"                */

/*
 * clear_canvas() — fill every pixel with white, giving a clean background.
 */
static void clear_canvas(void) {
    for (int y = 0; y < CH; y++)
        for (int x = 0; x < CW; x++)
            canvas[y][x] = L_WHITE;
}

/*
 * fill_rect(x0, y0, w, h, c) — paint a filled rectangle on the canvas.
 *
 * Draws a rectangle of color 'c' starting at (x0, y0) with size w × h.
 * Bounds checks prevent writing outside the canvas array.
 */
static void fill_rect(int x0, int y0, int w, int h, RGB c) {
    for (int y = y0; y < y0+h && y < CH; y++)
        for (int x = x0; x < x0+w && x < CW; x++)
            if (x >= 0 && y >= 0)
                canvas[y][x] = c;
}

/*
 * Letter geometry constants — shared by all three letter-drawing functions:
 *   LS = stroke thickness (5 pixels)
 *   LT = top y coordinate where all letters begin (row 9)
 *   LB = bottom y coordinate where all letters end (row 35)
 */
#define LS  5   /* stroke width: thickness of every bar in the letters */
#define LT  9   /* top edge of all letter bodies                       */
#define LB  35  /* bottom edge of all letter bodies                    */

/*
 * draw_e() — draw the letter "E" using filled rectangles.
 *
 * The E consists of:
 *   - a full-height left vertical bar
 *   - a top horizontal bar (full width)
 *   - a right stub covering only the upper half (E has no right stub on lower half)
 *   - a middle horizontal bar (at vertical center)
 *   - a bottom horizontal bar (full width)
 */
static void draw_e(void) {
    int x   = 10;               /* left edge of the E                              */
    int w   = 20;               /* total width of the E                            */
    int mid = (LT+LB)/2-LS/2;  /* y position of the middle bar (vertically centred) */
    fill_rect(x,      LT,    LS,    LB-LT,     L_BLACK); /* left vertical stroke  */
    fill_rect(x,      LT,    w,     LS,         L_BLACK); /* top horizontal bar    */
    fill_rect(x+w-LS, LT,    LS,    mid+LS-LT, L_BLACK); /* upper-right stub      */
    fill_rect(x,      mid,   w,     LS,         L_BLACK); /* middle horizontal bar */
    fill_rect(x,      LB-LS, w,     LS,         L_BLACK); /* bottom horizontal bar */
}

/*
 * draw_s() — draw the letter "S" using filled rectangles.
 *
 * The S consists of:
 *   - a top horizontal bar (full width)
 *   - a left vertical stroke covering only the upper half
 *   - a middle horizontal bar (at vertical center)
 *   - a right vertical stroke covering only the lower half
 *   - a bottom horizontal bar (full width)
 */
static void draw_s(void) {
    int x   = 34;
    int w   = 20;
    int mid = (LT+LB)/2-LS/2;
    fill_rect(x,      LT,    w,     LS,         L_BLACK); /* top horizontal bar    */
    fill_rect(x,      LT,    LS,    mid+LS-LT, L_BLACK); /* upper-left vertical   */
    fill_rect(x,      mid,   w,     LS,         L_BLACK); /* middle horizontal bar */
    fill_rect(x+w-LS, mid,   LS,    LB-mid,    L_BLACK); /* lower-right vertical  */
    fill_rect(x,      LB-LS, w,     LS,         L_BLACK); /* bottom horizontal bar */
}

/*
 * draw_i() — draw the letter "I" using filled rectangles.
 *
 * The I consists of:
 *   - a central vertical bar (6 pixels wide, full letter height)
 *   - a wider top serif
 *   - a wider bottom serif
 * The blue dot above the I is drawn separately by draw_dot().
 */
static void draw_i(void) {
    fill_rect(61, LT,    6,  LB-LT, L_BLACK); /* vertical bar                    */
    fill_rect(58, LT,    12, LS,    L_BLACK); /* top serif (wider than the bar)   */
    fill_rect(58, LB-LS, 12, LS,    L_BLACK); /* bottom serif                    */
}

/*
 * draw_dot() — draw the blue dot above the "i".
 *
 * Positioned at y=3, which is 6 pixels above LT=9, giving a clear visual gap
 * between the dot and the letter body. Color is L_BLUE (the ESI brand blue).
 */
static void draw_dot(void) {
    fill_rect(60, 3, 8, 5, L_BLUE);
}

/*
 * g_pad — global horizontal padding (spaces) added to the left of every logo
 * row so the 80-column canvas is centered in the current terminal width.
 * Computed in show_esi_logo() and used by render_logo() and print_sep().
 */
static int g_pad = 0;

/*
 * rgb_eq(a, b) — return 1 if two RGB colors are identical, 0 otherwise.
 * Used in render_logo() to detect pure-white pixels and handle them specially.
 */
static int rgb_eq(RGB a, RGB b) { return a.r==b.r && a.g==b.g && a.b==b.b; }

/*
 * render_logo() — convert the canvas pixel array to terminal characters and print.
 *
 * TECHNIQUE: Unicode ▀ (U+2580, "UPPER HALF BLOCK") fills the upper half of a
 * character cell. By setting:
 *   foreground color = top canvas pixel
 *   background color = bottom canvas pixel
 * one terminal character represents TWO canvas rows, doubling vertical resolution.
 *
 * Special case: if both pixels are white, we print a plain white-background space
 * instead of ▀ — this avoids rendering artifacts on terminals that interpret
 * foreground white on background white differently.
 *
 * \xe2\x96\x80 is the UTF-8 encoding of ▀ (U+2580).
 */
static void render_logo(void) {
    for (int y = 0; y < CH; y += 2) {
        for (int i = 0; i < g_pad; i++) putchar(' ');  /* left margin padding     */
        for (int x = 0; x < CW; x++) {
            RGB top = canvas[y][x];
            RGB bot = (y+1 < CH) ? canvas[y+1][x] : L_WHITE;
            if (rgb_eq(top, L_WHITE) && rgb_eq(bot, L_WHITE)) {
                printf("\033[48;2;255;255;255m ");      /* plain white space       */
            } else {
                printf("\033[38;2;%d;%d;%dm"            /* foreground = top pixel  */
                       "\033[48;2;%d;%d;%dm"            /* background = bot pixel  */
                       "\xe2\x96\x80",                   /* ▀ character             */
                       top.r, top.g, top.b,
                       bot.r, bot.g, bot.b);
            }
        }
        printf("\033[0m\n");  /* reset colors at end of each terminal row          */
    }
}

/*
 * logo_ms_sleep(ms) — self-contained millisecond sleep used inside the logo
 * typewriter effects. Duplicated from p_sleep() so this section compiles
 * independently if extracted. Uses nanosleep on POSIX, Sleep on Windows.
 */
static void logo_ms_sleep(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#endif
}

/*
 * typewrite_centered(text, tw) — print 'text' centered in terminal width 'tw',
 * one character at a time with a 28ms delay between each character.
 * Used for the "Encadreur" info line below the logo.
 */
static void typewrite_centered(const char *text, int tw) {
    int len = (int)strlen(text);
    int pad = (tw - len) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) putchar(' ');
    printf("\033[38;2;160;160;170m");               /* soft gray color            */
    for (int i = 0; text[i]; i++) {
        putchar(text[i]); fflush(stdout); logo_ms_sleep(28);
    }
    printf("\033[0m\n");
}

/*
 * typewrite(left, right, tw) — print two strings on one line:
 * 'left' at the left margin, 'right' at the right margin, with spaces filling
 * the gap. Both strings are typed character by character.
 * Used for the school name (left) and academic year (right) below the logo.
 */
static void typewrite(const char *left, const char *right, int tw) {
    int ll     = (int)strlen(left);
    int rl     = (int)strlen(right);
    int spaces = tw - 2*g_pad - ll - rl;  /* gap between left and right strings  */
    if (spaces < 2) spaces = 2;
    for (int i = 0; i < g_pad; i++) putchar(' ');   /* left margin                */
    printf("\033[38;2;160;160;170m");
    for (int i = 0; i < ll;     i++) { putchar(left[i]);  fflush(stdout); logo_ms_sleep(28); }
    for (int i = 0; i < spaces; i++) { putchar(' ');       fflush(stdout); logo_ms_sleep(5);  }
    for (int i = 0; i < rl;     i++) { putchar(right[i]); fflush(stdout); logo_ms_sleep(28); }
    printf("\033[0m\n");
}

/*
 * print_sep(tw) — print a full-width horizontal separator made of '-' characters.
 * Inset by g_pad on each side to align with the logo edges.
 */
static void print_sep(int tw) {
    for (int i = 0; i < g_pad; i++) putchar(' ');
    printf("\033[38;2;50;50;65m");                       /* very dark blue-gray    */
    for (int i = 0; i < tw - 2*g_pad; i++) putchar('-');
    printf("\033[0m\n"); fflush(stdout);
}

/*
 * show_esi_logo() — orchestrate the full ESI logo display:
 *   1. Compute centering (horizontal padding and vertical offset).
 *   2. Build the pixel canvas: clear, draw letters, draw dot.
 *   3. Render the canvas to the terminal.
 *   4. Type out the info lines (separator, encadreur, school/year).
 *   5. Show "Press any key to continue..." and block until a key is pressed.
 */
static void show_esi_logo(void) {
    int tw, th;
    p_term_size(&tw, &th);

    /* horizontal centering: the canvas is 80 columns wide */
    g_pad = (tw - CW) / 2;
    if (g_pad < 0) g_pad = 0;

    /* vertical centering: leave 2 rows below for the "press any key" prompt */
    int vpad = (th - TOTAL_ROWS - 2) / 2;
    if (vpad < 1) vpad = 1;

    /* move cursor to the starting row before drawing */
    printf("\033[%d;%dH", vpad, 1);

    /*
     * Info strings printed below the logo.
     * \xc3\x89 = É, \xc3\xa9 = é, \xe2\x80\x94 = — (all UTF-8 encoded)
     */
    static const char school[] =
        "  \xc3\x89""cole Sup\xc3\xa9""rieure d'Informatique  \xe2\x80\x94""  Alger";
    static const char year[] = "2025 / 2026  ";

    /* build and render the pixel-art logo */
    clear_canvas();
    draw_e();
    draw_s();
    draw_i();
    draw_dot();
    render_logo();

    printf("\n");
    print_sep(tw);
    printf("\n");
    typewrite_centered("Encadreur :  Dr. Kermi", tw);
    printf("\n");
    typewrite(school, year, tw);

    /* "Press any key to continue..." prompt — centered */
    printf("\n");
    {
        const char *msg = "Press any key to continue...";
        int len = (int)strlen(msg);
        int pad = (tw - len) / 2;
        if (pad < 0) pad = 0;
        for (int i = 0; i < pad; i++) putchar(' ');
        printf(DIM WHT "%s" RST "\n", msg);
        fflush(stdout);
    }

    p_getch();  /* block until user presses any key */
}


/* =============================================================================
 * FULLSCREEN NOTICE
 *
 * Shown at the very start, before anything else. Asks the user to maximise
 * the terminal window so the animation fits correctly. Waits for a keypress
 * before clearing the screen and proceeding.
 * ============================================================================= */
static void fullscreen_notice(void) {
    int tw, th; p_term_size(&tw, &th);
    cls();
    int m = th / 2 - 4;  /* starting row to vertically centre the ~8-line box */

    /* draw a bordered box using box-drawing characters, each line centred */
    cprint(m,     C8 BLD,  "\xe2\x94\x8c\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x98");
    cprint(m + 1, C8 BLD,  "│                                                  │");
    cprint(m + 2, C7 BLD,  "│    ⚠  PLEASE PUT YOUR TERMINAL IN FULLSCREEN  ⚠  │");
    cprint(m + 3, C8 BLD,  "│                                                  │");
    cprint(m + 4, C9,      "│          Minimum recommended: 120 x 35           │");
    cprint(m + 5, C8 BLD,  "│                                                  │");
    cprint(m + 6, C8 BLD,  "└──────────────────────────────────────────────────┘");
    cprint(m + 9, DIM WHT, "Press any key to continue...");
    fflush(stdout);
    p_getch();
    cls();  /* clear screen so the logo starts clean */
}


/* =============================================================================
 * TP1 — BIG BLOCK LETTERS
 *
 * Displays "TP1" in 6-row ASCII block art with the blue-to-cyan gradient.
 * Each row gets a progressively brighter shade from the gradient.
 * ============================================================================= */
static void show_tp1(void) {
    int tw, th; p_term_size(&tw, &th);

    /* 6-row ASCII block art spelling "TP1" */
    const char *tp[] = {
        "\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x95\x97 \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x95\x97   \xe2\x96\x88\xe2\x96\x88\xe2\x95\x97",
        "\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x90\xe2\x96\x88\xe2\x95\x94\xe2\x95\x90\xe2\x95\x90\xe2\x95\x9d \xe2\x96\x88\xe2\x96\x88\xe2\x95\x94\xe2\x95\x90\xe2\x95\x90\xe2\x96\x88\xe2\x96\x88\xe2\x95\x97 \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x95\x91",
        "   \xe2\x96\x88\xe2\x96\x88\xe2\x95\x91    \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x95\x94\xe2\x95\x9d \xe2\x95\x9a\xe2\x96\x88\xe2\x96\x88\xe2\x95\x91",
        "   \xe2\x96\x88\xe2\x96\x88\xe2\x95\x91    \xe2\x96\x88\xe2\x96\x88\xe2\x95\x94\xe2\x95\x90\xe2\x95\x90\xe2\x95\x90\xe2\x95\x9d   \xe2\x96\x88\xe2\x96\x88\xe2\x95\x91",
        "   \xe2\x96\x88\xe2\x96\x88\xe2\x95\x91    \xe2\x96\x88\xe2\x96\x88\xe2\x95\x91       \xe2\x96\x88\xe2\x96\x88\xe2\x95\x91",
        "   \xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d    \xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d       \xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d",
    };

    /* one gradient color per row: dark navy at top → bright cyan at bottom */
    const char *fc[] = { C2, C4, C6, C8, C10, C12 };

    /* slightly above vertical center */
    int row = (th - 6) / 2 - 3;
    if (row < 2) row = 2;

    for (int i = 0; i < 6; i++)
        type_at(row + i, fc[i], tp[i], 10);  /* 10ms per character */

    fl();
    p_sleep(1000);  /* pause 1 second so the title can be read */
}


/* =============================================================================
 * "ACCOMPLISHED BY" HEADING
 *
 * 3-row ASCII art heading displayed below the TP1 title.
 * ============================================================================= */
static void show_accomplished(void) {
    int tw, th; p_term_size(&tw, &th);

    const char *l[] = {
        "\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\x94\xe2\x95\xa6\xe2\x95\x97\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\xa6  \xe2\x95\xa6\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\xa6 \xe2\x95\xa6\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\x94\xe2\x95\xa6\xe2\x95\x97  \xe2\x95\x94\xe2\x95\x97 \xe2\x95\xa6 \xe2\x95\xa6",
        "\xe2\x95\xa0\xe2\x95\x90\xe2\x95\xa3\xe2\x95\x91  \xe2\x95\x91\xe2\x95\x91  \xe2\x95\x94\xe2\x95\x90\xe2\x95\x9d\xe2\x95\x91 \xe2\x95\xa9\xe2\x95\x94\xe2\x95\x90\xe2\x95\xa3\xe2\x95\x91  \xe2\x95\x91\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x97\xe2\x95\xa0\xe2\x95\x90\xe2\x95\xa3\xe2\x95\xa0\xe2\x95\x90\xe2\x95\xa3\xe2\x95\x94\xe2\x95\x90\xe2\x95\x9d  \xe2\x95\xa0\xe2\x95\xa9\xe2\x95\x97\xe2\x95\x9a\xe2\x95\xa6\xe2\x95\x97",
        "\xe2\x95\xa9 \xe2\x95\xa9\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d\xe2\x95\xa9 \xe2\x95\xa9\xe2\x95\xa9  \xe2\x95\xa9\xe2\x95\x90\xe2\x95\x9d\xe2\x95\xa9\xe2\x95\x90\xe2\x95\x9d\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d \xe2\x95\xa9  \xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d \xe2\x95\xa9 ",
    };

    /* brighter range of the gradient */
    const char *fc[] = { C7, C9, C11 };

    /* positioned just under the TP1 title */
    int row = (th - 6) / 2 + 4;

    for (int i = 0; i < 3; i++)
        type_at(row + i, fc[i], l[i], 7);  /* 7ms per character */

    fl();
    p_sleep(600);
}


/* =============================================================================
 * AUTHOR NAMES — FLASH REVEAL
 *
 * Builds "H.ABDELMOUMENE  &  B.TALBI" as combined ASCII art strings, then
 * uses a flash-reveal effect: the names start near-black and rapidly brighten
 * to full color over 6 steps.
 * ============================================================================= */
static void show_names(void) {
    int tw, th; p_term_size(&tw, &th);

    /* 3-row ASCII art for each name */
    const char *h1 = "\xe2\x95\xa6 \xe2\x95\xa6  \xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\x94\xe2\x95\x97 \xe2\x95\x94\xe2\x95\xa6\xe2\x95\x97\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\xa6  \xe2\x95\x94\xe2\x95\xa6\xe2\x95\x97\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\xa6 \xe2\x95\xa6\xe2\x95\x94\xe2\x95\xa6\xe2\x95\x97\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\x94\xe2\x95\x97 \xe2\x95\x97\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97";
    const char *h2 = "\xe2\x95\xa0\xe2\x95\x90\xe2\x95\xa3  \xe2\x95\xa0\xe2\x95\x90\xe2\x95\xa3\xe2\x95\xa0\xe2\x95\xa9\xe2\x95\x97 \xe2\x95\x91\xe2\x95\x94\xe2\x95\xa3\xe2\x95\xa0\xe2\x95\x90\xe2\x95\x9d\xe2\x95\x91  \xe2\x95\x91\xe2\x95\x94\xe2\x95\xa6\xe2\x95\x97\xe2\x95\x94\xe2\x95\x90\xe2\x95\x9d\xe2\x95\xa6 \xe2\x95\xa6\xe2\x95\x94\xe2\x95\x90\xe2\x95\xa3\xe2\x95\xa0\xe2\x95\x90\xe2\x95\xa3\xe2\x95\x94\xe2\x95\x90\xe2\x95\x9d  \xe2\x95\xa0\xe2\x95\xa9\xe2\x95\x97\xe2\x95\x94\xe2\x95\x90\xe2\x95\x9d";
    const char *h3 = "\xe2\x95\xa9 \xe2\x95\xa9\xe2\x97\x8f \xe2\x95\xa9 \xe2\x95\xa9\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d\xe2\x95\x90\xe2\x95\xa9\xe2\x95\x9d\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d\xe2\x95\xa9\xe2\x95\x90\xe2\x95\x9d\xe2\x95\xa9 \xe2\x95\xa9\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d\xe2\x95\xa9 \xe2\x95\xa9\xe2\x95\xa9 \xe2\x95\xa9\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d\xe2\x95\x9d\xe2\x95\x9a\xe2\x95\x9d\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d";

    const char *b1 = "\xe2\x95\x94\xe2\x95\x97  \xe2\x95\x94\xe2\x95\xa6\xe2\x95\x97\xe2\x95\x94\xe2\x95\x90\xe2\x95\x97\xe2\x95\xa6  \xe2\x95\x94\xe2\x95\x97 \xe2\x95\xa6";
    const char *b2 = "\xe2\x95\xa0\xe2\x95\xa9\xe2\x95\x97  \xe2\x95\x91\xe2\x95\x94\xe2\x95\x90\xe2\x95\xa3\xe2\x95\xa6  \xe2\x95\x91\xe2\x95\xa0\xe2\x95\xa9\xe2\x95\x97\xe2\x95\x91";
    const char *b3 = "\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d\xe2\x97\x8f \xe2\x95\xa9 \xe2\x95\xa9 \xe2\x95\xa9\xe2\x95\xa9\xe2\x95\x90\xe2\x95\x9d\xe2\x95\x9a\xe2\x95\x90\xe2\x95\x9d\xe2\x95\xa9";

    /* merge both names into one string per row with " & " between them */
    char f1[256], f2[256], f3[256];
    snprintf(f1, sizeof(f1), "%s    &    %s", h1, b1);
    snprintf(f2, sizeof(f2), "%s    &    %s", h2, b2);
    snprintf(f3, sizeof(f3), "%s    &    %s", h3, b3);

    /* positioned below the "ACCOMPLISHED BY" heading */
    int row = (th - 6) / 2 + 9;

    /*
     * Flash-reveal: overwrite the same 3 rows 6 times with increasing brightness.
     * Each step adds ~40ms, so the full flash takes about 240ms.
     */
    const char *rv[] = { C1, C3, C5, C7, C9, C11 };
    for (int s = 0; s < 6; s++) {
        cprint(row,     rv[s], f1);
        cprint(row + 1, rv[s], f2);
        cprint(row + 2, rv[s], f3);
        fl();
        p_sleep(40);
    }

    /* final bold render at full brightness — hold for 500ms */
    cprint(row,     C10 BLD, f1);
    cprint(row + 1, C11 BLD, f2);
    cprint(row + 2, C12 BLD, f3);
    fl();
    p_sleep(500);
}


/* =============================================================================
 * SEPARATOR — GROWS FROM CENTER OUTWARD
 *
 * A horizontal line of '─' characters that expands left and right simultaneously
 * from the center. Each character's color depends on its distance from center:
 * brightest (near-white) at the center, darkest (navy) at the edges.
 * ============================================================================= */
static void show_separator(void) {
    int tw, th; p_term_size(&tw, &th);

    /* row just below the names section */
    int row    = (th - 6) / 2 + 14;
    int w      = 70;              /* total width of the separator line             */
    int col    = (tw - w) / 2;   /* left column to centre the 70-char line        */
    if (col < 0) col = 0;
    int center = w / 2;           /* index of the centre character within the line */

    for (int half = 1; half <= center + 1; half++) {
        /*
         * Each iteration extends the visible portion by 1 on each side.
         * We move the cursor back to the line start every frame and overwrite
         * the entire row so there are no leftover characters from the previous frame.
         */
        cur(row, col);
        for (int x = 0; x < w; x++) {
            int d = abs(x - center);   /* distance from the centre                */
            if (d <= half) {
                /* 3-zone gradient based on distance from centre */
                if      (d < half / 3)     printf(C12);  /* inner: near-white     */
                else if (d < 2 * half / 3) printf(C8);   /* middle: blue-cyan     */
                else                        printf(C4);   /* outer: dark navy      */
                printf("\xe2\x94\x80");  /* UTF-8 for '─' (U+2500, box drawings)  */
            } else {
                printf(" ");  /* unrevealed portion: blank space                   */
            }
        }
        printf(RST); fl();
        p_sleep(6);  /* 6ms per frame → full 70-char line grows in ~210ms          */
    }
}


/* =============================================================================
 * animation() — PUBLIC ENTRY POINT
 *
 * This is the only function exported by this module (declared in intro.h).
 * It runs the complete intro sequence in order:
 *   0. p_init()           — enable ANSI on Windows; no-op on Linux
 *   1. HCUR               — hide the cursor to prevent flicker
 *   2. fullscreen_notice  — warn the user to maximise the terminal
 *   3. show_esi_logo      — render the pixel-art ESI logo with info lines
 *   4. cls + sleep        — clear screen with a short pause before TP1
 *   5. show_tp1           — animate the "TP1" block title
 *   6. show_accomplished  — animate the "ACCOMPLISHED BY" heading
 *   7. show_names         — flash-reveal the author names
 *   8. show_separator     — grow the animated separator line
 *   9. footer + prompt    — show school/year footer and "press any key" prompt
 *  10. SCUR + p_getch     — restore cursor and wait for final keypress
 *  11. cls + RST          — clean up the terminal before returning to main
 * ============================================================================= */
void animation(void) {
    p_init();            /* platform setup (ANSI enable on Windows)               */
    printf(HCUR); fl();  /* hide blinking cursor during animation                 */

    fullscreen_notice(); /* step 0: fullscreen warning — cleared at end of call   */
    show_esi_logo();     /* step 1: ESI pixel-art logo + info lines               */

    cls();               /* clear before TP1 animation                            */
    p_sleep(300);        /* 300ms pause for a clean visual transition             */

    show_tp1();          /* step 2: "TP1" large block art                         */
    show_accomplished(); /* step 3: "ACCOMPLISHED BY" heading                     */
    show_names();        /* step 4: flash-reveal author names                     */
    show_separator();    /* step 5: animated separator growing from centre        */

    /* footer: school/year below the separator, "press any key" at screen bottom */
    {
        int tw, th; p_term_size(&tw, &th);
        cprint((th - 6) / 2 + 16, DIM GR1, "ESI Alger  ·  2025/2026");
        cprint(th - 2,             DIM WHT, "Press any key to exit...");
    }

    fl();
    printf(SCUR); fl();  /* restore cursor visibility before blocking on input   */
    p_getch();           /* wait for final keypress                               */
    cls();               /* clear screen so the program's own UI starts cleanly  */
    cur(0, 0);           /* move cursor to top-left                               */
    printf(RST); fl();   /* reset all terminal styling                            */
}
