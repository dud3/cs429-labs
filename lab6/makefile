uname := $(shell uname)
suf := c
headers := $(wildcard *.h)
sources := $(wildcard *.$(suf))
objects := $(addsuffix .o, $(basename $(sources)))
ifeq ($(uname), Darwin)
cxx := gcc-mp-4.9
cxxflags := -g -O0 -Wall -Wextra -std=c11
else
cxx := gcc
cxxflags := -O0 -Wall
endif

cachesim: $(objects)
	$(cxx) $^ $(cxxflags) -o cachesim
%.o: %.$(suf) $(headers)
	$(cxx) -c -o $@ $< $(cxxflags)
.PHONY: clean
clean:
	rm -rf cachesim main.dSYM *.o
test: main
	@echo Test done

