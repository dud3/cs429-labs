#pragma once
#define _GNU_SOURCE  // For strcasestr
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define NUMBER_OF_MEMORY_ACCESS_TYPE 3

extern char debug;
extern FILE* debugFile;
enum MemoryAccessType {
    LOAD,
    STORE,
    FETCH
};

