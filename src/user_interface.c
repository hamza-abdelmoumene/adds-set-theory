/* ═════════════════════════════════════════════════════════════════════════════
 *  ADDS – Set Theory Text Analyser  |  user_interface.c
 *  Platform : POSIX (Linux / macOS) + Windows
 *  Encoding : UTF-8                 Date: 2025 / 2026
 * ═════════════════════════════════════════════════════════════════════════════ */

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include "../include/file_list.h"
#include "../include/sets_ops.h"
#include "../include/utils.h"
#include "../include/file_list.h"
#include "../include/word_bst.h"
#include "../include/sentence_ll.h"
#include "../include/paragraph_ll.h"
#include "../include/file_parser.h"

/* ─────────────────────────────────────────────────────────────────────────────
 *  BOX LAYOUT
 *
 *  Total width  = 100 visible columns (centred in the terminal).
 *  Left  panel  = 49 visible cols  (║ ... ║)
 *  Right panel  = 48 visible cols  (║ ... ║)
 *  Borders      =  3 cols
 *  49 + 48 + 3  = 100 ✓
 *
 *  DrawRow() HARD-CLAMPS the right content to RIGHT_W visible chars.
 *  Nothing can ever bleed past the closing ║.
 * ───────────────────────────────────────────────────────────────────────────── */
#define BOX_W 100
#define LEFT_W 49
#define RIGHT_W 48
#define RSLOTS 24

/* Compute the left padding needed to center the UI box. */
static int BoxColumn(void)
{
    int w, h;
    GetTerminalSize(&w, &h);
    int p = (w - BOX_W) / 2;
    return (p < 0) ? 0 : p;
}

/* Draw a full-width border line with the box centered. */
static void DrawBorder(const char *s)
{
    PadSpaces(BoxColumn());
    printf("%s\033[K\n", s);
}

/* Render a two-column row with optional left color and hard-clamped right text. */
static void DrawRow(const char *lc, const char *l, const char *r)
{
    /* hard-clamp right content — this is the single fix for bound crossing */
    char rbuf[512];
    const char *rc = TruncateVisible(r, RIGHT_W, rbuf, (int)sizeof(rbuf));

    int lpad = LEFT_W - VisibleLength(l);
    if (lpad < 0)
        lpad = 0;
    int rpad = RIGHT_W - VisibleLength(rc);
    if (rpad < 0)
        rpad = 0;

    PadSpaces(BoxColumn());
    printf(C7 "║" RST); /* ║ */
    if (lc)
        printf("%s", lc);
    printf("%s", l);
    if (lc)
        printf(RST);
    PadSpaces(lpad);
    printf(C7 "║" RST); /* ║ */
    printf("%s", rc);
    PadSpaces(rpad);
    printf(C7 "║" RST "\033[K\n"); /* ║ */
}

/* Draw a full-width row for result/dialog screens that should not show art. */
static void DrawFullRow(const char *content)
{
    enum { FULL_W = BOX_W - 2 };
    char buf[1024];
    const char *line = TruncateVisible(content ? content : "", FULL_W, buf, (int)sizeof(buf));
    int pad = FULL_W - VisibleLength(line);
    if (pad < 0)
        pad = 0;

    PadSpaces(BoxColumn());
    printf(C7 "║" RST "%s", line);
    PadSpaces(pad);
    printf(C7 "║" RST "\033[K\n");
}

/* Draw a simple full-width blue rectangle. */
static void DrawFullBox(const char *slots[RSLOTS])
{
    PadSpaces(BoxColumn());
    printf(C7 "╔");
    for (int i = 0; i < BOX_W - 2; i++)
        printf("═");
    printf("╗" RST "\033[K\n");

    for (int i = 0; i < RSLOTS; i++)
        DrawFullRow(slots[i]);

    PadSpaces(BoxColumn());
    printf(C7 "╚");
    for (int i = 0; i < BOX_W - 2; i++)
        printf("═");
    printf("╝" RST "\033[K\n");
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  STATUS BAR  (separator + gradient footer)
 * ───────────────────────────────────────────────────────────────────────────── */
/* Animate the horizontal separator line at the bottom. */
static void DrawSeparator(void)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    int w = tw, center = w / 2;

    for (int half = 1; half <= center + 1; half++)
    {
        MoveCursor(th - 3, 0);
        for (int x = 0; x < w; x++)
        {
            int d = abs(x - center);
            if (d <= half)
            {
                if (d < half / 3)
                    printf(C12);
                else if (d < 2 * half / 3)
                    printf(C8);
                else
                    printf(C4);
                printf("─"); /* ─ */
            }
            else
                printf(" ");
        }
        printf(RST "\033[K");
        FlushOutput();
        SleepMillis(2);
    }
}

/* Render the footer with a left/right gradient label. */
static void DrawFooter(void)
{
    int tw, th;
    GetTerminalSize(&tw, &th);

    const char *text = "ESI-2025/2026    ABDELMOUMENE Hamza  &  TALBI Baghdad";
    int len = VisibleLength(text);
    int start = (tw - len) / 2;
    if (start < 0)
        start = 0;

    static const char *fg[] = {
        C4, C5, C6, C7, C8, C9, C10, C11, C12
    };
    int fgN = (int)(sizeof(fg) / sizeof(fg[0])) - 1;

    MoveCursor(th - 2, 0);
    PadSpaces(start);
    for (int i = 0; text[i]; i++)
    {
        int gi = (len > 1) ? i * fgN / (len - 1) : 0;
        printf("%s%c" RST, fg[gi < fgN ? gi : fgN], text[i]);
    }
    printf("\033[K\n");
    FlushOutput();
}

/* Draw the separator and footer as a single status bar unit. */
static void DrawStatusBar(void)
{
    DrawSeparator();
    DrawFooter();
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  BREADCRUMB
 * ───────────────────────────────────────────────────────────────────────────── */
/* Print the breadcrumb label at the top of the UI. */
static void DrawBreadcrumb(const char *crumb)
{
    MoveCursor(0, 0);
    PadSpaces(BoxColumn());
    /* ›  U+203A = › */
    printf(C4 " ›  " C7 "%s" RST "\033[K\n", crumb);
    FlushOutput();
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  KEYBOARD READER
 *  Returns: 'U'=up  'D'=down  '\r'=enter  '1'..'9'=direct  27=ESC  0=ignored
 *
 *  ESC handling: read first byte. If a second byte arrives within ~100ms and
 *  is '[', it's an escape sequence. If no second byte - bare ESC -> return 27.
 *  This is reliable because ReadCharNonBlocking() uses VTIME=1 (100ms timeout).
 * ───────────────────────────────────────────────────────────────────────────── */
/* Read a single key and translate navigation keys to logical codes. */
static int ReadKey(void)
{
    int c = ReadChar();

    if (c >= '1' && c <= '9')
        return c;
    if (c == '\r' || c == '\n')
        return '\r';

    if (c == 27)
    {
        int c2 = ReadCharNonBlocking();
        if (c2 == -1)
            return 27; /* bare ESC */
        if (c2 == '[')
        {
            int c3 = ReadCharNonBlocking();
            if (c3 == 'A')
                return 'U';
            if (c3 == 'B')
                return 'D';
            if (c3 == '5')
            {
                ReadCharNonBlocking();
                return 'U';
            } /* PgUp */
            if (c3 == '6')
            {
                ReadCharNonBlocking();
                return 'D';
            } /* PgDn */
        }
        return 27; /* unrecognised sequence → treat as ESC */
    }
    return 0;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  LEFT-PANEL ASCII ART  (49 visible cols each — do not modify)
 * ───────────────────────────────────────────────────────────────────────────── */
static const char *ESI_ART[] = {
    "             ███████╗  ███████╗  ██╗             ",
    "             ██╔════╝  ██╔════╝  ██║             ",
    "             █████╗    ███████╗  ██║             ",
    "             ██╔══╝    ╚════██║  ██║             ",
    "             ███████╗  ███████║  ██║             ",
    "             ╚══════╝  ╚══════╝  ╚╝             ",
};
static const char *ESI_COL[] = {C5 BLD, C6 BLD, C7 BLD, C8 BLD, C9 BLD, C10 BLD};

static const char *ADDS_ART[] = {
    "       █████╗   ██████╗  ██████╗  ███████╗       ",
    "      ██╔══██╗  ██╔══██╗ ██╔══██╗ ██╔════╝       ",
    "      ███████║  ██║  ██║ ██║  ██║ ███████╗       ",
    "      ██╔══██║  ██║  ██║ ██║  ██║  ╚═══██║       ",
    "      ██║  ██║  ██████╔╝ ██████╔╝ ███████║       ",
    "      ╚═╝  ╚═╝  ╚═════╝  ╚═════╝  ╚══════╝       ",
};
static const char *ADDS_COL[] = {C7 BLD, C8 BLD, C9 BLD, C10 BLD, C11 BLD, C12 BLD};

static const char *TP_ART[] = {
    "               ████████╗ ██████╗                 ",
    "               ╚══██╔══╝ ██╔══██╗                ",
    "                  ██║    ██████╔╝                ",
    "                  ██║    ██╔═══╝                 ",
    "                  ██║    ██║                     ",
    "                  ╚═╝    ╚═╝                    ",
};
static const char *TP_COL[] = {C9 BLD, C10 BLD, C11 BLD, C12 BLD, C12 BLD, WHT BLD};

static const char *BADGE[] = {C6, C7, C8, C9, C10, C11, C12, WHT};
#define BADGE_N 8

/* ─────────────────────────────────────────────────────────────────────────────
 *  BOX SKELETON
 *
 *  Fixed 24 content rows (indices 0..23), so the box never overflows the screen.
 *
 *  Row  0      blank
 *  rows 1..6   ESI art     right slots 1..6
 *  Row  7      blank
 *  rows 8..13  ADDS art    right slots 8..13
 *  Row 14      blank
 *  rows 15..20 TP art      right slots 15..20
 *  Row 21      blank
 *  Row 22      nav hint    right slot 22
 *  Row 23      blank
 *
 *  Total content rows = 24.  Box height = 24 + 2 (borders) = 26 lines.
 *  Status bar = 2 lines below. Breadcrumb = 1 line above.
 *  Total screen usage = 29 lines — fits comfortably in any 35+ Row terminal.
 * ───────────────────────────────────────────────────────────────────────────── */
/* Render the full UI box with left art and right content slots. */
static void DrawBox(const char *rslots[RSLOTS])
{
    /* ── top Border ── */
    DrawBorder(C7
               "╔"
               "══════════"
               "══════════"
               "══════════"
               "══════════"
               "═════════"
               "╦"
               "══════════"
               "══════════"
               "══════════"
               "══════════"
               "════════"
               "╗" RST);

    /* ── content ── */
    DrawRow(NULL, "", rslots[0]);
    for (int i = 0; i < 6; i++)
        DrawRow(ESI_COL[i], ESI_ART[i], rslots[1 + i]);
    DrawRow(NULL, "", rslots[7]);
    for (int i = 0; i < 6; i++)
        DrawRow(ADDS_COL[i], ADDS_ART[i], rslots[8 + i]);
    DrawRow(NULL, "", rslots[14]);
    for (int i = 0; i < 6; i++)
        DrawRow(TP_COL[i], TP_ART[i], rslots[15 + i]);
    DrawRow(NULL, "", rslots[21]);
    DrawRow(NULL, "", rslots[22]);
    DrawRow(NULL, "", rslots[23]);

    /* ── bottom Border ── */
    DrawBorder(C7
               "╚"
               "══════════"
               "══════════"
               "══════════"
               "══════════"
               "═════════"
               "╩"
               "══════════"
               "══════════"
               "══════════"
               "══════════"
               "════════"
               "╝" RST);
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  OPTION FORMATTER
 *
 *  selected:  " ▶  [ N ]  LABEL"  (bright, bold)
 *  normal:    "     [ N ]  label"  (dim badge, muted label)
 *
 *  The string never needs explicit truncation because TruncateVisible() inside DrawRow()
 *  handles it automatically.
 * ───────────────────────────────────────────────────────────────────────────── */
/* Build the display string for one menu option. */
static void FormatOption(char *buf, int bufsz, int idx, const char *label, int selected)
{
    const char *bc = BADGE[idx < BADGE_N ? idx : BADGE_N - 1];
    if (selected)
        snprintf(buf, (size_t)bufsz,
                 C9 BLD " ▶  " C12 BLD "[ %d ]" RST "  " WHT BLD "%s" RST,
                 idx + 1, label);
    else
        snprintf(buf, (size_t)bufsz,
                 "     %s[ %d ]" RST "  " C7 "%s" RST,
                 bc, idx + 1, label);
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  GENERIC MENU ENGINE
 *
 *  Options 0..n-1 placed in ADDS rows (rslots 8..13):
 *    ≤3 options: spread with blank spacers for breathing room
 *    4-6 options: one per row, packed
 *
 *  Options 6..8 placed in TP rows (rslots 15..17).
 *  Title + separator: ESI rows 2..3.
 *  Nav hint: rslot 22.
 *
 *  Returns 0-based index, or -1 on ESC.
 * ───────────────────────────────────────────────────────────────────────────── */
/* Drive the menu input loop and return the selected index. */
static int GenericMenu(const char *title,
                       const char **opts, int n,
                       const char *crumb)
{
    if (n <= 0)
        return -1;

    int sel = 0;
    char nav[160];
    snprintf(nav, sizeof(nav),
             DIM GR2
             "  ↑↓  PgUp PgDn  navigate"
             "   Enter / 1-%d  select"
             "   ESC  back" RST,
             n < 9 ? n : 9);

    char opt_bufs[9][256];

    for (;;)
    {
        for (int i = 0; i < n && i < 9; i++)
            FormatOption(opt_bufs[i], 256, i, opts[i], i == sel);

        const char *rs[RSLOTS];
        for (int i = 0; i < RSLOTS; i++)
            rs[i] = "";

        /* title + separator in ESI rows */
        char rtitle[256];
        snprintf(rtitle, sizeof(rtitle), C10 BLD "  ◆  %s" RST, title);
        rs[2] = rtitle;
        rs[3] = C5
            "  ──────"
            "──────"
            "──────"
            "──────"
            "──────" RST;

        /*
         *  Spread options into ADDS rows (rslots 8..13):
         *  - 1..3 options: each gets a row, blank rows in between for spacing
         *  - 4..6 options: packed one-per-row
         */
        if (n <= 3)
        {
            /*  slot 8 = opt 0,  slot 9 = blank,
                slot 10 = opt 1,  slot 11 = blank,
                slot 12 = opt 2,  slot 13 = blank  */
            for (int i = 0; i < 3; i++)
            {
                rs[8 + i * 2] = (i < n) ? opt_bufs[i] : "";
                rs[8 + i * 2 + 1] = "";
            }
        }
        else
        {
            for (int i = 0; i < 6; i++)
                rs[8 + i] = (i < n) ? opt_bufs[i] : "";
        }

        /* options 6..8 in TP rows */
        for (int i = 0; i < 3; i++)
            rs[15 + i] = (6 + i < n) ? opt_bufs[6 + i] : "";

        rs[22] = nav;

        ClearScreen();
        DrawBreadcrumb(crumb);
        DrawBox(rs);
        DrawStatusBar();

        int k = ReadKey();
        if (k == 'U')
        {
            sel = (sel - 1 + n) % n;
        }
        else if (k == 'D')
        {
            sel = (sel + 1) % n;
        }
        else if (k == '\r')
        {
            return sel;
        }
        else if (k == 27)
        {
            return -1;
        }
        else if (k >= '1' && k <= '9')
        {
            int p = k - '1';
            if (p < n)
                return p;
        }
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  INTRO ANIMATION  (unchanged logic from your original)
 * ───────────────────────────────────────────────────────────────────────────── */
/* Type a centered line with a per-character delay. */
static void TypeAt(int r, const char *color, const char *text, int delay)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    int col = (tw - VisibleLength(text)) / 2;
    if (col < 0)
        col = 0;
    MoveCursor(r, col);
    printf("%s", color);
    for (int i = 0; text[i]; i++)
    {
        if (text[i] == '\033')
        {
            while (text[i] && text[i] != 'm')
            {
                putchar(text[i]);
                i++;
            }
            if (text[i])
            {
                putchar(text[i]);
            }
            FlushOutput();
            continue;
        }
        putchar(text[i]);
        FlushOutput();
        if (text[i] != ' ')
            SleepMillis(delay);
    }
    printf(RST "\033[K");
    FlushOutput();
}

/* Print a centered line with a single color. */
static void CenterPrint(int r, const char *color, const char *text)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    int col = (tw - VisibleLength(text)) / 2;
    if (col < 0)
        col = 0;
    MoveCursor(r, col);
    printf("%s%s" RST "\033[K", color, text);
    FlushOutput();
}

#define CW 80
#define CH 42
#define TROWS (CH / 2)
#define INFO_ROWS 4
#define TOTAL_ROWS (TROWS + INFO_ROWS)

typedef struct
{
    unsigned char r, g, b;
} RGB;
static RGB canvas[CH][CW];
static const RGB L_WHITE = {255, 255, 255};
static const RGB L_BLACK = {0, 0, 0};
static const RGB L_BLUE = {30, 105, 180};
static int g_pad = 0;

/* Reset the logo canvas to a white background. */
static void ClearCanvas(void)
{
    for (int y = 0; y < CH; y++)
        for (int x = 0; x < CW; x++)
            canvas[y][x] = L_WHITE;
}
/* Fill a rectangle on the logo canvas. */
static void FillRect(int x0, int y0, int w, int h, RGB c)
{
    for (int y = y0; y < y0 + h && y < CH; y++)
        for (int x = x0; x < x0 + w && x < CW; x++)
            if (x >= 0 && y >= 0)
                canvas[y][x] = c;
}
#define LS 5
#define LT 9
#define LB 35
/* Draw the letter E on the logo canvas. */
static void DrawE(void)
{
    int x = 10, w = 20, mid = (LT + LB) / 2 - LS / 2;
    FillRect(x, LT, LS, LB - LT, L_BLACK);
    FillRect(x, LT, w, LS, L_BLACK);
    FillRect(x + w - LS, LT, LS, mid + LS - LT, L_BLACK);
    FillRect(x, mid, w, LS, L_BLACK);
    FillRect(x, LB - LS, w, LS, L_BLACK);
}
/* Draw the letter S on the logo canvas. */
static void DrawS(void)
{
    int x = 34, w = 20, mid = (LT + LB) / 2 - LS / 2;
    FillRect(x, LT, w, LS, L_BLACK);
    FillRect(x, LT, LS, mid + LS - LT, L_BLACK);
    FillRect(x, mid, w, LS, L_BLACK);
    FillRect(x + w - LS, mid, LS, LB - mid, L_BLACK);
    FillRect(x, LB - LS, w, LS, L_BLACK);
}
/* Draw the letter I on the logo canvas. */
static void DrawI(void)
{
    FillRect(61, LT, 6, LB - LT, L_BLACK);
    FillRect(58, LT, 12, LS, L_BLACK);
    FillRect(58, LB - LS, 12, LS, L_BLACK);
}
/* Draw the accent dot on the logo canvas. */
static void DrawDot(void) { FillRect(60, 3, 8, 5, L_BLUE); }
/* Compare two RGB values. */
static int RgbEquals(RGB a, RGB b) { return a.r == b.r && a.g == b.g && a.b == b.b; }

/* Render the logo canvas using ANSI color blocks. */
static void RenderLogo(void)
{
    for (int y = 0; y < CH; y += 2)
    {
        for (int i = 0; i < g_pad; i++)
            putchar(' ');
        for (int x = 0; x < CW; x++)
        {
            RGB top = canvas[y][x];
            RGB bot = (y + 1 < CH) ? canvas[y + 1][x] : L_WHITE;
            if (RgbEquals(top, L_WHITE) && RgbEquals(bot, L_WHITE))
                printf("\033[48;2;255;255;255m ");
            else
                printf("\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm▀",
                       top.r, top.g, top.b, bot.r, bot.g, bot.b);
        }
        printf("\033[0m\033[K\n");
    }
}

/* Pause within the logo animation for the given duration. */
static void LogoSleep(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#endif
}

/* Type a centered line with a light typing effect. */
static void TypewriteCentered(const char *t, int tw)
{
    int p = (tw - (int)strlen(t)) / 2;
    if (p < 0)
        p = 0;
    PadSpaces(p);
    printf("\033[38;2;160;160;170m");
    for (int i = 0; t[i]; i++)
    {
        putchar(t[i]);
        fflush(stdout);
        LogoSleep(28);
    }
    printf("\033[0m\033[K\n");
}
/* Type two labels spaced left and right with a typing effect. */
static void TypewriteLeftRight(const char *l, const char *r, int tw)
{
    int ll = (int)strlen(l), rl = (int)strlen(r), sp = tw - 2 * g_pad - ll - rl;
    if (sp < 2)
        sp = 2;
    PadSpaces(g_pad);
    printf("\033[38;2;160;160;170m");
    for (int i = 0; i < ll; i++)
    {
        putchar(l[i]);
        fflush(stdout);
        LogoSleep(28);
    }
    for (int i = 0; i < sp; i++)
    {
        putchar(' ');
        fflush(stdout);
        LogoSleep(5);
    }
    for (int i = 0; i < rl; i++)
    {
        putchar(r[i]);
        fflush(stdout);
        LogoSleep(28);
    }
    printf("\033[0m\033[K\n");
}
/* Print a dashed separator line centered on the logo layout. */
static void PrintSeparatorLine(int tw)
{
    PadSpaces(g_pad);
    printf("\033[38;2;50;50;65m");
    for (int i = 0; i < tw - 2 * g_pad; i++)
        putchar('-');
    printf("\033[0m\033[K\n");
    fflush(stdout);
}

/* Show the ESI logo screen with typewriter metadata. */
static void ShowEsiLogo(void)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    g_pad = (tw - CW) / 2;
    if (g_pad < 0)
        g_pad = 0;
    int vpad = (th - TOTAL_ROWS - 2) / 2;
    if (vpad < 1)
        vpad = 1;
    printf("\033[%d;%dH", vpad, 1);
    ClearCanvas();
    DrawE();
    DrawS();
    DrawI();
    DrawDot();
    RenderLogo();
    printf("\n");
    PrintSeparatorLine(tw);
    printf("\n");
    TypewriteCentered("Encadreur :  Dr. Kermi", tw);
    printf("\n");
    static const char school[] = "  É"
                                "cole Supé"
                                "rieure d'Informatique  —"
                                "  Alger";
    static const char year[] = "2025 / 2026  ";
    TypewriteLeftRight(school, year, tw);
    printf("\n");
    {
        const char *msg = "Press any key to continue...";
        int lp = (tw - (int)strlen(msg)) / 2;
        if (lp < 0)
            lp = 0;
        PadSpaces(lp);
        printf(DIM WHT "%s" RST "\033[K\n", msg);
        fflush(stdout);
    }
    ReadChar();
}

/* Prompt the user to switch to fullscreen for best layout. */
static void ShowFullscreenNotice(void)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    ClearScreen();
    int m = th / 2 - 4;
    CenterPrint(m,     C8 BLD, "┌────────────────────────────────────────────────────┐");
    CenterPrint(m + 1, C8 BLD, "│                                                    │");
    
    int box_width = 54;
    int col = (tw - box_width) / 2;
    if (col < 0) col = 0;
    MoveCursor(m + 2, col);
    printf("%s│     ⚠  PLEASE PUT YOUR TERMINAL IN FULLSCREEN  ⚠\033[%dG│" RST "\033[K", C7 BLD, col + box_width);
    FlushOutput();

    CenterPrint(m + 3, C8 BLD, "│                                                    │");
    CenterPrint(m + 4, C9,     "│           Minimum recommended: 120 x 35            │");
    CenterPrint(m + 5, C8 BLD, "│                                                    │");
    CenterPrint(m + 6, C8 BLD, "└────────────────────────────────────────────────────┘");
    CenterPrint(m + 9, DIM WHT, "Press any key to continue...");
    fflush(stdout);
    ReadChar();
    ClearScreen();
}

/* Show the TP1 title animation. */
static void ShowTp1(void)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    const char *tp[] = {
        "████████╗ ██████╗ ",
        "╚══██╔══╝ ██╔══██╗",
        "   ██║    ██████╔╝",
        "   ██║    ██╔═══╝ ",
        "   ██║    ██║     ",
        "   ╚═╝    ╚═╝     "};
    const char *fc[] = {C2, C4, C6, C8, C10, C12};
    int r = (th - 6) / 2 - 3;
    if (r < 2)
        r = 2;
    for (int i = 0; i < 6; i++)
        TypeAt(r + i, fc[i], tp[i], 10);
    FlushOutput();
    SleepMillis(1000);
}
/* Show the accomplishment banner animation. */
static void ShowAccomplished(void)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    const char *l[] = {
        "╔═╗╔═╗╔═╗╔═╗╔╦╗╔═╗╦  ╦╔═╗╦ ╦╔═╗╔╦╗  ╔╗ ╦ ╦",
        "╠═╣║  ║  ║ ║║║║╠═╝║  ║╚═╗╠═╣║╣  ║║  ╠╩╗╚╦╝",
        "╩ ╩╚═╝╚═╝╚═╝╩ ╩╩  ╩═╝╩╚═╝╩ ╩╚═╝═╩╝  ╚═╝ ╩ "};
    const char *fc[] = {C7, C9, C11};
    int r = (th - 6) / 2 + 4;
    for (int i = 0; i < 3; i++)
        TypeAt(r + i, fc[i], l[i], 7);
    FlushOutput();
    SleepMillis(600);
}
/* Show the authors' names with a color sweep. */
static void ShowNames(void)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    char f1[256], f2[256], f3[256];
    snprintf(f1, sizeof(f1), "%s    &    %s", "╦ ╦  ╔═╗╔╗ ╔╦╗╔═╗╦  ╔╦╗╔═╗╦ ╦╔╦╗╔═╗╔╗╔╔═╗", "╔╗  ╔╦╗╔═╗╦  ╔╗ ╦");
    snprintf(f2, sizeof(f2), "%s    &    %s", "╠═╣  ╠═╣╠╩╗ ║║║╣ ║  ║║║║ ║║ ║║║║║╣ ║║║║╣ ", "╠╩╗  ║ ╠═╣║  ╠╩╗║");
    snprintf(f3, sizeof(f3), "%s    &    %s", "╩ ╩● ╩ ╩╚═╝═╩╝╚═╝╩═╝╩ ╩╚═╝╚═╝╩ ╩╚═╝╝╚╝╚═╝", "╚═╝● ╩ ╩ ╩╩═╝╚═╝╩");
    int r = (th - 6) / 2 + 9;
    const char *rv[] = {C1, C3, C5, C7, C9, C11};
    for (int s = 0; s < 6; s++)
    {
        CenterPrint(r, rv[s], f1);
        CenterPrint(r + 1, rv[s], f2);
        CenterPrint(r + 2, rv[s], f3);
        FlushOutput();
        SleepMillis(40);
    }
    CenterPrint(r, C10 BLD, f1);
    CenterPrint(r + 1, C11 BLD, f2);
    CenterPrint(r + 2, C12 BLD, f3);
    FlushOutput();
    SleepMillis(500);
}
/* Animate the divider line after the names. */
static void ShowSeparatorAnimation(void)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    int r = (th - 6) / 2 + 14, w = 70, col = (tw - w) / 2, center = w / 2;
    if (col < 0)
        col = 0;
    for (int half = 1; half <= center + 1; half++)
    {
        MoveCursor(r, col);
        for (int x = 0; x < w; x++)
        {
            int d = abs(x - center);
            if (d <= half)
            {
                if (d < half / 3)
                    printf(C12);
                else if (d < 2 * half / 3)
                    printf(C8);
                else
                    printf(C4);
                printf("─");
            }
            else
                printf(" ");
        }
        printf(RST "\033[K");
        FlushOutput();
        SleepMillis(6);
    }
}

/* Run the full intro animation sequence. */
void RunAnimation(void)
{
    InitTerminal();
    printf(HCUR);
    FlushOutput();
    ShowFullscreenNotice();
    ShowEsiLogo();
    ClearScreen();
    SleepMillis(300);
    ShowTp1();
    ShowAccomplished();
    ShowNames();
    ShowSeparatorAnimation();
    {
        int tw, th;
        GetTerminalSize(&tw, &th);
        CenterPrint((th - 6) / 2 + 16, DIM GR1, "ESI Alger  ·  2025/2026");
        CenterPrint(th - 2, DIM WHT, "Press any key to continue...");
    }
    FlushOutput();
    printf(SCUR);
    FlushOutput();
    ReadChar();
    ClearScreen();
    MoveCursor(0, 0);
    printf(RST);
    FlushOutput();
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  NOTIFICATION HELPERS
 * ───────────────────────────────────────────────────────────────────────────── */
/* Show a short success notification near the bottom. */
static void NotifyOk(const char *msg)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    MoveCursor(th - 4, 0);
    int lp = (tw - (int)strlen(msg) - 8) / 2;
    if (lp < 0)
        lp = 0;
    PadSpaces(lp);
    printf(GRN "  ✔  " RST C11 "%s" RST "\033[K\n", msg);
    FlushOutput();
    SleepMillis(900);
}
/* Show a short error notification near the bottom. */
static void NotifyError(const char *msg)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    MoveCursor(th - 4, 0);
    int lp = (tw - (int)strlen(msg) - 8) / 2;
    if (lp < 0)
        lp = 0;
    PadSpaces(lp);
    printf(RED "  ✘  " RST C11 "%s" RST "\033[K\n", msg);
    FlushOutput();
    SleepMillis(1200);
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  PUBLIC MENU FUNCTIONS
 * ───────────────────────────────────────────────────────────────────────────── */
/* Display the main menu and return the selected index. */
int MenuMain(void)
{
    static const char *opts[] = {
        "Load File",
        "List Loaded Files",
        "Set Operations",
        "Exit",
    };
    /* U+203A = › (›) */
    return GenericMenu("MAIN MENU", opts, 4, "ADDS  ›  Set Theory");
}

/* Display the level selection menu. */
int MenuLevel(void)
{
    static const char *opts[] = {
        "Word        —  BST word operations",
        "Sentence    —  sentence linked-list operations",
        "Paragraph   —  paragraph linked-list operations",
    };
    return GenericMenu("SELECT LEVEL", opts, 3,
                       "Main Menu  ›  Set Operations  ›  Level");
}

/* Display the operation selection menu. */
int MenuOperation(void)
{
    static const char *names[] = {
        "Union",
        "Intersection",
        "Difference",
    };
    static const char *plain[] = {
        "elements from A or B",
        "elements from both A & B",
        "elements from A, not B",
    };
    static const char *math[] = {
        "A ∪ B = { x | (x ∈ A) ∨ (x ∈ B) }",
        "A ∩ B = { x | (x ∈ A) ∧ (x ∈ B) }",
        "A ∖ B = { x | (x ∈ A) ∧ (x ∉ B) }",
    };
    int sel = 0;
    char opt_lines[3][256];
    char def_lines[3][256];
    char nav[160];
    snprintf(nav, sizeof(nav),
             DIM GR2 "  ↑↓ navigate   Enter/1-3 select   ESC back" RST);

    for (;;)
    {
        const char *rs[RSLOTS];
        for (int i = 0; i < RSLOTS; i++)
            rs[i] = "";

        rs[2] = C10 BLD "  ◆  SELECT OPERATION" RST;
        rs[3] = C5 "  ──────────────────────────────" RST;

        for (int i = 0; i < 3; i++)
        {
            if (i == sel)
                snprintf(opt_lines[i], sizeof(opt_lines[i]),
                         C9 BLD " ▶ " C12 BLD "[%d]" RST " " WHT BLD "%-12s" RST
                         C4 " " RST C10 "%s" RST,
                         i + 1, names[i], plain[i]);
            else
                snprintf(opt_lines[i], sizeof(opt_lines[i]),
                         "   %s[%d]" RST " " C7 "%-12s" RST
                         C4 " " RST DIM GR2 "%s" RST,
                         BADGE[i], i + 1, names[i], plain[i]);

            snprintf(def_lines[i], sizeof(def_lines[i]),
                     "    " C8 "%s" RST, math[i]);
        }

        rs[8] = opt_lines[0];
        rs[9] = def_lines[0];
        rs[11] = opt_lines[1];
        rs[12] = def_lines[1];
        rs[15] = opt_lines[2];
        rs[16] = def_lines[2];
        rs[22] = nav;

        ClearScreen();
        DrawBreadcrumb("Main Menu  ›  Set Operations  ›  Operation");
        DrawBox(rs);
        DrawStatusBar();

        int k = ReadKey();
        if (k == 'U')
            sel = (sel - 1 + 3) % 3;
        else if (k == 'D')
            sel = (sel + 1) % 3;
        else if (k == '\r')
            return sel;
        else if (k == 27)
            return -1;
        else if (k >= '1' && k <= '3')
            return k - '1';
    }
}

/* Display a file picker menu for the loaded files list. */
int MenuPickFile(const char **files, int n, const char *crumb)
{
    if (n == 0)
    {
        NotifyError("No files loaded — please load a file first.");
        return -1;
    }
    return GenericMenu("SELECT FILE", files, n, crumb);
}

/* Display a paragraph picker menu for a file. */
int MenuPickParagraph(const char **labels, int n, const char *crumb)
{
    if (n == 0)
    {
        NotifyError("File is empty — no paragraphs found.");
        return -1;
    }
    return GenericMenu("SELECT PARAGRAPH", labels, n, crumb);
}

/* Display a sentence picker menu for a paragraph. */
int MenuPickSentence(const char **labels, int n, const char *crumb)
{
    if (n == 0)
    {
        NotifyError("Paragraph is empty — no sentences found.");
        return -1;
    }
    return GenericMenu("SELECT SENTENCE", labels, n, crumb);
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  DIALOG SCREENS
 * ───────────────────────────────────────────────────────────────────────────── */
/* Prompt the user to enter a file path. */
int ScreenLoadFile(char *buf, int buf_size)
{
    const char *rs[RSLOTS];
    for (int i = 0; i < RSLOTS; i++)
        rs[i] = "";
    rs[2] = C10 BLD "  ◆  LOAD FILE" RST;
    rs[3] = C5 "  ──────────────────────────────" RST;
    rs[5] = C7 "  Enter the path to the file:" RST;
    rs[9] = C8 "  Examples:" RST;
    rs[10] = C5 "    tests/test1.txt" RST;
    rs[11] = C5 "    tests/test2.txt" RST;
    rs[12] = C5 "    /absolute/path/to/file.txt" RST;
    rs[20] = DIM GR2 "  ESC  cancel   Backspace  delete" RST;

    ClearScreen();
    DrawBreadcrumb("Main Menu  ›  Load File");
    DrawBox(rs);
    DrawStatusBar();

    int tw, th;
    GetTerminalSize(&tw, &th);
    MoveCursor(th - 4, 0);
    PadSpaces(BoxColumn());
    printf(C9 "  ›  " RST);
    printf(SCUR);
    FlushOutput();

    int pos = 0;
    buf[0] = '\0';
    for (;;)
    {
        int c = ReadChar();
        if (c == 27)
        {
            printf("\n" HCUR);
            FlushOutput();
            buf[0] = '\0';
            return 0;
        }
        if (c == '\r' || c == '\n')
        {
            printf("\n");
            break;
        }
        if (c == 127 || c == '\b')
        {
            if (pos > 0)
            {
                pos--;
                printf("\b \b");
                FlushOutput();
            }
            continue;
        }
        if (pos < buf_size - 1 && c >= 32 && c < 127)
        {
            buf[pos++] = (char)c;
            putchar(c);
            FlushOutput();
        }
    }
    buf[pos] = '\0';
    printf(HCUR);
    FlushOutput();
    return pos > 0;
}

/* Show a list of loaded files with paragraph counts. */
void ScreenListFiles(const char **files, const int *para_cnt, int n)
{
    static char fbufs[8][160];
    const char *rs[RSLOTS];
    for (int i = 0; i < RSLOTS; i++)
        rs[i] = "";

    rs[2] = C10 BLD "  ◆  LOADED FILES" RST;
    rs[3] = C5 "  ──────────────────────────────" RST;

    if (n == 0)
    {
        rs[5] = C4 "  No files loaded yet." RST;
    }
    else
    {
        for (int i = 0; i < n && i < 8; i++)
        {
            snprintf(fbufs[i], sizeof(fbufs[i]),
                     C9 "  [%d]" RST "  " C7 "%s" RST C4 "  (%d para)" RST,
                     i, files[i], para_cnt[i]);
            if (i < 6)
                rs[8 + i] = fbufs[i];
            else
                rs[15 + (i - 6)] = fbufs[i];
        }
    }
    rs[22] = DIM GR2 "  Press any key to return" RST;

    ClearScreen();
    DrawBreadcrumb("Main Menu  ›  List Loaded Files");
    DrawBox(rs);
    DrawStatusBar();
    ReadChar();
}

static int WrapPlainText(const char *text, char lines[][512], int max_lines, int width)
{
    const char *p = text;
    int count = 0;

    for (int i = 0; i < max_lines; i++)
        lines[i][0] = '\0';

    if (p == NULL || p[0] == '\0')
    {
        snprintf(lines[0], 512, "∅");
        return 1;
    }

    while (*p && count < max_lines)
    {
        char word[256];
        int wpos = 0;

        while (*p && isspace((unsigned char)*p))
            p++;
        if (!*p)
            break;

        while (*p && !isspace((unsigned char)*p) && wpos < (int)sizeof(word) - 1)
            word[wpos++] = *p++;
        word[wpos] = '\0';

        while (*p && !isspace((unsigned char)*p))
            p++;

        int line_len = VisibleLength(lines[count]);
        int word_len = VisibleLength(word);
        if (line_len > 0 && line_len + 1 + word_len > width)
        {
            count++;
            if (count >= max_lines)
                break;
            line_len = 0;
        }

        if (line_len > 0)
            strncat(lines[count], " ", 512 - strlen(lines[count]) - 1);
        strncat(lines[count], word, 512 - strlen(lines[count]) - 1);
    }

    if (count >= max_lines)
        count = max_lines - 1;
    if (*p && max_lines > 0)
        strncat(lines[max_lines - 1], " ...", 512 - strlen(lines[max_lines - 1]) - 1);

    if (lines[count][0] != '\0')
        count++;
    return count > 0 ? count : 1;
}

/* Show the result screen for a set operation. */
void ScreenShowResult(const char *op_name, const char *a_label,
                      const char *b_label, const char *result_line)
{
    static char title[160], metaA[180], metaB[180], result_rows[10][512], display_rows[10][640];
    int result_count = WrapPlainText(result_line, result_rows, 10, 84);

    snprintf(title, sizeof(title), C10 BLD "  ◆  RESULT  │  %s" RST, op_name);
    snprintf(metaA, sizeof(metaA), C7 "  A" RST C4 "  ──  " RST C12 "%s" RST, a_label);
    snprintf(metaB, sizeof(metaB), C7 "  B" RST C4 "  ──  " RST C12 "%s" RST, b_label);

    const char *rs[RSLOTS];
    for (int i = 0; i < RSLOTS; i++)
        rs[i] = "";

    rs[1] = title;
    rs[2] = C5 "  ────────────────────────────────────────────────────────────────────────────────" RST;
    rs[4] = metaA;
    rs[5] = metaB;
    rs[7] = C8 BLD "  Computed Set" RST C4 "  ═══════════════════════════════════════════════════════" RST;

    for (int i = 0; i < result_count && i < 10; i++)
    {
        snprintf(display_rows[i], sizeof(display_rows[i]),
                 C9 "  %02d" RST C4 "  │  " RST C12 "%s" RST,
                 i + 1, result_rows[i]);
        rs[9 + i] = display_rows[i];
    }

    rs[21] = C4 "  ────────────────────────────────────────────────────────────────────────────────" RST;
    rs[22] = DIM GR2 "  Press any key to return" RST;

    ClearScreen();
    DrawBreadcrumb("Main Menu  ›  Set Operations  ›  Result");
    DrawFullBox(rs);
    DrawStatusBar();
    ReadChar();
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  FUTURISTIC LOADING ANIMATION
 * ───────────────────────────────────────────────────────────────────────────── */
static void ShowLoadingBar(const char *label, int duration_ms)
{
    int tw, th;
    GetTerminalSize(&tw, &th);
    int panel_w = 70;
    int bar_w = 46;
    int steps = 54;
    int step_ms = duration_ms / steps;
    if (step_ms < 1)
        step_ms = 1;
    int col = (tw - panel_w) / 2;
    if (col < 0) col = 0;
    int row = th / 2 - 4;
    if (row < 1)
        row = 1;
    const char *spin = "◜◝◞◟";
    int spin_len = 4;

    ClearScreen();
    for (int i = 0; i <= steps; i++)
    {
        int pct = (i * 100) / steps;
        int si = (i / 2) % spin_len;

        MoveCursor(row, col);
        printf(C7 "╔");
        for (int j = 0; j < panel_w - 2; j++)
            printf("═");
        printf("╗" RST "\033[K");

        MoveCursor(row + 1, col);
        printf(C7 "║" RST);
        int title_w = VisibleLength(label) + 9;
        int title_pad = (panel_w - 2 - title_w) / 2;
        if (title_pad < 0)
            title_pad = 0;
        PadSpaces(title_pad);
        printf(C10 BLD "%.*s" RST "  " WHT BLD "%s" RST "  " C12 BLD "%3d%%" RST,
               3, spin + si * 3, label, pct);
        int used = title_pad + title_w;
        PadSpaces(panel_w - 2 - used > 0 ? panel_w - 2 - used : 0);
        printf(C7 "║" RST "\033[K");

        MoveCursor(row + 2, col);
        printf(C7 "║" RST);
        PadSpaces(10);
        printf(C4 "▕" RST);
        for (int j = 0; j < bar_w; j++)
        {
            int fill = (i * bar_w) / steps;
            if (j < fill)
            {
                if ((j + i) % 11 == 0)    printf(C12 "█" RST);
                else if (j < bar_w / 3)   printf(C6 "█" RST);
                else if (j < 2 * bar_w / 3) printf(C8 "█" RST);
                else                       printf(C10 "█" RST);
            }
            else
                printf(DIM C4 "░" RST);
        }
        printf(C4 "▏" RST);
        PadSpaces(10);
        printf(C7 "║" RST "\033[K");

        MoveCursor(row + 3, col);
        printf(C7 "║" RST);
        PadSpaces(10);
        printf(DIM GR2);
        for (int j = 0; j < bar_w + 2; j++)
        {
            int center = (bar_w + 2) / 2;
            int reach = (i * (bar_w + 2)) / (2 * steps);
            if (abs(j - center) <= reach)
                printf("─");
            else
                printf(" ");
        }
        printf(RST);
        PadSpaces(10);
        printf(C7 "║" RST "\033[K");

        MoveCursor(row + 4, col);
        printf(C7 "╚");
        for (int j = 0; j < panel_w - 2; j++)
            printf("═");
        printf("╝" RST "\033[K");

        FlushOutput();
        SleepMillis(step_ms);
    }
    SleepMillis(200);
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  HELPERS: collect BST words into a single display string
 * ───────────────────────────────────────────────────────────────────────────── */
static void InorderToPrettyBuffer(WordNode *r, char *buf, int bufsz, int *pos, int *first)
{
    if (!r || *pos >= bufsz - 1)
        return;

    InorderToPrettyBuffer(r->left, buf, bufsz, pos, first);

    int n = snprintf(buf + *pos, bufsz - *pos, "%s%s",
                     *first ? "" : "  •  ", r->val);
    if (n > 0)
    {
        *pos += n;
        *first = 0;
    }

    InorderToPrettyBuffer(r->right, buf, bufsz, pos, first);
}

static void BstToString(WordNode *r, char *buf, int bufsz)
{
    int pos = 0, first = 1;
    if (!r)
    {
        snprintf(buf, bufsz, "∅");
        return;
    }

    pos += snprintf(buf + pos, bufsz - pos, "{ ");
    InorderToPrettyBuffer(r, buf, bufsz, &pos, &first);
    snprintf(buf + pos, bufsz - pos, " }");
}

/* Build a display string for a SentenceList result using original text */
static void SentenceListToString(SentenceList list, char *buf, int bufsz)
{
    int pos = 0;
    buf[0] = '\0';
    SentenceNode *cur = list.head;
    if (!cur)
    {
        snprintf(buf, bufsz, "∅");
        return;
    }
    while (cur && pos < bufsz - 2)
    {
        const char *text = (cur->original && cur->original[0]) ? cur->original : "(?)";
        int n = snprintf(buf + pos, bufsz - pos, "S%d: %s  •  ", cur->id, text);
        if (n > 0) pos += n;
        cur = Next(cur);
    }
}

/* Build a display string for a ParagraphList result using original text */
static void ParagraphListToString(ParagraphList list, char *buf, int bufsz)
{
    int pos = 0;
    buf[0] = '\0';
    ParagraphNode *cur = list.head;
    if (!cur)
    {
        snprintf(buf, bufsz, "∅");
        return;
    }
    while (cur && pos < bufsz - 2)
    {
        const char *text = (cur->original && cur->original[0]) ? cur->original : "(?)";
        int n = snprintf(buf + pos, bufsz - pos, "P%d: %s  •  ", cur->id, text);
        if (n > 0) pos += n;
        cur = Next(cur);
    }
}

/* Build label arrays for paragraphs in a file */
static int BuildParagraphLabels(ParagraphList *pl, char labels[][80], const char **ptrs, int max)
{
    int n = pl->count < max ? pl->count : max;
    for (int i = 0; i < n; i++)
    {
        ParagraphNode *p = GetParagraphByIndex(pl, i);
        snprintf(labels[i], 80, "Paragraph %d  (%d sentence%s)", i, p->val.count, p->val.count == 1 ? "" : "s");
        ptrs[i] = labels[i];
    }
    return n;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  FULL-PAGE RESULT with scrollable output to stdout
 * ───────────────────────────────────────────────────────────────────────────── */
static void ScreenShowFullResult(const char *op_name, const char *a_label,
                                  const char *b_label, const char *level_name,
                                  WordNode *word_result, SentenceList *sent_result,
                                  ParagraphList *para_result)
{
    char rbuf[4096];
    rbuf[0] = '\0';
    if (strcmp(level_name, "WORD") == 0)
        BstToString(word_result, rbuf, sizeof(rbuf));
    else if (sent_result)
        SentenceListToString(*sent_result, rbuf, sizeof(rbuf));
    else if (para_result)
        ParagraphListToString(*para_result, rbuf, sizeof(rbuf));

    char title[128];
    snprintf(title, sizeof(title), "%s  %s", level_name, op_name);
    ScreenShowResult(title, a_label, b_label, rbuf);
}

static const char *OP_NAMES[] = {"UNION", "INTERSECTION", "DIFFERENCE"};

static int IsReadableFile(const char *filename)
{
    if (filename == NULL || filename[0] == '\0')
        return 0;

    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return 0;

    fclose(file);
    return 1;
}

/* ─────────────────────────────────────────────────────────────────────────────
 *  MAIN LOOP
 * ───────────────────────────────────────────────────────────────────────────── */
void RunMenu(void)
{
    printf(HCUR);
    FlushOutput();

    FileList file_list = CreateFileList();

    for (;;)
    {
        int choice = MenuMain();
        if (choice == -1)
            choice = 3;

        switch (choice)
        {

        case 0:
        { /* ── Load File ── */
            char filename[512];
            int got = ScreenLoadFile(filename, sizeof(filename));
            if (!got)
                break;
            if (!IsReadableFile(filename))
            {
                NotifyError("Invalid path — file could not be opened.");
                break;
            }
            ShowLoadingBar("Parsing file...", 600);
            if (AddFile(&file_list, filename))
                NotifyOk("File loaded successfully.");
            else
                NotifyError("Invalid path — file could not be opened.");
            break;
        }

        case 1:
        { /* ── List Loaded Files ── */
            const char *fnames[8];
            int pcounts[8];
            int n = file_list.count < 8 ? file_list.count : 8;
            for (int i = 0; i < n; i++)
            {
                FileNode *fn = GetFileByIndex(&file_list, i);
                fnames[i] = fn->filename;
                pcounts[i] = fn->val.count;
            }
            ScreenListFiles(fnames, pcounts, n);
            break;
        }

        case 2:
        { /* ── Set Operations ── */
            if (file_list.count == 0)
            {
                NotifyError("No files loaded — please load a file first.");
                break;
            }
            int level = MenuLevel();
            if (level == -1) break;
            int op = MenuOperation();
            if (level == -1 || op == -1) break;

            /* Build file name array for pickers */
            const char *fnames[8];
            int fn_count = file_list.count < 8 ? file_list.count : 8;
            for (int i = 0; i < fn_count; i++)
                fnames[i] = GetFileByIndex(&file_list, i)->filename;

            if (level == 0)
            { /* ── Word Level (between paragraphs) ── */
                /* Pick file A → paragraph A */
                int fa = MenuPickFile(fnames, fn_count, "Set Ops  ›  Word  ›  File A");
                if (fa < 0) break;
                FileNode *fnA = GetFileByIndex(&file_list, fa);
                char plA[9][80]; const char *ppA[9];
                int npA = BuildParagraphLabels(&fnA->val, plA, ppA, 9);
                int pa = MenuPickParagraph(ppA, npA, "Set Ops  ›  Word  ›  Para A");
                if (pa < 0) break;
                ParagraphNode *parA = GetParagraphByIndex(&fnA->val, pa);

                /* Pick file B → paragraph B */
                int fb = MenuPickFile(fnames, fn_count, "Set Ops  ›  Word  ›  File B");
                if (fb < 0) break;
                FileNode *fnB = GetFileByIndex(&file_list, fb);
                char plB[9][80]; const char *ppB[9];
                int npB = BuildParagraphLabels(&fnB->val, plB, ppB, 9);
                int pb = MenuPickParagraph(ppB, npB, "Set Ops  ›  Word  ›  Para B");
                if (pb < 0) break;
                ParagraphNode *parB = GetParagraphByIndex(&fnB->val, pb);

                /* Collect all words from all sentences in each paragraph */
                WordNode *allA = NULL, *allB = NULL;
                for (SentenceNode *s = parA->val.head; s; s = Next(s))
                    CopyTree(s->val, &allA);
                for (SentenceNode *s = parB->val.head; s; s = Next(s))
                    CopyTree(s->val, &allB);

                ShowLoadingBar("Computing word operation...", 800);

                /* Execute */
                WordNode *result = NULL;
                if (op == 0)      result = WordUnion(allA, allB);
                else if (op == 1) result = WordIntersection(allA, allB);
                else              result = WordDifference(allA, allB);

                char labA[80], labB[80];
                snprintf(labA, sizeof(labA), "File%d P%d", fa, pa);
                snprintf(labB, sizeof(labB), "File%d P%d", fb, pb);
                ScreenShowFullResult(OP_NAMES[op], labA, labB, "WORD", result, NULL, NULL);
                FreeTree(&allA);
                FreeTree(&allB);
                FreeTree(&result);
            }
            else if (level == 1)
            { /* ── Sentence Level ── */
                int fa = MenuPickFile(fnames, fn_count, "Set Ops  ›  Sentence  ›  File A");
                if (fa < 0) break;
                FileNode *fnA = GetFileByIndex(&file_list, fa);
                char plA[9][80]; const char *ppA[9];
                int npA = BuildParagraphLabels(&fnA->val, plA, ppA, 9);
                int pa = MenuPickParagraph(ppA, npA, "Set Ops  ›  Sentence  ›  Para A");
                if (pa < 0) break;
                ParagraphNode *parA = GetParagraphByIndex(&fnA->val, pa);

                int fb = MenuPickFile(fnames, fn_count, "Set Ops  ›  Sentence  ›  File B");
                if (fb < 0) break;
                FileNode *fnB = GetFileByIndex(&file_list, fb);
                char plB[9][80]; const char *ppB[9];
                int npB = BuildParagraphLabels(&fnB->val, plB, ppB, 9);
                int pb = MenuPickParagraph(ppB, npB, "Set Ops  ›  Sentence  ›  Para B");
                if (pb < 0) break;
                ParagraphNode *parB = GetParagraphByIndex(&fnB->val, pb);

                SentenceList result;
                if (op == 0)      result = SentenceUnion(parA->val, parB->val);
                else if (op == 1) result = SentenceIntersection(parA->val, parB->val);
                else              result = SentenceDifference(parA->val, parB->val);

                ShowLoadingBar("Computing sentence operation...", 800);

                char labA[80], labB[80];
                snprintf(labA, sizeof(labA), "File%d P%d", fa, pa);
                snprintf(labB, sizeof(labB), "File%d P%d", fb, pb);
                ScreenShowFullResult(OP_NAMES[op], labA, labB, "SENTENCE", NULL, &result, NULL);
                FreeSentenceList(&result);
            }
            else
            { /* ── Paragraph Level ── */
                if (file_list.count < 2)
                {
                    NotifyError("Need at least 2 files for paragraph operations.");
                    break;
                }
                int fa = MenuPickFile(fnames, fn_count, "Set Ops  ›  Paragraph  ›  File A");
                if (fa < 0) break;
                int fb = MenuPickFile(fnames, fn_count, "Set Ops  ›  Paragraph  ›  File B");
                if (fb < 0) break;
                FileNode *fnA = GetFileByIndex(&file_list, fa);
                FileNode *fnB = GetFileByIndex(&file_list, fb);

                ParagraphList result;
                if (op == 0)      result = ParagraphUnion(fnA->val, fnB->val);
                else if (op == 1) result = ParagraphIntersection(fnA->val, fnB->val);
                else              result = ParagraphDifference(fnA->val, fnB->val);

                ShowLoadingBar("Computing paragraph operation...", 800);

                ScreenShowFullResult(OP_NAMES[op], fnA->filename, fnB->filename, "PARAGRAPH", NULL, NULL, &result);
                FreeParagraphList(&result);
            }
            break;
        }

        case 3:
        { /* ── Exit ── */
            FreeFileList(&file_list);
            ClearScreen();
            int tw, th;
            GetTerminalSize(&tw, &th);
            CenterPrint(th / 2, C9 BLD, "  ✦  Thank you for using ADDS Set Theory  ✦");
            CenterPrint(th / 2 + 1, DIM GR1, "ESI Alger  ·  2025/2026");
            FlushOutput();
            SleepMillis(1400);
            printf(SCUR);
            FlushOutput();
            ClearScreen();
            MoveCursor(0, 0);
            printf(RST);
            FlushOutput();
            return;
        }

        default:
            break;
        }
    }
}
