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
	@./main -v test.obj > tmp
	@diff tmp test.out
	@./main -v all.obj > tmp
	@diff tmp all.out
	@./main prime.obj > tmp
	@diff tmp prime.out
	@rm -f tmp
	@echo Test done

