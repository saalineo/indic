/* Force ANSI on so the test is independent of the test runner's stdout. */
#define INDIC_ISATTY(f) 1
#define INDIC_IMPLEMENTATION
#include "../indic.h"

#include <assert.h>

int worker_observes_initialized_indic(void);

int main(void) {
    unsetenv("NO_COLOR");
    assert(indic_init() == 1);
    assert(worker_observes_initialized_indic());
    return 0;
}
