CC     ?= gcc
CXX    ?= g++
FLAGS  = -Wall -Wextra -pedantic -O2

all: demo test

demo: examples/demo.c indic.h
	$(CC) -std=c99 $(FLAGS) examples/demo.c -o demo

test: tests/test.c indic.h
	$(CC) -std=c99 $(FLAGS) tests/test.c -o test_c
	$(CXX) -x c++ $(FLAGS) tests/test.c -o test_cpp
	./test_c | cat
	./test_cpp | cat

WINCC  ?= x86_64-w64-mingw32-gcc
WINCXX ?= x86_64-w64-mingw32-g++

win:
	$(WINCC) -std=c99 $(FLAGS) examples/demo.c -o demo.exe
	$(WINCC) -std=c99 $(FLAGS) tests/test.c -o test.exe
	$(WINCXX) -x c++ $(FLAGS) tests/test.c -o test_cpp.exe

clean:
	rm -f demo test_c test_cpp demo.exe test.exe test_cpp.exe

.PHONY: all test win clean
