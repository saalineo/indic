/*
 * indic.h - terminal UI helpers for C/C++
 *
 * Colors, progress bars, spinners and cursor control, with no dependencies
 * beyond the C standard library and OS terminal headers.
 *
 * Usage:
 *     #define INDIC_IMPLEMENTATION  (in exactly one .c/.cpp file)
 *     #include "indic.h"
 *     indic_init();                       // once, at start of main()
 *     printf(INDIC_PAINT(INDIC_GREEN, "ok\n"));
 *
 * Options (define before including):
 *     INDIC_ASCII   use '#' and '-' instead of block characters
 *
 * License: GPL-3.0-or-later (see LICENSE)
 */
#ifndef INDIC_H
#define INDIC_H

#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE) && !defined(_GNU_SOURCE)
  #define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <io.h>
  #ifndef INDIC_ISATTY
    #define INDIC_ISATTY(f) _isatty(_fileno(f))
  #endif
#else
  #include <unistd.h>
  #include <sys/ioctl.h>
  #ifndef INDIC_ISATTY
    #define INDIC_ISATTY(f) isatty(fileno(f))
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define INDIC_RESET    "\033[0m"
#define INDIC_BOLD     "\033[1m"
#define INDIC_DIM      "\033[2m"
#define INDIC_RED      "\033[31m"
#define INDIC_GREEN    "\033[32m"
#define INDIC_YELLOW   "\033[33m"
#define INDIC_BLUE     "\033[34m"
#define INDIC_MAGENTA  "\033[35m"
#define INDIC_CYAN     "\033[36m"
#define INDIC_WHITE    "\033[37m"

#define INDIC_PAINT(style, text) style text INDIC_RESET
int indic_init(void);
const char *indic_style(const char *s);
void indic_cursor_hide(void);
void indic_cursor_show(void);

// Terminal width
int indic_term_width(void);

// Progress bar
#ifdef INDIC_ASCII
  #define INDIC__FULL  "#"
  #define INDIC__EMPTY "-"
#else
  #define INDIC__FULL  "\xE2\x96\x88" /* █ */
  #define INDIC__EMPTY "\xE2\x96\x91" /* ░ */
#endif

/*
 * Return the number of bytes needed for a progress string, including its
 * trailing NUL.  Returns 0 when width cannot be represented safely.
 */
size_t indic_progress_str_size(int width);

/*
 * Write a progress string to buf.  buf_size must include space for the NUL.
 * Returns 0 on success and -1 if buf is invalid or too small.
 */
int indic_progress_str(char *buf, size_t buf_size, double frac, int width);
void indic_progress(double frac, int width);

// Spinner
char indic_spinner_frame(int tick);

void indic_spinner_tick(const char *msg);
void indic_spinner_done(const char *msg);

#ifdef __cplusplus
}
#endif

#endif /* INDIC_H */

/*
 * Define INDIC_IMPLEMENTATION in exactly one translation unit before this
 * header.  That translation unit owns the library state and function bodies.
 */
#ifdef INDIC_IMPLEMENTATION
#ifndef INDIC_IMPLEMENTATION_INCLUDED
#define INDIC_IMPLEMENTATION_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

static int indic__ansi_on = 0;
static int indic__color_on = 0;
static int indic__cursor_hidden = 0;
static int indic__inited = 0;
static int indic__spinner_tick = 0;

static void indic__cleanup(void) {
    if (indic__ansi_on) {
        if (indic__cursor_hidden) {
            fputs("\033[?25h", stdout);
            indic__cursor_hidden = 0;
        }
        if (indic__color_on) fputs(INDIC_RESET, stdout);
        fflush(stdout);
    }
}

static void indic__on_sigint(int sig) {
    indic__cleanup();
    signal(sig, SIG_DFL);
    raise(sig);
}

void indic_cursor_hide(void) {
    if (!indic__ansi_on) return;
    fputs("\033[?25l", stdout);
    fflush(stdout);
    indic__cursor_hidden = 1;
}

void indic_cursor_show(void) {
    if (!indic__ansi_on) return;
    fputs("\033[?25h", stdout);
    fflush(stdout);
    indic__cursor_hidden = 0;
}

int indic_init(void) {
    if (indic__inited) return indic__ansi_on;
    indic__inited = 1;

    if (!INDIC_ISATTY(stdout)) return 0;
#ifdef _WIN32
    {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (h == INVALID_HANDLE_VALUE || !GetConsoleMode(h, &mode)) return 0;
        if (!SetConsoleMode(h, mode | 0x0004 /* VIRTUAL_TERMINAL_PROCESSING */))
            return 0;
        SetConsoleOutputCP(65001); /* UTF-8 */
    }
#endif
    indic__ansi_on = 1;
    indic__color_on = getenv("NO_COLOR") == NULL;
    atexit(indic__cleanup);
    signal(SIGINT, indic__on_sigint);
    return 1;
}

const char *indic_style(const char *s) {
    return indic__color_on ? s : "";
}

int indic_term_width(void) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO i;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &i))
        return i.srWindow.Right - i.srWindow.Left + 1;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
        return w.ws_col;
#endif
    return 80;
}

size_t indic_progress_str_size(int width) {
    const size_t glyph_bytes = sizeof(INDIC__FULL) - 1;
    size_t bar_width;

    if (width < 1) width = 1;
    bar_width = (size_t)width;
    /* '[' + glyphs + "] 100%" + NUL */
    if (bar_width > ((size_t)-1 - 8) / glyph_bytes) return 0;
    return 8 + bar_width * glyph_bytes;
}

int indic_progress_str(char *buf, size_t buf_size, double frac, int width) {
    int i, filled;
    char *p;
    size_t needed;

    if (buf && buf_size) buf[0] = '\0';
    needed = indic_progress_str_size(width);
    if (!buf || !needed || buf_size < needed) return -1;

    p = buf;
    if (width < 1) width = 1;
    if (frac != frac || frac < 0) frac = 0; /* NaN and negative values */
    if (frac > 1) frac = 1;
    filled = (int)(frac * width + 0.5);
    *p++ = '[';
    for (i = 0; i < width; i++) {
        const char *s = i < filled ? INDIC__FULL : INDIC__EMPTY;
        size_t n = strlen(s);
        memcpy(p, s, n);
        p += n;
    }
    snprintf(p, buf_size - (size_t)(p - buf), "] %3d%%",
             (int)(frac * 100 + 0.5));
    return 0;
}

void indic_progress(double frac, int width) {
    char *buf;
    size_t buf_size;
    if (width <= 0) {
        width = indic_term_width() - 10;
        if (width > 50) width = 50;
        if (width < 10) width = 10;
    }
    buf_size = indic_progress_str_size(width);
    if (!buf_size) return;
    buf = (char *)malloc(buf_size);
    if (!buf) return;
    if (indic_progress_str(buf, buf_size, frac, width) != 0) {
        free(buf);
        return;
    }
    if (indic__ansi_on) {
        printf("\r\033[K%s", buf);
        fflush(stdout);
        if (frac >= 1.0) putchar('\n');
    } else if (frac >= 1.0) {
        printf("%s\n", buf);
    }
    free(buf);
}

char indic_spinner_frame(int tick) {
    static const char frames[4] = { '|', '/', '-', '\\' };
    return frames[((tick % 4) + 4) % 4];
}

void indic_spinner_tick(const char *msg) {
    if (!indic__ansi_on) return;
    printf("\r\033[K%c %s", indic_spinner_frame(indic__spinner_tick++), msg);
    fflush(stdout);
}

void indic_spinner_done(const char *msg) {
    if (indic__ansi_on)
        printf("\r\033[K%s\xE2\x9C\x94%s %s\n",
               indic_style(INDIC_GREEN), indic_style(INDIC_RESET), msg);
    else
        printf("done: %s\n", msg);
    fflush(stdout);
}

#ifdef __cplusplus
}
#endif

#endif /* INDIC_IMPLEMENTATION_INCLUDED */
#endif /* INDIC_IMPLEMENTATION */
