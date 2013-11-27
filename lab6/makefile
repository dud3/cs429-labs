output := cachesim
uname := $(shell uname)
suf := c
headers := $(wildcard *.h)
sources := $(wildcard *.$(suf))
objects := $(addsuffix .o, $(basename $(sources)))
ifeq ($(uname), Darwin)
cxx := gcc-mp-4.9
cxxflags := -g -O0 -Wall -Wextra
python := python
else
cxx := gcc
cxxflags := -O0 -Wall
python := python3
endif

$(output): $(objects)
	$(cxx) $^ $(cxxflags) -o $(output)
%.o: %.$(suf) $(headers)
	$(cxx) -c -o $@ $< $(cxxflags)
.PHONY: clean test test_fast
clean:
	@rm -rf $(output) main.dSYM *.o
test: $(output)
	@$(python) test.py
	@echo Test done
test_fast: $(output)
	@$(python) test.py fast
	@echo Test done
