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

test: tests/test.c indic.h
	$(CC) -std=c99 $(FLAGS) tests/test.c -o test_c
	$(CXX) -x c++ $(FLAGS) tests/test.c -o test_cpp
	$(CC) -std=c99 $(FLAGS) tests/multifile_main.c tests/multifile_worker.c -o test_multifile_c
	$(CXX) -x c++ $(FLAGS) tests/multifile_main.c tests/multifile_worker.c -o test_multifile_cpp
	./test_c
	./test_cpp
	./test_multifile_c
	./test_multifile_cpp

WINCC  ?= x86_64-w64-mingw32-gcc
WINCXX ?= x86_64-w64-mingw32-g++

win:
	$(WINCC) -std=c99 $(FLAGS) examples/demo.c -o demo.exe
	$(WINCC) -std=c99 $(FLAGS) tests/test.c -o test.exe
	$(WINCXX) -x c++ $(FLAGS) tests/test.c -o test_cpp.exe
	$(WINCC) -std=c99 $(FLAGS) tests/multifile_main.c tests/multifile_worker.c -o test_multifile.exe
	$(WINCXX) -x c++ $(FLAGS) tests/multifile_main.c tests/multifile_worker.c -o test_multifile_cpp.exe

clean:
	rm -f demo $(EXAMPLE_BINS) test_c test_cpp test_multifile_c test_multifile_cpp demo.exe test.exe test_cpp.exe test_multifile.exe test_multifile_cpp.exe

.PHONY: all examples test win clean
