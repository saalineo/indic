#define INDIC_IMPLEMENTATION
#include "../indic.h"

int main(void) {
    int percent;

    indic_init();
    for (percent = 0; percent <= 100; percent += 10)
        indic_progress(percent / 100.0, 30);
    return 0;
}
