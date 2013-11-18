#pragma once
#include "global.h"
#include "cds.h"

extern void readCacheDescriptions(char*);
extern void debugPrintCds(CacheDescription);
extern void printCacheStatistics();
extern int countDirtyLines(Cache*);
extern void initCaches();
extern void initCachesForTrace();
extern void deleteCaches();
extern void simulateCaches(char*);

