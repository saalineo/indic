# indic.h

A single-header C/C++ library for terminal colors, progress bars, spinners,
and cursor control. It depends only on libc and the platform terminal headers.

It works on Linux, macOS, and Windows; Windows console ANSI support is enabled
by `indic_init()`.

## Usage

Include `indic.h` before other system headers and initialize once near the
start of `main`:

```c
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

Colors are disabled automatically when standard output is not a terminal or
when `NO_COLOR` is set. The cursor and terminal colors are restored at exit and
after Ctrl+C.

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

## License

GNU GPL v3 or later. See [LICENSE](LICENSE).
