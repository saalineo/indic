#define INDIC_IMPLEMENTATION
#include "../indic.h"

int main(void) {
    indic_init();
    printf("%ssafe for terminals and pipes%s\n",
           indic_style(INDIC_GREEN), indic_style(INDIC_RESET));
    return 0;
}
