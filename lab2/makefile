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
