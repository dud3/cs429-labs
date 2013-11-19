#pragma once
#include <stdio.h>

#define NUMBER_OF_MEMORY_ACCESS_TYPE 3

extern char debug;
extern FILE* debugFile;
enum MemoryAccessType {
    LOAD,
    STORE,
    FETCH
};

