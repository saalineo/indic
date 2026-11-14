#define INDIC_IMPLEMENTATION
#include "../indic.h"

int main(void) {
    indic_init();
    indic_cursor_hide();
    puts("The cursor was hidden while this line was written.");
    indic_cursor_show();
    return 0;
}
