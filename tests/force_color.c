#define INDIC_FORCE_COLOR
#define INDIC_IMPLEMENTATION
#include "../indic.h"
#include <assert.h>

int main(void) {
    FILE *output = tmpfile();
    char buf[32] = { 0 };

    assert(output != NULL);
    assert(setenv("TERM", "dumb", 1) == 0);
    assert(unsetenv("NO_COLOR") == 0);
    assert(indic_set_output(output) == 0);
    assert(indic_init() == 0);
    assert(strcmp(indic_style(INDIC_RED), INDIC_RED) == 0);

    fprintf(output, "%sX%s", indic_style(INDIC_RED), indic_style(INDIC_RESET));
    fflush(output);
    rewind(output);
    assert(fread(buf, 1, sizeof buf - 1, output) == 10);
    assert(strcmp(buf, "\033[31mX\033[0m") == 0);
    return 0;
}
