#define INDIC_IMPLEMENTATION
#include "../indic.h"

int main(void) {
    const int width = 20;
    size_t size = indic_progress_str_size(width);
    char *bar = (char *)malloc(size);

    if (bar == NULL || indic_progress_str(bar, size, 0.65, width) != 0) {
        free(bar);
        return 1;
    }
    puts(bar);
    free(bar);
    return 0;
}
