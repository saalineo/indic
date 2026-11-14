#define INDIC_IMPLEMENTATION
#include "../indic.h"

int main(void) {
    int tick;

    for (tick = 0; tick < 8; ++tick)
        printf("frame %d: %c\n", tick, indic_spinner_frame(tick));
    return 0;
}
