uname := $(shell uname)
suf := c
headers := $(wildcard *.h)
sources := $(wildcard *.$(suf))
objects := $(addsuffix .o, $(basename $(sources)))
ifeq ($(uname), Darwin)
cxx := gcc-mp-4.9
else
cxx := gcc
endif
cxxflags := -g -Wall -Wextra -std=c11

main: $(objects)
	$(cxx) $^ $(cxxflags) -o main
%.o: %.$(suf) $(headers)
	$(cxx) -c -o $@ $< $(cxxflags)
.PHONY: clean
clean:
	rm -rf main main.dSYM *.o
test: main
	./main four.txt > tmp
	diff tmp four.5b
	./main -d < four.5b > tmp
	diff tmp four.txt
	./main gettysburg.txt > tmp
	diff tmp gettysburg.5b
	./main -d < gettysburg.5b > tmp
	diff tmp gettysburg.txt
	./main smile.gif > tmp
	diff tmp smile.5b
	./main -d < smile.5b > tmp
	diff tmp smile.gif
	rm -f tmp
