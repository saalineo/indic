CC     ?= gcc
CXX    ?= g++
FLAGS  = -Wall -Wextra -pedantic -O2
EXAMPLE_NAMES = init colors conditional_style terminal_width progress progress_string spinner_frames spinner cursor ascii_progress
EXAMPLE_BINS = $(addprefix example-,$(EXAMPLE_NAMES))

all: demo test

demo: examples/demo.c indic.h
	$(CC) -std=c99 $(FLAGS) examples/demo.c -o demo

examples: $(EXAMPLE_BINS)

example-%: examples/%.c indic.h
	$(CC) -std=c99 $(FLAGS) $< -o $@

test: tests/test.c tests/terminal_config.c tests/force_tty.c tests/force_color.c indic.h
	$(CC) -std=c99 $(FLAGS) tests/test.c -o test_c
	$(CXX) -x c++ $(FLAGS) tests/test.c -o test_cpp
	$(CC) -std=c99 $(FLAGS) tests/multifile_main.c tests/multifile_worker.c -o test_multifile_c
	$(CXX) -x c++ $(FLAGS) tests/multifile_main.c tests/multifile_worker.c -o test_multifile_cpp
	$(CC) -std=c99 $(FLAGS) tests/terminal_config.c -o test_terminal_config_c
	$(CXX) -x c++ $(FLAGS) tests/terminal_config.c -o test_terminal_config_cpp
	$(CC) -std=c99 $(FLAGS) tests/force_tty.c -o test_force_tty_c
	$(CXX) -x c++ $(FLAGS) tests/force_tty.c -o test_force_tty_cpp
	$(CC) -std=c99 $(FLAGS) tests/force_color.c -o test_force_color_c
	$(CXX) -x c++ $(FLAGS) tests/force_color.c -o test_force_color_cpp
	./test_c
	./test_cpp
	./test_multifile_c
	./test_multifile_cpp
	./test_terminal_config_c
	./test_terminal_config_cpp
	./test_force_tty_c
	./test_force_tty_cpp
	./test_force_color_c
	./test_force_color_cpp

WINCC  ?= x86_64-w64-mingw32-gcc
WINCXX ?= x86_64-w64-mingw32-g++

win:
	$(WINCC) -std=c99 $(FLAGS) examples/demo.c -o demo.exe
	$(WINCC) -std=c99 $(FLAGS) tests/test.c -o test.exe
	$(WINCXX) -x c++ $(FLAGS) tests/test.c -o test_cpp.exe
	$(WINCC) -std=c99 $(FLAGS) tests/multifile_main.c tests/multifile_worker.c -o test_multifile.exe
	$(WINCXX) -x c++ $(FLAGS) tests/multifile_main.c tests/multifile_worker.c -o test_multifile_cpp.exe

clean:
	rm -f demo $(EXAMPLE_BINS) test_c test_cpp test_multifile_c test_multifile_cpp test_terminal_config_c test_terminal_config_cpp test_force_tty_c test_force_tty_cpp test_force_color_c test_force_color_cpp demo.exe test.exe test_cpp.exe test_multifile.exe test_multifile_cpp.exe

.PHONY: all examples test win clean
