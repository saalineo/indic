#define INDIC_FORCE_TTY
#define INDIC_IMPLEMENTATION
#include "../indic.h"

#include <assert.h>
#include <pthread.h>

typedef struct worker_args {
    indic_context *context;
    const char *message;
} worker_args;

static void *tick_spinner(void *opaque) {
    worker_args *args = (worker_args *)opaque;
    int i;
    for (i = 0; i < 100; i++)
        indic_context_spinner_tick(args->context, args->message);
    return NULL;
}

int main(void) {
    FILE *left = tmpfile();
    FILE *right = tmpfile();
    indic_context *first;
    indic_context *second;
    pthread_t workers[2];
    worker_args args[2];
    char output[8192] = { 0 };
    size_t bytes;

    assert(left != NULL && right != NULL);
    first = indic_context_create(left);
    second = indic_context_create(right);
    assert(first != NULL && second != NULL);
    assert(indic_context_init(first) == 1);
    assert(indic_context_init(second) == 1);
    assert(indic_context_set_output(first, stdout) == -1);

    /* Spinner frames are owned by each context, not the process. */
    indic_context_spinner_tick(first, "first");
    indic_context_spinner_tick(first, "first");
    indic_context_spinner_tick(second, "second");
    fflush(left);
    rewind(left);
    bytes = fread(output, 1, sizeof output - 1, left);
    output[bytes] = '\0';
    assert(strstr(output, "\033[K| first") != NULL);
    assert(strstr(output, "\033[K/ first") != NULL);

    fflush(right);
    rewind(right);
    bytes = fread(output, 1, sizeof output - 1, right);
    output[bytes] = '\0';
    assert(strstr(output, "\033[K| second") != NULL);

    /* Simultaneous calls on a shared context preserve its spinner state. */
    args[0].context = first;
    args[0].message = "worker-a";
    args[1].context = first;
    args[1].message = "worker-b";
    assert(pthread_create(&workers[0], NULL, tick_spinner, &args[0]) == 0);
    assert(pthread_create(&workers[1], NULL, tick_spinner, &args[1]) == 0);
    assert(pthread_join(workers[0], NULL) == 0);
    assert(pthread_join(workers[1], NULL) == 0);

    indic_context_destroy(first);
    indic_context_destroy(second);
    fclose(left);
    fclose(right);
    return 0;
}
