#pragma once
#include "cds.h"
#include <stdio.h>

CacheDescription* readCacheDescriptionFileEntry(FILE*);
void readCacheDescriptions(char*);
