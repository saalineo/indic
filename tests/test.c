#define INDIC_IMPLEMENTATION
#include "../indic.h"
#include <assert.h>

int main(void) {
    char buf[256];
    char too_small[1] = { 'x' };
    size_t exact_size;
    int spinner_tick;

    assert(indic_progress_str(buf, sizeof buf, 0.0, 4) == 0);
    assert(strcmp(buf, "[" INDIC__EMPTY INDIC__EMPTY INDIC__EMPTY INDIC__EMPTY "]   0%") == 0);

    assert(indic_progress_str(buf, sizeof buf, 0.5, 4) == 0);
    assert(strcmp(buf, "[" INDIC__FULL INDIC__FULL INDIC__EMPTY INDIC__EMPTY "]  50%") == 0);

    assert(indic_progress_str(buf, sizeof buf, 1.0, 4) == 0);
    assert(strcmp(buf, "[" INDIC__FULL INDIC__FULL INDIC__FULL INDIC__FULL "] 100%") == 0);

    /* clamping */
    assert(indic_progress_str(buf, sizeof buf, 5.0, 2) == 0);
    assert(strstr(buf, "100%"));
    assert(indic_progress_str(buf, sizeof buf, -1.0, 2) == 0);
    assert(strstr(buf, "  0%"));

    /* capacity is required and failure never writes past the buffer */
    assert(indic_progress_str(too_small, sizeof too_small, 0.5, 4) == -1);
    assert(too_small[0] == '\0');
    assert(indic_progress_str(NULL, 0, 0.5, 4) == -1);
    exact_size = indic_progress_str_size(4);
    assert(exact_size > 0 && exact_size <= sizeof buf);
    assert(indic_progress_str(buf, exact_size, 0.5, 4) == 0);
    assert(indic_progress_str_size(0) == indic_progress_str_size(1));

    /* spinner frames cycle */
    assert(indic_spinner_frame(0) == '|');
    assert(indic_spinner_frame(1) == '/');
    assert(indic_spinner_frame(4) == '|');
    assert(indic_spinner_frame(-1) == '\\');

    /* paint macro appends reset */
    assert(strcmp(INDIC_PAINT(INDIC_RED, "x"), "\033[31mx\033[0m") == 0);

    /* NO_COLOR disables styling but not ANSI terminal controls. */
    assert(setenv("NO_COLOR", "1", 1) == 0);
    assert(indic_init() == 1 || !INDIC_ISATTY(stdout));
    assert(strcmp(indic_style(INDIC_RED), "") == 0);
    if (INDIC_ISATTY(stdout)) {
        indic_cursor_hide();
        assert(indic__cursor_hidden == 1);
        indic_cursor_show();
        assert(indic__cursor_hidden == 0);
        spinner_tick = indic__spinner_tick;
        indic_spinner_tick("testing NO_COLOR controls");
        assert(indic__spinner_tick == spinner_tick + 1);
    }

    puts("all tests passed");
    return 0;
}
