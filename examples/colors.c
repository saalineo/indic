#define INDIC_IMPLEMENTATION
#include "../indic.h"

int main(void) {
    indic_init();

    puts(INDIC_PAINT(INDIC_BOLD, "bold"));
    puts(INDIC_PAINT(INDIC_DIM, "dim"));
    puts(INDIC_PAINT(INDIC_RED, "red"));
    puts(INDIC_PAINT(INDIC_GREEN, "green"));
    puts(INDIC_PAINT(INDIC_YELLOW, "yellow"));
    puts(INDIC_PAINT(INDIC_BLUE, "blue"));
    puts(INDIC_PAINT(INDIC_MAGENTA, "magenta"));
    puts(INDIC_PAINT(INDIC_CYAN, "cyan"));
    puts(INDIC_PAINT(INDIC_WHITE, "white"));
    return 0;
}
