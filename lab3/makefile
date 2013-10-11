uname := $(shell uname)
suf := c
headers := $(wildcard *.h)
sources := $(wildcard *.$(suf))
objects := $(addsuffix .o, $(basename $(sources)))
ifeq ($(uname), Darwin)
cxx := gcc-mp-4.9
cxxflags := -g -Wall -Wextra -std=c11
else
cxx := gcc
cxxflags := -g -Wall
endif

main: $(objects)
	$(cxx) $^ $(cxxflags) -o main
%.o: %.$(suf) $(headers)
	$(cxx) -c -o $@ $< $(cxxflags)
.PHONY: clean
clean:
	rm -rf main main.dSYM *.o
test: main
	@./main computers.fax computers.q > tmp
	@diff tmp computers.a
	@rm -f tmp
	@echo Test passed.
	
