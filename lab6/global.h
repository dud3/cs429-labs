#pragma once
#define _GNU_SOURCE  // For strcasestr
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#define TRUE 1
#define FALSE 0

extern char debug;
FILE* debugFile;
/* A memory address is a 32-bit integer */

enum memory_access_type  { MAT_LOAD, MAT_STORE, MAT_FETCH };
#define NUMBER_OF_MEMORY_ACCESS_TYPE 3

