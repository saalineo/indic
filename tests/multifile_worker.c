#include "../indic.h"

#include <string.h>

int worker_observes_initialized_indic(void) {
    return strcmp(indic_style(INDIC_GREEN), INDIC_GREEN) == 0;
}
