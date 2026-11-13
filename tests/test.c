#include "../indic.h"
#include <assert.h>

int main(void) {
    char buf[256];

    indic_progress_str(buf, 0.0, 4);
    assert(strcmp(buf, "[" INDIC__EMPTY INDIC__EMPTY INDIC__EMPTY INDIC__EMPTY "]   0%") == 0);

    indic_progress_str(buf, 0.5, 4);
    assert(strcmp(buf, "[" INDIC__FULL INDIC__FULL INDIC__EMPTY INDIC__EMPTY "]  50%") == 0);

    indic_progress_str(buf, 1.0, 4);
    assert(strcmp(buf, "[" INDIC__FULL INDIC__FULL INDIC__FULL INDIC__FULL "] 100%") == 0);

    /* clamping */
    indic_progress_str(buf, 5.0, 2);
    assert(strstr(buf, "100%"));
    indic_progress_str(buf, -1.0, 2);
    assert(strstr(buf, "  0%"));

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
