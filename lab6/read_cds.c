#define _GNU_SOURCE // For strcasestr
#include "read_cds.h"
#include "utils.h"
#include "cds.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MIN_TOKEN_SIZE 128

typedef struct {
    int length;
    char* value;
} Token;

Token* createToken() {
    Token* t = (Token*) malloc(sizeof(Token));
    t->length = MIN_TOKEN_SIZE;
    t->value = (char*) malloc(t->length);
    return t;
}

void deleteToken(Token* t) {
    free(t->value);
    free(t);
}

void putCharInTokenAt(Token* t, char c, int i) {
    if (t->length <= i) {
        t->length <<= 1;
        t->value = (char*) realloc(t->value, t->length);
        if (!t->value) {
            fprintf(stderr, "Cannot allocate memory\n");
            exit(-1);
        }
    }
    t->value[i] = c;
}

void getToken(FILE* cacheDescriptionFile, Token *t) {
    int c;
    int i = 0;
    putCharInTokenAt(t, '\0', 0);
    c = skipBlanks(cacheDescriptionFile);
    if (c == EOF) {
        return;
    }
    while (isalnum(c) || (c == '_')) {
        putCharInTokenAt(t, c, i);
        ++i;
        c = getc(cacheDescriptionFile);
    }
    putCharInTokenAt(t, '\0', i);
    ungetc(c, cacheDescriptionFile);
}

int getKeyValuePair(FILE* cacheDescriptionFile, Token* key, Token* value) {
    int c;
    c = skipBlanks(cacheDescriptionFile);
    if (c == EOF) {
        return EOF;
    }
    if (c == '}') {
        return EOF;
    }
    ungetc(c, cacheDescriptionFile);
    getToken(cacheDescriptionFile, key);
    c = skipBlanks(cacheDescriptionFile);
    if (c == EOF) {
        return EOF;
    }
    if ((c != '=') && (c != ':') && (c != '-')) {
        fprintf(stderr, "not key=value pair: %s %c\n", key->value, c);
        return EOF;
    }
    getToken(cacheDescriptionFile, value);
    c = skipBlanks(cacheDescriptionFile);
    if (c == EOF) {
        return EOF;
    }
    if ((c != ',') && (c != ';') && (c != '}')) {
        fprintf(stderr, "not key=value pair: %s %c\n", key->value, c);
        return EOF;
    }
    if (c == '}') { // Reached last pair, but let it process this and terminate at the next cycle
        ungetc(c, cacheDescriptionFile);
    }
    return c;
}

void defineKeyValuePair(CacheDescription* cacheDescription, Token* key, Token* value) {
    if (strcasestr(key->value, "name")) {
        if (cacheDescription->name) {
            free(cacheDescription->name);
        }
        cacheDescription->name = allocateString(value->value);
        return;
    }
    if (strcasestr(key->value, "line") && strcasestr(key->value, "size")) {
        cacheDescription->mainCache->cacheLineSize = atoi(value->value);
        return;
    }
    if (strcasestr(key->value, "entries")) {
        cacheDescription->mainCache->entries = atoi(value->value);
        return;
    }
    if (strcasestr(key->value, "ways")) {
        cacheDescription->mainCache->numberOfWays = atoi(value->value);
        return;
    }
    if (strcasestr(key->value, "write") && strcasestr(key->value, "back")) {
        if (strcasestr(value->value, "true")) {
            cacheDescription->mainCache->writeBack = 1;
            return;
        }
        if (strcasestr(value->value, "false")) {
            cacheDescription->mainCache->writeBack = 0;
            return;
        }
    }
    if ((strcasestr(key->value, "write")) && (strcasestr(key->value, "thru"))) {
        if (strcasestr(value->value, "true")) {
            cacheDescription->mainCache->writeBack = 0;
            return;
        }
        if (strcasestr(value->value, "false")) {
            cacheDescription->mainCache->writeBack = 1;
            return;
        }
    }
    if ((strcasestr(key->value, "policy")) || (strcasestr(key->value, "replace"))) {
        if (strcasestr(value->value, "LRU")) {
            cacheDescription->mainCache->replacementPolicy = LRU;
            return;
        }
        if (strcasestr(value->value, "LFU")) {
            cacheDescription->mainCache->replacementPolicy = LFU;
            return;
        }
        if (strcasestr(value->value, "random")) {
            cacheDescription->mainCache->replacementPolicy = RANDOM;
            return;
        }
        if (strcasestr(value->value, "FIFO")) {
            cacheDescription->mainCache->replacementPolicy = FIFO;
            return;
        }
    }
    if ((strcasestr(key->value, "decay")) && (strcasestr(key->value, "interval"))) {
        cacheDescription->mainCache->lfuDecayInterval = atoi(value->value);
        return;
    }
    if (strcasestr(key->value, "ways")) {
        cacheDescription->mainCache->numberOfWays = atoi(value->value);
        return;
    }
    // Define victim cache
    if (strcasestr(key->value, "victim")) {
        if (cacheDescription->victimCache) {
            free(cacheDescription->victimCache);
        }
        cacheDescription->victimCache = (Cache*) malloc(sizeof(Cache));
        cacheDescription->victimCache->entries = atoi(value->value);
        return;
    }
    fprintf(stderr, "don't understand %s = %s\n",key->value, value->value);
}

CacheDescription* readCacheDescriptionFileEntry(FILE* cacheDescriptionFile) {
    int c;
    Token* key;
    Token* value;
    c = skipBlanks(cacheDescriptionFile);
    while (c == '#') {
        c = skipLine(cacheDescriptionFile);
    }
    if (c == EOF) {
        return 0;
    }
    if (c != '{') {
        fprintf(stderr, "Cache description should start with {, not %c\n", c);
        return 0;
    }
    CacheDescription* cacheDescription = (CacheDescription*) malloc(sizeof(CacheDescription));
    cacheDescription->name = 0;
    cacheDescription->mainCache = (Cache*) malloc(sizeof(Cache));
    cacheDescription->victimCache = 0;
    cacheDescription->next = 0;
    cacheDescription->mainCache->cacheLineSize = 64;
    cacheDescription->mainCache->numberOfWays = 2;
    cacheDescription->mainCache->entries = 1024;
    cacheDescription->mainCache->lfuDecayInterval = 200000;
    cacheDescription->mainCache->name = 0;
    cacheDescription->mainCache->cacheLine = 0;
    cacheDescription->mainCache->writeBack = 1;
    cacheDescription->mainCache->replacementPolicy = FIFO;
    key = createToken();
    value = createToken();
    while ((c = getKeyValuePair(cacheDescriptionFile, key, value)) != EOF) {
        defineKeyValuePair(cacheDescription, key, value);
    }
    deleteToken(key);
    deleteToken(value);
    cacheDescription->mainCache->name = allocateString(cacheDescription->name);
    if (cacheDescription->victimCache) {
        cacheDescription->victimCache->cacheLineSize = cacheDescription->mainCache->cacheLineSize;
        cacheDescription->victimCache->numberOfWays = cacheDescription->victimCache->entries;
        cacheDescription->victimCache->lfuDecayInterval = 0;
        cacheDescription->victimCache->name = allocateString(cacheDescription->name);
        cacheDescription->victimCache->cacheLine = 0;
        cacheDescription->victimCache->writeBack = 1;
        cacheDescription->victimCache->replacementPolicy = FIFO;
    }
    return cacheDescription;
}

void readCacheDescriptions(const char* cacheDescriptionFileName) {
    FILE* cacheDescriptionFile;
    CacheDescription* cacheDescription;
    cacheDescriptionFile = fopen(cacheDescriptionFileName, "r");
    if (!cacheDescriptionFile) {
        fprintf (stderr,"Cannot open CDS file %s\n", cacheDescriptionFileName);
    }
    while ((cacheDescription = readCacheDescriptionFileEntry(cacheDescriptionFile))) {
        if (!cacheDescriptionRoot) {
            cacheDescriptionRoot = cacheDescription;
        } else {
            CacheDescription* q = cacheDescriptionRoot;
            while (q->next) {
                q = q->next;
            }
            q->next = cacheDescription;
        }
        cacheDescription->next = 0;
    }
    fclose(cacheDescriptionFile);
}

