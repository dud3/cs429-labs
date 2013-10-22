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
cxxflags := -Wall
endif

main: $(objects)
	$(cxx) $^ $(cxxflags) -o main
%.o: %.$(suf) $(headers)
	$(cxx) -c -o $@ $< $(cxxflags)
.PHONY: clean
clean:
	rm -rf main main.dSYM *.o
test: main
	@./main -v test.obj > tmp 2>&1
	@diff tmp test.out
	@./main -v all.obj > tmp 2>&1
	@diff tmp all.out
	@./main prime.obj > tmp 2>&1
	@diff tmp prime.out
	@rm -f tmp
	@echo Test done

