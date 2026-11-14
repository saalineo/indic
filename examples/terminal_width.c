#define INDIC_IMPLEMENTATION
#include "../indic.h"

int main(void) {
    printf("Terminal width: %d columns\n", indic_term_width());
    return 0;
}
