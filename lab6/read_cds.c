#include "global.h"
#include "utils.h"
#include "cds.h"

#define MIN_TOKEN_SIZE 128

// TODO Hide implementation in .h files
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
        t->length = 2 * t->length;
        t->value = (char*) realloc(t->value, t->length);
        if (t->value == 0) {
            fprintf(stderr, "Cannot allocate memory\n");
            exit(-1);
        }
    }
    t->value[i] = c;
}

void getToken(FILE* cacheDescriptionFile, Token *t) {
    int c;
    int i = 0;
    putCharInTokenAt(t, '\0', i);
    c = skipBlanks(cacheDescriptionFile);
    if (c == EOF) return;

    while (isalnum(c) || (c == '_'))
    {
        putCharInTokenAt(t, c, i);
        i = i + 1;
        putCharInTokenAt(t, '\0', i);
        c = getc(cacheDescriptionFile);
    }
    ungetc(c, cacheDescriptionFile);
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* Syntax for Cache Descriptions:  { key=value, key=value, ... } */
/* So, we read a key and a value and define the field of the
   cacheDescription defined by the key to have the given value. */

int getKeyValuePair(FILE *cacheDescriptionFile, Token *key, Token *value)
{
    int c;

    /* skip initial spaces */
    c = skipBlanks(cacheDescriptionFile);
    if (c == EOF) return EOF;
    if (c == '}') return EOF;

    /* went one too far, put it back */
    ungetc(c, cacheDescriptionFile);

    /* we want a string for the key */
    getToken(cacheDescriptionFile, key);

    /* skip spacing, look for "=" */
    c = skipBlanks(cacheDescriptionFile);
    if (c == EOF) return EOF;
    if ((c != '=') && (c != ':') && (c != '-'))
    {
        fprintf(stderr, "not key=value pair: %s %c\n", key->value, c);
        return EOF;
    }

    /* we want a second string for the value */
    getToken(cacheDescriptionFile, value);

    /* skip spacing, look for "," */
    c = skipBlanks(cacheDescriptionFile);
    if (c == EOF) return EOF;
    if ((c != ',') && (c != ';') && (c != '}'))
    {
        fprintf(stderr, "not key=value pair: %s %c\n", key->value, c);
        return EOF;
    }
    if (c == '}')
    {
        /* we have the last pair, terminated by a '}'.
           put it back, so that this last pair is processed */
        ungetc(c, cacheDescriptionFile);
        return ',';
    }

    return c;
}


void defineKeyValuePair(CacheDescription* cacheDescription, Token* key, Token* value) {
    if (debug) {
        fprintf(debugFile, "define %s = %s \n", key->value, value->value);
    }
    /* look for the name */
    if (strcasestr(key->value, "name") != 0) {
        if (cacheDescription->name != 0) {
            free(cacheDescription->name);
        }
        cacheDescription->name = allocateString(value->value);
        return;
    }

    /* look for line size */
    if ((strcasestr(key->value, "line") != 0) && (strcasestr(key->value, "size") != 0))
    {
        int n = atoi(value->value);
        cacheDescription->cache->cacheLineSize = n;
        return;
    }

    /* look for number of cache entries */
    if (strcasestr(key->value, "entries") != 0)
    {
        int n = atoi(value->value);
        cacheDescription->cache->entries = n;
        return;
    }

    /* look for the number of ways */
    if (strcasestr(key->value, "ways") != 0)
    {
        int n = atoi(value->value);
        cacheDescription->cache->numberOfWays = n;
        return;
    }

    /* look for write-back */
    if ((strcasestr(key->value, "write") != 0) && (strcasestr(key->value, "back") != 0))
    {
        if (strcasestr(value->value, "true") != 0)
        {
            cacheDescription->cache->writeBack = 1;
            return;
        }
        if (strcasestr(value->value, "false") != 0)
        {
            cacheDescription->cache->writeBack = 0;
            return;
        }
    }

    /* look for write-thru */
    if ((strcasestr(key->value, "write") != 0) && (strcasestr(key->value, "thru") != 0))
    {
        if (strcasestr(value->value, "true") != 0)
        {
            cacheDescription->cache->writeBack = 0;
            return;
        }
        if (strcasestr(value->value, "false") != 0)
        {
            cacheDescription->cache->writeBack = 1;
            return;
        }
    }

    /* look for the replacement policy */
    if ((strcasestr(key->value, "policy") != 0) || (strcasestr(key->value, "replace") != 0))
    {
        if (strcasestr(value->value, "LRU") != 0)
        {
            cacheDescription->cache->replacement_policy = CRP_LRU;
            return;
        }
        if (strcasestr(value->value, "LFU") != 0)
        {
            cacheDescription->cache->replacement_policy = CRP_LFU;
            return;
        }
        if (strcasestr(value->value, "random") != 0)
        {
            cacheDescription->cache->replacement_policy = CRP_RANDOM;
            return;
        }
        if (strcasestr(value->value, "FIFO") != 0)
        {
            cacheDescription->cache->replacement_policy = CRP_FIFO;
            return;
        }
    }

    /* look for line size */
    if ((strcasestr(key->value, "decay") != 0) && (strcasestr(key->value, "interval") != 0)) {
        int n = atoi(value->value);
        cacheDescription->cache->LFU_Decay_Interval = n;
        return;
    }
    if (strcasestr(key->value, "ways") != 0) {
        int n = atoi(value->value);
        cacheDescription->cache->numberOfWays = n;
        return;
    }
    // Define victim cache
    if (strcasestr(key->value, "victim") != 0) {
        int n = atoi(value->value);
        cacheDescription->cache->victimCache.entries = n;
        return;
    }
    fprintf(stderr, "don't understand %s = %s\n",key->value, value->value);
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

CacheDescription* readCacheDescriptionFileEntry(FILE *cacheDescriptionFile) {
    int c;

    c = skipBlanks(cacheDescriptionFile);
    while (c == '#')
    {
        c = skipLine(cacheDescriptionFile);
    }
    if (c == EOF) return 0;

    /* Syntax for Cache Descriptions:  { key=value, key=value, ... } */
    /* So, we read a key and a value and define the field of the
       cacheDescription defined by the key to have the given value. */

    if (c != '{')
    {
        fprintf(stderr, "Cache description should start with {, not %c\n", c);
        return 0;
    }

    /* starting a new cache description.  Get a structure,
       and fill in default values. */
    CacheDescription* cacheDescription = (CacheDescription*) malloc(sizeof(CDS));
    cacheDescription->c = (Cache*) malloc(sizeof(Cache));
    cacheDescription->name = 0;

    /* default values */
    cacheDescription->cache->cacheLineSize = 64;
    cacheDescription->cache->entries = 1024;
    cacheDescription->cache->numberOfWays = 2;
    cacheDescription->cache->writeBack = 1;
    cacheDescription->cache->replacement_policy = CRP_FIFO;
    cacheDescription->cache->LFU_Decay_Interval = 200000;
    cacheDescription->cache->cacheLine = 0;
    cacheDescription->cache->victimCache.entries = 0;

    Token* key = createToken();
    Token* value = createToken();
    while (((c = getKeyValuePair(cacheDescriptionFile, key, value)) != EOF) && (c != '}')) {
        defineKeyValuePair(cacheDescription, key, value);
    }
    deleteToken(key);
    deleteToken(value);

    cacheDescription->cache->name = allocateString(cacheDescription->name);

    if (debug) {
        debugPrintCds(cacheDescription);
    }
    return cacheDescription;
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

void readCacheDescriptions(char* CDS_file_name)
{
    FILE *cacheDescriptionFile;
    CacheDescription*cacheDescription;

    /* open input file */
    cacheDescriptionFile = fopen(CDS_file_name, "r");
    if (cacheDescriptionFile == 0)
    {
        fprintf (stderr,"Cannot open CDS file %s\n", CDS_file_name);
    }
    while ((cacheDescription = readCacheDescriptionFileEntry(cacheDescriptionFile)) != 0)
    {
        /* we use a linked list for all the cache descriptions,
           but we want to keep the list in the same order tha
           we read them in.  Bummer. */
        if (cacheDescriptionRoot == 0)
        {
            cacheDescriptionRoot = cacheDescription;
        }
        else
        {
            CacheDescription*q = cacheDescriptionRoot;
            while (q->next != 0) q = q->next;
            q->next = cacheDescription;
        }
        cacheDescription->next = 0;
    }
    fclose(cacheDescriptionFile);
}
