#define INDIC_IMPLEMENTATION
#include "../indic.h"

int main(void) {
    if (indic_init())
        puts("ANSI terminal output is enabled.");
    else
        puts("ANSI terminal output is disabled.");
    return 0;
}
