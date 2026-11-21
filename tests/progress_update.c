#define INDIC_FORCE_TTY
#define INDIC_IMPLEMENTATION
#include "../indic.h"
#include <assert.h>

int main(void) {
    FILE *output = tmpfile();
    char text[4096] = { 0 };
    char duration[16];
    indic_progress_options options = INDIC_PROGRESS_OPTIONS_INIT;

    assert(output != NULL);
    assert(indic_set_output(output) == 0);
    assert(indic_init() == 1);

    indic__format_duration(duration, sizeof duration, 1.0);
    assert(strcmp(duration, "00:00:01") == 0);

    indic_progress_update("Downloading", 0.5);
    indic_progress_update("Waiting", INDIC_PROGRESS_INDETERMINATE);
    options.width = 4;
    options.show_eta = 0;
    indic_progress_update_ex("Finishing", 1.0, &options);

    rewind(output);
    assert(fread(text, 1, sizeof text - 1, output) > 0);
    assert(strstr(text, "Downloading") != NULL);
    assert(strstr(text, " 50%") != NULL);
    assert(strstr(text, "elapsed") != NULL);
    assert(strstr(text, "eta") != NULL);
    assert(strstr(text, "Waiting") != NULL);
    assert(strstr(text, "Finishing") != NULL);
    assert(strstr(strstr(text, "Finishing"), "eta") == NULL);
    fclose(output);
    return 0;
}
