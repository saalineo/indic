#define INDIC_IMPLEMENTATION
#include "../indic.h"
#include <time.h>

static void sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    struct timespec t = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&t, NULL);
#endif
}

int main(void) {
    int i;
    indic_init();

    printf(INDIC_PAINT(INDIC_BOLD, "indic.h demo\n"));
    printf(INDIC_PAINT(INDIC_GREEN, "success") " "
           INDIC_PAINT(INDIC_YELLOW, "warning") " "
           INDIC_PAINT(INDIC_RED, "error") " "
           INDIC_PAINT(INDIC_DIM, "dim") "\n\n");

    indic_cursor_hide();
    for (i = 0; i < 20; i++) {
        indic_spinner_tick("Processing data...");
        sleep_ms(80);
    }
    indic_spinner_done("Processing complete");

    for (i = 0; i <= 100; i++) {
        indic_progress(i / 100.0, 30);
        sleep_ms(25);
    }
    indic_cursor_show();

    printf("Hello World.\n");
    return 0;
}
