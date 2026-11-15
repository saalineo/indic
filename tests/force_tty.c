#define INDIC_FORCE_TTY
#define INDIC_IMPLEMENTATION
#include "../indic.h"
#include <assert.h>

int main(void) {
    FILE *output = tmpfile();
    char buf[128] = { 0 };

    assert(output != NULL);
    assert(setenv("TERM", "dumb", 1) == 0);
    assert(setenv("NO_COLOR", "1", 1) == 0);
    assert(indic_set_output(output) == 0);
    assert(indic_init() == 1);
    assert(strcmp(indic_style(INDIC_GREEN), "") == 0);

    indic_cursor_hide();
    indic_cursor_show();
    indic_spinner_tick("working");
    indic_spinner_done("complete");
    assert(indic_set_output(stdout) == -1);

    fflush(output);
    rewind(output);
    assert(fread(buf, 1, sizeof buf - 1, output) > 0);
    assert(strstr(buf, "\033[?25l") != NULL);
    assert(strstr(buf, "\033[K| working") != NULL);
    assert(strstr(buf, "\033[32m") == NULL);
    return 0;
}
