#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>

/* ═════════════════════════════════════════════════════════════════════════════
 *  CROSS-PLATFORM LAYER
 * ═════════════════════════════════════════════════════════════════════════════ */
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>

    void p_sleep(int ms) { Sleep(ms); }

    void p_init(void) {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD m = 0;
        GetConsoleMode(h, &m);
        SetConsoleMode(h, m | 0x0004);
        SetConsoleOutputCP(65001);
    }

    int p_getch(void) { return _getch(); }

    void p_term_size(int *w, int *h) {
        CONSOLE_SCREEN_BUFFER_INFO c;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &c)) {
            *w = c.srWindow.Right  - c.srWindow.Left + 1;
            *h = c.srWindow.Bottom - c.srWindow.Top  + 1;
        } else { *w = 120; *h = 40; }
    }
#else
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>

    /* nanosleep is POSIX.1b and reliably declared with _POSIX_C_SOURCE 199309L.
       Using it avoids the obsolete usleep() warning. */
    void p_sleep(int ms) {
        struct timespec ts;
        ts.tv_sec  = ms / 1000;
        ts.tv_nsec = (ms % 1000) * 1000000L;
        nanosleep(&ts, NULL);
    }

    void p_init(void) {}

    int p_getch(void) {
        struct termios o, n;
        int c;
        tcgetattr(STDIN_FILENO, &o);
        n = o;
        n.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &n);
        c = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &o);
        return c;
    }

    void p_term_size(int *w, int *h) {
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
            *w = ws.ws_col;
            *h = ws.ws_row;
        } else { *w = 120; *h = 40; }
    }
#endif

/* ═════════════════════════════════════════════════════════════════════════════
 *  ANSI ESCAPE MACROS
 * ═════════════════════════════════════════════════════════════════════════════ */
#define RST  "\033[0m"
#define BLD  "\033[1m"
#define DIM  "\033[2m"
#define HCUR "\033[?25l"
#define SCUR "\033[?25h"

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

#define WHT "\033[38;2;255;255;255m"
#define GR1 "\033[38;2;80;80;80m"

/* ═════════════════════════════════════════════════════════════════════════════
 *  UTILITY FUNCTIONS
 * ═════════════════════════════════════════════════════════════════════════════ */
static void cls(void)  { printf("\033[2J\033[H"); fflush(stdout); }
static void fl(void)   { fflush(stdout); }

static void cur(int r, int c) { printf("\033[%d;%dH", r + 1, c + 1); fl(); }

/* Count visible characters, skipping ANSI escapes and UTF-8 continuation bytes. */
static int vis_len(const char *s) {
    int len = 0;
    for (int i = 0; s[i]; i++) {
        if (s[i] == '\033') {
            while (s[i] && s[i] != 'm') i++;
            continue;
        }
        unsigned char c = (unsigned char)s[i];
        if (c < 0x80 || c >= 0xC0) len++;
    }
    return len;
}

static void cprint(int row, const char *color, const char *text) {
    int tw, th; p_term_size(&tw, &th);
    int col = (tw - vis_len(text)) / 2;
    if (col < 0) col = 0;
    cur(row, col);
    printf("%s%s" RST, color, text);
    fl();
}

static void type_at(int row, const char *color, const char *text, int delay) {
    int tw, th; p_term_size(&tw, &th);
    int col = (tw - vis_len(text)) / 2;
    if (col < 0) col = 0;
    cur(row, col);
    printf("%s", color);
    for (int i = 0; text[i]; i++) {
        if (text[i] == '\033') {
            while (text[i] && text[i] != 'm') { putchar(text[i]); i++; }
            if (text[i]) putchar(text[i]);
            fl(); continue;
        }
        putchar(text[i]); fl();
        if (text[i] != ' ' && text[i] != '\n') p_sleep(delay);
    }
    printf(RST); fl();
}

/* ═════════════════════════════════════════════════════════════════════════════
 *  ESI LOGO  (80×42 virtual canvas → 21 terminal rows via half-blocks)
 * ═════════════════════════════════════════════════════════════════════════════ */
#define CW      80
#define CH      42
#define TROWS   (CH / 2)
#define INFO_ROWS 4
#define TOTAL_ROWS (TROWS + INFO_ROWS)

typedef struct { unsigned char r, g, b; } RGB;

static RGB canvas[CH][CW];

static const RGB L_WHITE = {255, 255, 255};
static const RGB L_BLACK = {  0,   0,   0};
static const RGB L_BLUE  = { 30, 105, 180};

static void clear_canvas(void) {
    for (int y = 0; y < CH; y++)
        for (int x = 0; x < CW; x++)
            canvas[y][x] = L_WHITE;
}

static void fill_rect(int x0, int y0, int w, int h, RGB c) {
    for (int y = y0; y < y0 + h && y < CH; y++)
        for (int x = x0; x < x0 + w && x < CW; x++)
            if (x >= 0 && y >= 0)
                canvas[y][x] = c;
}

#define LS  5
#define LT  9
#define LB  35

static void draw_e(void) {
    int x = 10, w = 20;
    int mid = (LT + LB) / 2 - LS / 2;
    fill_rect(x,      LT,    LS,    LB - LT,     L_BLACK);
    fill_rect(x,      LT,    w,     LS,          L_BLACK);
    fill_rect(x + w - LS, LT,    LS,    mid + LS - LT, L_BLACK);
    fill_rect(x,      mid,   w,     LS,          L_BLACK);
    fill_rect(x,      LB - LS, w,     LS,          L_BLACK);
}

static void draw_s(void) {
    int x = 34, w = 20;
    int mid = (LT + LB) / 2 - LS / 2;
    fill_rect(x,      LT,    w,     LS,          L_BLACK);
    fill_rect(x,      LT,    LS,    mid + LS - LT, L_BLACK);
    fill_rect(x,      mid,   w,     LS,          L_BLACK);
    fill_rect(x + w - LS, mid,   LS,    LB - mid,      L_BLACK);
    fill_rect(x,      LB - LS, w,     LS,          L_BLACK);
}

static void draw_i(void) {
    fill_rect(61, LT,    6,  LB - LT, L_BLACK);
    fill_rect(58, LT,    12, LS,      L_BLACK);
    fill_rect(58, LB - LS, 12, LS,      L_BLACK);
}

static void draw_dot(void) {
    fill_rect(60, 3, 8, 5, L_BLUE);
}

static int g_pad = 0;

static int rgb_eq(RGB a, RGB b) { return a.r == b.r && a.g == b.g && a.b == b.b; }

/* Render canvas using Unicode U+2580 (▀): foreground = top pixel, background = bottom pixel.
   This doubles the vertical resolution in the terminal. */
static void render_logo(void) {
    for (int y = 0; y < CH; y += 2) {
        for (int i = 0; i < g_pad; i++) putchar(' ');
        for (int x = 0; x < CW; x++) {
            RGB top = canvas[y][x];
            RGB bot = (y + 1 < CH) ? canvas[y + 1][x] : L_WHITE;
            if (rgb_eq(top, L_WHITE) && rgb_eq(bot, L_WHITE)) {
                printf("\033[48;2;255;255;255m ");
            } else {
                printf("\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm\xe2\x96\x80",
                       top.r, top.g, top.b, bot.r, bot.g, bot.b);
            }
        }
        printf("\033[0m\n");
    }
}

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

static void typewrite_centered(const char *text, int tw) {
    int len = (int)strlen(text);
    int pad = (tw - len) / 2;
    if (pad < 0) pad = 0;
    for (int i = 0; i < pad; i++) putchar(' ');
    printf("\033[38;2;160;160;170m");
    for (int i = 0; text[i]; i++) {
        putchar(text[i]); fflush(stdout); logo_ms_sleep(28);
    }
    printf("\033[0m\n");
}

static void typewrite(const char *left, const char *right, int tw) {
    int ll = (int)strlen(left);
    int rl = (int)strlen(right);
    int spaces = tw - 2 * g_pad - ll - rl;
    if (spaces < 2) spaces = 2;

    for (int i = 0; i < g_pad; i++) putchar(' ');
    printf("\033[38;2;160;160;170m");
    for (int i = 0; i < ll; i++) { putchar(left[i]);  fflush(stdout); logo_ms_sleep(28); }
    for (int i = 0; i < spaces; i++) { putchar(' ');  fflush(stdout); logo_ms_sleep(5);  }
    for (int i = 0; i < rl; i++) { putchar(right[i]); fflush(stdout); logo_ms_sleep(28); }
    printf("\033[0m\n");
}

static void print_sep(int tw) {
    for (int i = 0; i < g_pad; i++) putchar(' ');
    printf("\033[38;2;50;50;65m");
    for (int i = 0; i < tw - 2 * g_pad; i++) putchar('-');
    printf("\033[0m\n"); fflush(stdout);
}

static void show_esi_logo(void) {
    int tw, th;
    p_term_size(&tw, &th);

    g_pad = (tw - CW) / 2;
    if (g_pad < 0) g_pad = 0;

    int vpad = (th - TOTAL_ROWS - 2) / 2;
    if (vpad < 1) vpad = 1;

    printf("\033[%d;%dH", vpad, 1);

    static const char school[] =
        "  \xc3\x89""cole Sup\xc3\xa9""rieure d'Informatique  \xe2\x80\x94""  Alger";
    static const char year[] = "2025 / 2026  ";

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

    p_getch();
}

static void fullscreen_notice(void) {
    int tw, th; p_term_size(&tw, &th);
    cls();
    int m = th / 2 - 4;
    cprint(m,     C8 BLD,  "┌──────────────────────────────────────────────────┐");
    cprint(m + 1, C8 BLD,  "│                                                  │");
    cprint(m + 2, C7 BLD,  "│    ⚠  PLEASE PUT YOUR TERMINAL IN FULLSCREEN  ⚠  │");
    cprint(m + 3, C8 BLD,  "│                                                  │");
    cprint(m + 4, C9,      "│          Minimum recommended: 120 x 35           │");
    cprint(m + 5, C8 BLD,  "│                                                  │");
    cprint(m + 6, C8 BLD,  "└──────────────────────────────────────────────────┘");
    cprint(m + 9, DIM WHT, "Press any key to continue...");
    fflush(stdout);
    p_getch();
    cls();
}

static void show_tp1(void) {
    int tw, th; p_term_size(&tw, &th);

    const char *tp[] = {
        "████████╗ ██████╗ ",
        "╚══██╔══╝ ██╔══██╗",
        "   ██║    ██████╔╝",
        "   ██║    ██╔═══╝ ",
        "   ██║    ██║     ",
        "   ╚═╝    ╚═╝     ",
    };
    const char *fc[] = { C2, C4, C6, C8, C10, C12 };
    int row = (th - 6) / 2 - 3;
    if (row < 2) row = 2;

    for (int i = 0; i < 6; i++)
        type_at(row + i, fc[i], tp[i], 10);

    fl();
    p_sleep(1000);
}

static void show_accomplished(void) {
    int tw, th; p_term_size(&tw, &th);

    const char *l[] = {
        "╔═╗╔═╗╔═╗╔═╗╔╦╗╔═╗╦  ╦╔═╗╦ ╦╔═╗╔╦╗  ╔╗ ╦ ╦",
        "╠═╣║  ║  ║ ║║║║╠═╝║  ║╚═╗╠═╣║╣  ║║  ╠╩╗╚╦╝",
        "╩ ╩╚═╝╚═╝╚═╝╩ ╩╩  ╩═╝╩╚═╝╩ ╩╚═╝═╩╝  ╚═╝ ╩ ",
    };
    const char *fc[] = { C7, C9, C11 };
    int row = (th - 6) / 2 + 4;

    for (int i = 0; i < 3; i++)
        type_at(row + i, fc[i], l[i], 7);

    fl();
    p_sleep(600);
}

static void show_names(void) {
    int tw, th; p_term_size(&tw, &th);

    const char *h1 = "╦ ╦  ╔═╗╔╗ ╔╦╗╔═╗╦  ╔╦╗╔═╗╦ ╦╔╦╗╔═╗╔╗╔╔═╗";
    const char *h2 = "╠═╣  ╠═╣╠╩╗ ║║║╣ ║  ║║║║ ║║ ║║║║║╣ ║║║║╣ ";
    const char *h3 = "╩ ╩● ╩ ╩╚═╝═╩╝╚═╝╩═╝╩ ╩╚═╝╚═╝╩ ╩╚═╝╝╚╝╚═╝";

    const char *b1 = "╔╗  ╔╦╗╔═╗╦  ╔╗ ╦";
    const char *b2 = "╠╩╗  ║ ╠═╣║  ╠╩╗║";
    const char *b3 = "╚═╝● ╩ ╩ ╩╩═╝╚═╝╩";

    char f1[256], f2[256], f3[256];
    snprintf(f1, sizeof(f1), "%s    &    %s", h1, b1);
    snprintf(f2, sizeof(f2), "%s    &    %s", h2, b2);
    snprintf(f3, sizeof(f3), "%s    &    %s", h3, b3);

    int row = (th - 6) / 2 + 9;

    const char *rv[] = { C1, C3, C5, C7, C9, C11 };
    for (int s = 0; s < 6; s++) {
        cprint(row,     rv[s], f1);
        cprint(row + 1, rv[s], f2);
        cprint(row + 2, rv[s], f3);
        fl();
        p_sleep(40);
    }

    cprint(row,     C10 BLD, f1);
    cprint(row + 1, C11 BLD, f2);
    cprint(row + 2, C12 BLD, f3);
    fl();
    p_sleep(500);
}

static void show_separator(void) {
    int tw, th; p_term_size(&tw, &th);
    int row   = (th - 6) / 2 + 14;
    int w     = 70;
    int col   = (tw - w) / 2;
    if (col < 0) col = 0;
    int center = w / 2;

    for (int half = 1; half <= center + 1; half++) {
        cur(row, col);
        for (int x = 0; x < w; x++) {
            int d = abs(x - center);
            if (d <= half) {
                if      (d < half / 3)     printf(C12);
                else if (d < 2 * half / 3) printf(C8);
                else                        printf(C4);
                printf("─");
            } else {
                printf(" ");
            }
        }
        printf(RST); fl();
        p_sleep(6);
    }
}



static void show_menu(void) {
    int tw, th; p_term_size(&tw, &th);
    cls();
    int m = th / 2 - 4;
    cprint(m ,     C8 BLD,  "╔═════════════════════════════════════════════════╦════════════════════════════════════════════════╗");
    cprint(m + 2 , C7 BLD,  "║                                                 ║                                                ║");
    cprint(m + 3 , C8 BLD,  "║             ███████╗  ███████╗  ██╗             ║                                                ║");
    cprint(m + 4 , C9,      "║             ██╔════╝  ██╔════╝  ██║             ║                                                ║");
    cprint(m + 5 , C8 BLD,  "║             █████╗    ███████╗  ██║             ║                                                ║");
    cprint(m + 6 , C8 BLD,  "║             ██╔══╝    ╚════██║  ██║             ║                                                ║");
    cprint(m + 7 , C8 BLD,  "║             ███████╗  ███████║  ██║             ║                                                ║");
    cprint(m + 8 , C8 BLD,  "║             ╚══════╝  ╚══════╝  ╚═╝             ║                                                ║");
    cprint(m + 9 , C8 BLD,  "║                                                 ║                                                ║");
    cprint(m + 10, C8 BLD,  "║                                                 ║                                                ║");
    cprint(m + 11, C8 BLD,  "║       █████╗   ██████╗  ██████╗  ███████╗       ║                                                ║");
    cprint(m + 12, C8 BLD,  "║      ██╔══██╗  ██╔══██╗ ██╔══██╗ ██╔════╝       ║                                                ║");
    cprint(m + 13, C8 BLD,  "║      ███████║  ██║  ██║ ██║  ██║ ███████╗       ║                                                ║");
    cprint(m + 14, C8 BLD,  "║      ██╔══██║  ██║  ██║ ██║  ██║  ╚═══██║       ║                                                ║");
    cprint(m + 15, C8 BLD,  "║      ██║  ██║  ██████╔╝ ██████╔╝ ███████║       ║                                                ║");
    cprint(m + 16, C8 BLD,  "║      ╚═╝  ╚═╝  ╚═════╝  ╚═════╝  ╚══════╝       ║                                                ║");
    cprint(m + 17, C8 BLD,  "║                                                 ║                                                ║");
    cprint(m + 18, C8 BLD,  "║                                                 ║                                                ║");
    cprint(m + 19, C8 BLD,  "║               ████████╗ ██████╗                 ║                                                ║");
    cprint(m + 20, C8 BLD,  "║               ╚══██╔══╝ ██╔══██╗                ║                                                ║");
    cprint(m + 21, C8 BLD,  "║                  ██║    ██████╔╝                ║                                                ║");
    cprint(m + 22, C8 BLD,  "║                  ██║    ██╔═══╝                 ║                                                ║");
    cprint(m + 23, C8 BLD,  "║                  ██║    ██║                     ║                                                ║");
    cprint(m + 24, C8 BLD,  "║                  ╚═╝    ╚═╝                     ║                                                ║");
    cprint(m + 25, C8 BLD,  "║                                                 ║                                                ║");
    cprint(m + 26, C8 BLD,  "╚═════════════════════════════════════════════════╩════════════════════════════════════════════════╝");

    cprint(m + 29, DIM WHT, "Press any key to continue...");
    fflush(stdout);
    p_getch();
    cls();
}


void animation(void) {
    p_init();
    printf(HCUR); fl();

    fullscreen_notice();

    show_esi_logo();

    cls();
    p_sleep(300);

    show_tp1();
    show_accomplished();
    show_names();
    show_separator();

    {
        int tw, th; p_term_size(&tw, &th);
        cprint((th - 6) / 2 + 16, DIM GR1, "ESI Alger  ·  2025/2026");
        cprint(th - 2,             DIM WHT, "Press any key to exit...");
    }

    fl();
    printf(SCUR); fl();
    p_getch();
    cls();
    cur(0, 0);
    printf(RST); fl();
}
