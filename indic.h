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
 *     INDIC_ASCII        use '#' and '-' instead of block characters
 *     INDIC_FORCE_TTY    assume the selected output accepts ANSI controls
 *     INDIC_FORCE_COLOR  emit styles even when ANSI controls are unavailable
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
  #include <sys/time.h>
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
int indic_set_output(FILE *output);
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

/* Pass this as fraction to show activity when the amount of work is unknown. */
#define INDIC_PROGRESS_INDETERMINATE (-1.0)

/* Options for indic_progress_update_ex().  Width 0 fits the terminal. */
typedef struct indic_progress_options {
    int width;
    int show_elapsed;
    int show_eta;
} indic_progress_options;

#define INDIC_PROGRESS_OPTIONS_INIT { 0, 1, 1 }

void indic_progress_update(const char *label, double fraction);
void indic_progress_update_ex(const char *label, double fraction,
                              const indic_progress_options *options);
void indic_progress_reset(void);

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
static volatile sig_atomic_t indic__color_on = 0;
static volatile sig_atomic_t indic__cursor_hidden = 0;
static int indic__inited = 0;
static int indic__spinner_tick = 0;
static int indic__progress_active = 0;
static int indic__progress_tick = 0;
static double indic__progress_started_at = 0.0;
static FILE *indic__output = NULL;
#ifndef _WIN32
static int indic__output_fd = -1;
#endif

static FILE *indic__output_stream(void) {
    return indic__output ? indic__output : stdout;
}

static double indic__now(void) {
#ifdef _WIN32
    static LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    if (!frequency.QuadPart) QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return frequency.QuadPart ? (double)counter.QuadPart / frequency.QuadPart : 0.0;
#else
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) return 0.0;
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
#endif
}

static void indic__cleanup(void) {
    if (indic__ansi_on) {
        if (indic__cursor_hidden) {
            fputs("\033[?25h", indic__output_stream());
            indic__cursor_hidden = 0;
        }
        if (indic__color_on) fputs(INDIC_RESET, indic__output_stream());
        fflush(indic__output_stream());
    }
}

#ifndef _WIN32
/* A signal handler may not call stdio, malloc, atexit cleanup, or signal().
 * write(2) and _exit(2) are async-signal-safe on POSIX. */
static void indic__on_sigint(int sig) {
    if (indic__output_fd >= 0 && indic__cursor_hidden)
        (void)write(indic__output_fd, "\033[?25h", 6);
    if (indic__output_fd >= 0 && indic__color_on)
        (void)write(indic__output_fd, INDIC_RESET, 4);
    _exit(128 + sig);
}
#endif

int indic_set_output(FILE *output) {
    if (!output || indic__inited) return -1;
    indic__output = output;
    return 0;
}

void indic_cursor_hide(void) {
    if (!indic__ansi_on) return;
    fputs("\033[?25l", indic__output_stream());
    fflush(indic__output_stream());
    indic__cursor_hidden = 1;
}

void indic_cursor_show(void) {
    if (!indic__ansi_on) return;
    fputs("\033[?25h", indic__output_stream());
    fflush(indic__output_stream());
    indic__cursor_hidden = 0;
}

int indic_init(void) {
    FILE *output;
    const char *term;
    int terminal = 0;
#ifdef INDIC_FORCE_TTY
    const int force_tty = 1;
#else
    const int force_tty = 0;
#endif
#ifdef INDIC_FORCE_COLOR
    const int force_color = 1;
#else
    const int force_color = 0;
#endif

    if (indic__inited) return indic__ansi_on;
    indic__inited = 1;
    output = indic__output_stream();
    term = getenv("TERM");

    if ((force_tty || INDIC_ISATTY(output)) &&
        (force_tty || !term || strcmp(term, "dumb") != 0))
        terminal = 1;
#ifdef _WIN32
    if (terminal) {
        HANDLE h = (HANDLE)_get_osfhandle(_fileno(output));
        DWORD mode = 0;
        if (h == INVALID_HANDLE_VALUE || !GetConsoleMode(h, &mode) ||
            !SetConsoleMode(h, mode | 0x0004 /* VIRTUAL_TERMINAL_PROCESSING */)) {
            if (!force_tty) terminal = 0;
        } else {
            SetConsoleOutputCP(65001); /* UTF-8 */
        }
    }
#endif
    indic__ansi_on = terminal;
    indic__color_on = !getenv("NO_COLOR") && (terminal || force_color);
    if (!indic__ansi_on) return 0;
#ifndef _WIN32
    indic__output_fd = fileno(output);
#endif
    atexit(indic__cleanup);
#ifndef _WIN32
    if (indic__output_fd >= 0) signal(SIGINT, indic__on_sigint);
#endif
    return 1;
}

const char *indic_style(const char *s) {
    return indic__color_on ? s : "";
}

int indic_term_width(void) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO i;
    HANDLE h = (HANDLE)_get_osfhandle(_fileno(indic__output_stream()));
    if (h != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(h, &i))
        return i.srWindow.Right - i.srWindow.Left + 1;
#else
    struct winsize w;
    if (ioctl(fileno(indic__output_stream()), TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
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
        fprintf(indic__output_stream(), "\r\033[K%s", buf);
        fflush(indic__output_stream());
        if (frac >= 1.0) fputc('\n', indic__output_stream());
    } else if (frac >= 1.0) {
        fprintf(indic__output_stream(), "%s\n", buf);
    }
    free(buf);
}

static void indic__format_duration(char *buf, size_t buf_size, double seconds) {
    unsigned long long total;
    if (seconds != seconds || seconds < 0.0) seconds = 0.0;
    if (seconds > (double)-1) seconds = (double)-1;
    total = (unsigned long long)(seconds + 0.5);
    snprintf(buf, buf_size, "%02llu:%02llu:%02llu",
             total / 3600ULL, (total / 60ULL) % 60ULL, total % 60ULL);
}

void indic_progress_reset(void) {
    indic__progress_active = 0;
    indic__progress_tick = 0;
    indic__progress_started_at = 0.0;
}

void indic_progress_update(const char *label, double fraction) {
    indic_progress_update_ex(label, fraction, NULL);
}

void indic_progress_update_ex(const char *label, double fraction,
                              const indic_progress_options *options) {
    const char *name = label ? label : "";
    char elapsed_text[48], eta_text[48], details[128];
    char *line, *p;
    size_t bar_size, line_size, name_size;
    int width = 0, show_elapsed = 1, show_eta = 1;
    int indeterminate, complete, i, filled;
    double now, elapsed, eta = 0.0;

    if (options) {
        width = options->width;
        show_elapsed = options->show_elapsed != 0;
        show_eta = options->show_eta != 0;
    }
    if (width <= 0) {
        width = indic_term_width() - 10;
        if (width > 50) width = 50;
        if (width < 10) width = 10;
    }
    if (fraction != fraction) fraction = 0.0;
    indeterminate = fraction < 0.0;
    if (!indeterminate && fraction > 1.0) fraction = 1.0;

    now = indic__now();
    if (!indic__progress_active) {
        indic__progress_active = 1;
        indic__progress_started_at = now;
    }
    elapsed = now - indic__progress_started_at;
    if (elapsed < 0.0) elapsed = 0.0;

    details[0] = '\0';
    if (show_elapsed) {
        indic__format_duration(elapsed_text, sizeof elapsed_text, elapsed);
        snprintf(details + strlen(details), sizeof details - strlen(details),
                 " | elapsed %s", elapsed_text);
    }
    if (!indeterminate && show_eta && fraction > 0.0 && fraction < 1.0) {
        eta = elapsed * (1.0 - fraction) / fraction;
        indic__format_duration(eta_text, sizeof eta_text, eta);
        snprintf(details + strlen(details), sizeof details - strlen(details),
                 " | eta %s", eta_text);
    }

    bar_size = indic_progress_str_size(width);
    name_size = strlen(name);
    if (!bar_size || name_size > (size_t)-1 - bar_size - strlen(details) - 2)
        return;
    line_size = name_size + (name_size ? 1 : 0) + bar_size + strlen(details);
    line = (char *)malloc(line_size);
    if (!line) return;

    p = line;
    if (name_size) {
        memcpy(p, name, name_size);
        p += name_size;
        *p++ = ' ';
    }
    if (indeterminate) {
        *p++ = '[';
        for (i = 0; i < width; i++) {
            const char *glyph = i == indic__progress_tick % width
                ? INDIC__FULL : INDIC__EMPTY;
            size_t glyph_size = strlen(glyph);
            memcpy(p, glyph, glyph_size);
            p += glyph_size;
        }
        *p++ = ']';
        indic__progress_tick++;
    } else {
        filled = (int)(fraction * width + 0.5);
        *p++ = '[';
        for (i = 0; i < width; i++) {
            const char *glyph = i < filled ? INDIC__FULL : INDIC__EMPTY;
            size_t glyph_size = strlen(glyph);
            memcpy(p, glyph, glyph_size);
            p += glyph_size;
        }
        snprintf(p, line_size - (size_t)(p - line), "] %3d%%",
                 (int)(fraction * 100.0 + 0.5));
        p += strlen(p);
    }
    memcpy(p, details, strlen(details) + 1);

    complete = !indeterminate && fraction >= 1.0;
    if (indic__ansi_on) {
        fprintf(indic__output_stream(), "\r\033[K%s", line);
        if (complete) fputc('\n', indic__output_stream());
        fflush(indic__output_stream());
    } else if (complete) {
        fprintf(indic__output_stream(), "%s\n", line);
        fflush(indic__output_stream());
    }
    free(line);
    if (complete) indic_progress_reset();
}

char indic_spinner_frame(int tick) {
    static const char frames[4] = { '|', '/', '-', '\\' };
    return frames[((tick % 4) + 4) % 4];
}

void indic_spinner_tick(const char *msg) {
    if (!indic__ansi_on) return;
    fprintf(indic__output_stream(), "\r\033[K%c %s",
            indic_spinner_frame(indic__spinner_tick++), msg);
    fflush(indic__output_stream());
}

void indic_spinner_done(const char *msg) {
    if (indic__ansi_on)
        fprintf(indic__output_stream(), "\r\033[K%s\xE2\x9C\x94%s %s\n",
               indic_style(INDIC_GREEN), indic_style(INDIC_RESET), msg);
    else
        fprintf(indic__output_stream(), "done: %s\n", msg);
    fflush(indic__output_stream());
}

#ifdef __cplusplus
}
#endif

#endif /* INDIC_IMPLEMENTATION_INCLUDED */
#endif /* INDIC_IMPLEMENTATION */
