#pragma once
#include "debug.h"
#include <stdio.h>
#include <stdint.h>

#define NUMBER_OF_MEMORY_ACCESS_TYPE 3

enum MemoryAccessType {
    LOAD,
    STORE,
    FETCH
};

uint32_t logOfTwo(const uint32_t);
int mask(int);
char isHex(int);
int hexValue(char);
int decValue(char);
char* allocateString(const char*);
int skipLine(FILE*);
int skipBlanks(FILE*);
const char* memoryAccessTypeName(enum MemoryAccessType);

