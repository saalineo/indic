#define INDIC_IMPLEMENTATION
#include "../indic.h"

int main(void) {
    int tick;

    indic_init();
    for (tick = 0; tick < 12; ++tick)
        indic_spinner_tick("working");
    indic_spinner_done("complete");
    return 0;
}
