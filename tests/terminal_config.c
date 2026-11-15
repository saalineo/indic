#define INDIC_IMPLEMENTATION
#include "../indic.h"
#include <assert.h>

int main(void) {
    assert(setenv("TERM", "dumb", 1) == 0);
    assert(unsetenv("NO_COLOR") == 0);

    assert(indic_init() == 0);
    assert(strcmp(indic_style(INDIC_RED), "") == 0);
    puts("terminal configuration test passed");
    return 0;
}
