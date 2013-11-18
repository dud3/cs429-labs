#include "utils.h"
#include "global.h"

int log2(int n) {
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
    int m;
    if (n <= 0) {
        return 0;
    }
    m = (1 << n) - 1;
    return m;
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

char ishex(int c)
{
    if (('0' <= c) && (c <= '9')) return 1;
    if (('a' <= c) && (c <= 'f')) return 1;
    if (('A' <= c) && (c <= 'F')) return 1;
    return 0;
}

int hexvalue(int c)
{
    if (('0' <= c) && (c <= '9')) return c - '0';
    if (('a' <= c) && (c <= 'f')) return c - 'a' + 10;
    if (('A' <= c) && (c <= 'F')) return c - 'A' + 10;
    return -1;
}

int decvalue(int c)
{
    if (('0' <= c) && (c <= '9')) return c - '0';
    return -1;
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* MALLOC space for a string and copy it */

char* remember_string(const char* name)
{
    size_t n;
    char* p;

    if (name == NULL) return NULL;

    /* get memory to remember file name */
    n = strlen(name) + 1;
    p = (char*) malloc(n);
    strcpy(p, name);
    return p;
}

char* augment_name(const char* name, const char* plus)
{
    int n = strlen(name) + 1 + strlen(plus) + 1;
    char* p = (char*) malloc(n);
    sprintf(p, "%s %s", name, plus);
    return p;
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

int skip_blanks(FILE *file)
{
    int c;
    while (((c = getc(file)) != EOF) && isspace(c)) /* keep reading */;
    return c;
}


int skip_line(FILE *file)
{
    int c;
    while (((c = getc(file)) != EOF) && (c != '\n')) /* keep reading */;
    c = skip_blanks(file);
    return c;
}


