#pragma once
#define _GNU_SOURCE  /* for strcasestr */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#define TRUE 1
#define FALSE 0

extern char debug;
FILE *debug_file;

/* A byte is just 8 bits */
typedef unsigned char byte;

/* A memory address is a 32-bit integer */
typedef int memory_address;


enum memory_access_type  { MAT_LOAD, MAT_STORE, MAT_FETCH };
#define NUMBER_OF_MEMORY_ACCESS_TYPE 3

