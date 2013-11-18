#include "utils.h"
#include "global.h"

int logOfTwo(int n) {
    int i = 0;
    if (n <= 0) {
        return 0;
    }
    n >>= 1;
    while (n) {
        n >>= 1;
        ++i;
    }
    return i;
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

char* allocString(const char* name) {
    char* p;
    if (name == 0) {
        return 0;
    }
    p = (char*) malloc(strlen(name) + 1);
    strcpy(p, name);
    return p;
}

// TODO delete
// char* augment_name(const char* name, const char* plus) {
//     int n = strlen(name) + 1 + strlen(plus) + 1;
//     char* p = (char*) malloc(n);
//     sprintf(p, "%s %s", name, plus);
//     return p;
// }

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

