# indic.h

A single-header C/C++ library for terminal colors, progress bars, spinners,
and cursor control. It depends only on libc and the platform terminal headers.

It works on Linux, macOS, and Windows; Windows console ANSI support is enabled
by `indic_init()`.

## Usage

Include `indic.h` where it is used. In exactly one translation unit (normally
`main.c`), define `INDIC_IMPLEMENTATION` before including it; that compilation
unit owns the shared initialization, color, cursor, and spinner state.
Initialize once near the start of `main`:

```c
#define INDIC_IMPLEMENTATION  /* main.c only */
#include "indic.h"

int main(void) {
    int i;

    indic_init();
    printf(INDIC_PAINT(INDIC_GREEN, "ok\n"));
    for (i = 0; i <= 100; i++) {
        indic_progress(i / 100.0, 30);
    }
    return 0;
}
```

Other source files include the header normally, without the definition:

```c
/* worker.c */
#include "indic.h"

void worker(void) {
    printf("%sworking%s\n", indic_style(INDIC_CYAN), indic_style(INDIC_RESET));
}
```

Do not define `INDIC_IMPLEMENTATION` in more than one source file: the linker
will correctly report duplicate library symbols. Defining it nowhere produces
undefined-symbol linker errors. This replaces the old header-local `static`
state, which gave every translation unit a different copy.

Terminal controls are disabled automatically when the selected output is not a
terminal or when `TERM=dumb`. Styling follows the same rules, and is also
disabled when `NO_COLOR` is set. `NO_COLOR` affects only SGR styling: on an
ANSI-capable terminal, progress-bar redraws, spinner animation, and cursor
hide/show remain available. The cursor and any active terminal styling are
restored at exit and after Ctrl+C on POSIX. The Ctrl+C path uses only
async-signal-safe terminal writes; Windows relies on normal-exit cleanup.

`INDIC_PAINT(INDIC_RED, "text")` always emits ANSI escapes. When output may be
piped, use `indic_style(INDIC_RED)` with `%s` instead. Available styles are
`INDIC_RED`, `INDIC_GREEN`, `INDIC_YELLOW`, `INDIC_BLUE`, `INDIC_MAGENTA`,
`INDIC_CYAN`, `INDIC_WHITE`, `INDIC_BOLD`, and `INDIC_DIM`.

## Progress bars

```c
void indic_progress(double frac, int width);
```

`indic_progress(0.5, 30)` redraws a 30-column progress bar and prints a
newline when `frac` reaches 1.0. Specify a width of `0` to fit the terminal.

For a formatted progress string, supply a buffer and its full capacity,
including the terminating NUL:

```c
size_t size = indic_progress_str_size(30);
char *buf = malloc(size);

if (buf != NULL && indic_progress_str(buf, size, 0.5, 30) == 0) {
    puts(buf);
}
free(buf);
```

```c
size_t indic_progress_str_size(int width);
int indic_progress_str(char *buf, size_t buf_size, double frac, int width);
```

`indic_progress_str_size()` returns the exact required capacity or `0` if the
width cannot be represented safely. `indic_progress_str()` returns `0` on
success and `-1` for an invalid or undersized buffer.

Define `INDIC_ASCII` before including the header to use `#` and `-` instead of
Unicode block characters.

### Rich progress updates

For a labelled progress line with elapsed time and an ETA, use:

```c
indic_progress_update("Downloading", 0.42);
```

The first update starts the timer.  A completed update prints a newline and
starts a fresh timer the next time it is called.  Use
`indic_progress_reset()` to start a fresh timer before completion.

When work cannot be measured, pass `INDIC_PROGRESS_INDETERMINATE` to display
an animated activity bar without an ETA:

```c
indic_progress_update("Waiting for server", INDIC_PROGRESS_INDETERMINATE);
```

Use the `_ex` form to choose the bar width or hide either time field:

```c
indic_progress_options options = INDIC_PROGRESS_OPTIONS_INIT;
options.width = 24;
options.show_eta = 0;
indic_progress_update_ex("Uploading", 0.42, &options);
```

## Contexts, streams, and threads

The original functions use one process-wide default context and remain fully
supported.  Create a context when a component needs its own stream or its own
spinner/progress state:

```c
FILE *log = fopen("worker.log", "w");
indic_context *worker = indic_context_create(log);

if (worker == NULL) {
    fclose(log);
    return 1;
}
indic_context_init(worker); /* returns 1 only when ANSI controls are usable */

indic_context_spinner_tick(worker, "fetching");
indic_context_progress_update(worker, "fetching", 0.5);
indic_context_spinner_done(worker, "fetched");
indic_context_destroy(worker);
fclose(log);
```

```c
indic_context *indic_context_create(FILE *output);
void indic_context_destroy(indic_context *context);
int indic_context_set_output(indic_context *context, FILE *output);
int indic_context_init(indic_context *context);
```

`indic_context_create()` requires an open stream.  `indic_context_set_output()`
can replace it only before initialization.  A context keeps an independent
spinner frame counter and rich-progress timer, so two contexts do not affect
each other's animation state.  Use the corresponding `indic_context_*`
functions for cursor, terminal-width, progress, and spinner operations.

Calls sharing one context are thread-safe.  Indic also serializes each complete
terminal write across contexts, including contexts that target the same stream.
This prevents garbled escape sequences, but it is not a multi-line terminal
layout manager: two live spinners on one terminal still redraw the same line.
Give independently visible spinners separate streams/terminals, or manage
their line layout in the application.  Do not destroy a context until all
threads using it have stopped.  Existing direct writes such as `fprintf()` are
outside Indic's lock and should be synchronized by the application if they
share the stream.

## Default terminal configuration

Output defaults to `stdout`. Select another live stream before initialization:

```c
if (indic_set_output(stderr) != 0) {
    /* output is invalid, or indic_init() has already run */
}
indic_init();
```

The selected stream must remain open until program exit. Define either option
before including `indic.h` when automatic detection is unsuitable:

| Option | Effect |
| --- | --- |
| `INDIC_FORCE_TTY` | Assumes the selected output accepts ANSI controls, bypassing both `isatty()` and `TERM=dumb`. |
| `INDIC_FORCE_COLOR` | Emits SGR styles even without ANSI terminal controls; it does not enable redraws or cursor control. |

`NO_COLOR` always disables styles, including when `INDIC_FORCE_COLOR` is set.

## Spinner and cursor API

```c
void indic_spinner_tick(const char *msg);
void indic_spinner_done(const char *msg);
void indic_cursor_hide(void);
void indic_cursor_show(void);
```

## Build and test

```sh
make       # build the demo and run C99/C++ tests
make win   # cross-compile Windows executables with MinGW
make clean
```

See `examples/demo.c` for a complete example and `tests/test.c` for tests.

## Examples

Every public feature also has a small standalone program in `examples/`. Build
all of them with `make examples`, or build one with, for example,
`make example-progress`.

| File | Demonstrates |
| --- | --- |
| `init.c` | `indic_init()` and ANSI detection |
| `colors.c` | `INDIC_PAINT` and all supplied style constants |
| `conditional_style.c` | `indic_style()` for output that is safe to pipe |
| `terminal_width.c` | `indic_term_width()` |
| `progress.c` | `indic_progress()` |
| `progress_string.c` | `indic_progress_str_size()` and `indic_progress_str()` |
| `spinner_frames.c` | `indic_spinner_frame()` |
| `spinner.c` | `indic_spinner_tick()` and `indic_spinner_done()` |
| `cursor.c` | `indic_cursor_hide()` and `indic_cursor_show()` |
| `ascii_progress.c` | the `INDIC_ASCII` option |

## License

GNU GPL v3 or later. See [LICENSE](LICENSE).
