#define INDIC_IMPLEMENTATION
#include "../indic.h"
#include <assert.h>

int main(void) {
    char buf[256];
    char too_small[1] = { 'x' };
    size_t exact_size;

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

    /* stdout is not a tty under test -> colors off, style empty */
    indic_init();
    assert(strcmp(indic_style(INDIC_RED), "") == 0 || INDIC_ISATTY(stdout));

    puts("all tests passed");
    return 0;
}
