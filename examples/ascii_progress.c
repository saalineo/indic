#define INDIC_ASCII
#define INDIC_IMPLEMENTATION
#include "../indic.h"

int main(void) {
    char bar[32];

    if (indic_progress_str(bar, sizeof bar, 0.5, 10) != 0)
        return 1;
    puts(bar);
    return 0;
}
