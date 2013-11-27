#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

uint32_t logOfTwo(const uint32_t n) {
    uint32_t y;
    asm volatile ("bsr %1, %0\n" : "=r"(y) : "r"(n));
    return y;
}

int mask(int n) {
    if (n <= 0) {
        return 0;
    }
    return (1 << n) - 1;
}

char isHex(int c) {
    if (('0' <= c) && (c <= '9')) {
        return 1;
    }
    if (('a' <= c) && (c <= 'f')) {
        return 1;
    }
    if (('A' <= c) && (c <= 'F')) {
        return 1;
    }
    return 0;
}

int hexValue(char c) {
    if (('0' <= c) && (c <= '9')) {
        return c - '0';
    }
    if (('a' <= c) && (c <= 'f')) {
        return c - 'a' + 10;
    }
    if (('A' <= c) && (c <= 'F')) {
        return c - 'A' + 10;
    }
    return -1;
}

int decValue(char c) {
    if (('0' <= c) && (c <= '9')) {
        return c - '0';
    }
    return -1;
}

char* allocateString(const char* name) {
    char* p;
    if (!name) {
        return 0;
    }
    p = (char*) malloc(strlen(name) + 1);
    strcpy(p, name);
    return p;
}

int skipBlanks(FILE* file) {
    int c;
    while (((c = getc(file)) != EOF) && isspace(c)) {
    }
    return c;
}


int skipLine(FILE* file) {
    int c;
    while (((c = getc(file)) != EOF) && (c != '\n')) {
    }
    c = skipBlanks(file);
    return c;
}

const char* memoryAccessTypeName(enum MemoryAccessType type) {
    switch(type) {
        case FETCH:
            return "Fetch";
        case LOAD:
            return "Load";
        case STORE:
            return "Store";
    }
    return "invalid";
}

