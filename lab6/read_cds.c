
/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* Reading cache descriptions */

#include "global.h"
#include "utils.h"
#include "cds.h"

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* we want to read in a string of some unknown length */
#define MIN_TOKEN_SIZE 128

struct Token
{
    char* string;
    int    length;
};
typedef struct Token Token;

Token *new_token(void)
{
    Token *t = (Token*) malloc(sizeof(Token));
    t->length = MIN_TOKEN_SIZE;
    t->string = (char*) malloc(t->length);
    return t;
}

void delete_token(Token *t)
{
    free(t->string);
    free(t);
}

void put_char_in_token_at(Token *t, char c, int i)
{
    if (i >= t->length)
        {
            /* need more space */
            t->length = 2 * t->length;
            t->string = (char*) realloc(t->string, t->length);
            if (t->string == 0)
                {
                    fprintf(stderr, "Hell has frozen over!!!\n");
                    exit(-1);
                }
        }
    t->string[i] = c;
}


void get_token(FILE *CDS_file, Token *t)
{
    int c;

    /* get the next token in the input stream */
    int i = 0;

    /* token is empty to start */
    put_char_in_token_at(t, '\0', i);

    /* skip spacing, look for first character */
    c = skipBlanks(CDS_file);
    if (c == EOF) return;

    while (isalnum(c) || (c == '_'))
        {
            put_char_in_token_at(t, c, i);
            i = i + 1;
            put_char_in_token_at(t, '\0', i);
            c = getc(CDS_file);
        }

    /* went one too far, put it back */
    ungetc(c, CDS_file);
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* Syntax for Cache Descriptions:  { key=value, key=value, ... } */
/* So, we read a key and a value and define the field of the
   cacheDescription defined by the key to have the given value. */

int get_key_value_pair(FILE *CDS_file, Token *key, Token *value)
{
    int c;

    /* skip initial spaces */
    c = skipBlanks(CDS_file);
    if (c == EOF) return EOF;
    if (c == '}') return EOF;

    /* went one too far, put it back */
    ungetc(c, CDS_file);

    /* we want a string for the key */
    get_token(CDS_file, key);

    /* skip spacing, look for "=" */
    c = skipBlanks(CDS_file);
    if (c == EOF) return EOF;
    if ((c != '=') && (c != ':') && (c != '-'))
        {
            fprintf(stderr, "not key=value pair: %s %c\n", key->string, c);
            return EOF;
        }

    /* we want a second string for the value */
    get_token(CDS_file, value);

    /* skip spacing, look for "," */
    c = skipBlanks(CDS_file);
    if (c == EOF) return EOF;
    if ((c != ',') && (c != ';') && (c != '}'))
        {
            fprintf(stderr, "not key=value pair: %s %c\n", key->string, c);
            return EOF;
        }
    if (c == '}')
        {
            /* we have the last pair, terminated by a '}'.
               put it back, so that this last pair is processed */
            ungetc(c, CDS_file);
            return ',';
        }

    return c;
}


void defineKeyValuePair(CacheDescription* cacheDescription, Token* key, Token* value) {
    if (debug) {
        fprintf(debugFile, "define %s = %s \n", key->string, value->string);
    }
    /* look for the name */
    if (strcasestr(key->string, "name") != 0) {
        if (cacheDescription->name != 0) {
            free(cacheDescription->name);
        }
        cacheDescription->name = allocateString(value->string);
        return;
    }

    /* look for line size */
    if ((strcasestr(key->string, "line") != 0) && (strcasestr(key->string, "size") != 0))
        {
            int n = atoi(value->string);
            cacheDescription->cache->cacheLineSize = n;
            return;
        }

    /* look for number of cache entries */
    if (strcasestr(key->string, "entries") != 0)
        {
            int n = atoi(value->string);
            cacheDescription->cache->entries = n;
            return;
        }

    /* look for the number of ways */
    if (strcasestr(key->string, "ways") != 0)
        {
            int n = atoi(value->string);
            cacheDescription->cache->numberOfWays = n;
            return;
        }

    /* look for write-back */
    if ((strcasestr(key->string, "write") != 0) && (strcasestr(key->string, "back") != 0))
        {
            if (strcasestr(value->string, "true") != 0)
                {
                    cacheDescription->cache->writeBack = 1;
                    return;
                }
            if (strcasestr(value->string, "false") != 0)
                {
                    cacheDescription->cache->writeBack = 0;
                    return;
                }
        }

    /* look for write-thru */
    if ((strcasestr(key->string, "write") != 0) && (strcasestr(key->string, "thru") != 0))
        {
            if (strcasestr(value->string, "true") != 0)
                {
                    cacheDescription->cache->writeBack = 0;
                    return;
                }
            if (strcasestr(value->string, "false") != 0)
                {
                    cacheDescription->cache->writeBack = 1;
                    return;
                }
        }

    /* look for the replacement policy */
    if ((strcasestr(key->string, "policy") != 0) || (strcasestr(key->string, "replace") != 0))
        {
            if (strcasestr(value->string, "LRU") != 0)
                {
                    cacheDescription->cache->replacement_policy = CRP_LRU;
                    return;
                }
            if (strcasestr(value->string, "LFU") != 0)
                {
                    cacheDescription->cache->replacement_policy = CRP_LFU;
                    return;
                }
            if (strcasestr(value->string, "random") != 0)
                {
                    cacheDescription->cache->replacement_policy = CRP_RANDOM;
                    return;
                }
            if (strcasestr(value->string, "FIFO") != 0)
                {
                    cacheDescription->cache->replacement_policy = CRP_FIFO;
                    return;
                }
        }

    /* look for line size */
    if ((strcasestr(key->string, "decay") != 0) && (strcasestr(key->string, "interval") != 0)) {
        int n = atoi(value->string);
        cacheDescription->cache->LFU_Decay_Interval = n;
        return;
    }
    if (strcasestr(key->string, "ways") != 0) {
        int n = atoi(value->string);
        cacheDescription->cache->numberOfWays = n;
        return;
    }
    // Define victim cache
    if (strcasestr(key->string, "victim") != 0) {
        int n = atoi(value->string);
        cacheDescription->cache->victimCache.entries = n;
        return;
    }
    fprintf(stderr, "don't understand %s = %s\n",key->string, value->string);
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

CacheDescription* Read_CDS_file_entry(FILE *CDS_file) {
    int c;

    c = skipBlanks(CDS_file);
    while (c == '#')
        {
            c = skipLine(CDS_file);
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

    Token* key = new_token();
    Token* value = new_token();
    while (((c = get_key_value_pair(CDS_file, key, value)) != EOF) && (c != '}')) {
        defineKeyValuePair(cacheDescription, key, value);
    }
    delete_token(key);
    delete_token(value);

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
    FILE *CDS_file;
    CacheDescription*cacheDescription;

    /* open input file */
    CDS_file = fopen(CDS_file_name, "r");
    if (CDS_file == 0)
        {
            fprintf (stderr,"Cannot open CDS file %s\n", CDS_file_name);
        }
    while ((cacheDescription = Read_CDS_file_entry(CDS_file)) != 0)
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
    fclose(CDS_file);
}
