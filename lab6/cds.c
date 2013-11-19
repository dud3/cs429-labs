#include "global.h"
#include "cds.h"
#include <stdlib.h>

CacheDescription* cacheDescriptionRoot = 0;

const char* printSetsAndWays(Cache* cache) {
    if (cache->numberOfWays == 1) {
        return "direct-mapped";
    }
    if (cache->numberOfWays == cache->entries) {
        return "fully associative";
    }
    static char buffer[64];
    sprintf(buffer, "%d sets of %d ways", cache->entries / cache->numberOfWays, cache->numberOfWays);
    return buffer;
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

int percent(int a, int b) {
    if (!b) {
        return 0;
    }
    return a * 100 / b;
}

void printCacheStatisticsForCache(Cache* cache) {
    char buffer[1024];
    printf("%s: %d entries of lines of %d bytes; %s, %s, %s\n", cache->name, cache->entries, cache->cacheLineSize, printSetsAndWays(cache), cache->writeBack ? "write-back" : "write-thru", cacheReplacementPolicyName(cache, buffer));
    printf("%s: %d accesses, %d hits (%d%%), %d misses, %d miss reads, %d miss writes\n", cache->name, cache->totalCacheAccess, cache->totalCacheHits, percent(cache->totalCacheHits, cache->totalCacheAccess), cache->totalCacheMisses, cache->totalMissReads, cache->totalMissWrites);
    if (cache->writeBack) {
        printf("%s: %d dirty cache lines remain\n", cache->name, countDirtyLines(cache));
    }
}

void printCacheStatisticsForCacheDescription(CacheDescription* cacheDescription) {
    printf("      %d addresses (%d %s, %d %s, %d %s)\n", cacheDescription->numberOfMemoryReference, cacheDescription->numberOfType[FETCH], memoryAccessTypeName(FETCH), cacheDescription->numberOfType[LOAD], memoryAccessTypeName(LOAD), cacheDescription->numberOfType[STORE], memoryAccessTypeName(STORE));
    printCacheStatisticsForCache(cacheDescription->cache);
    printf("\n");
}

void printCacheStatistics() {
    CacheDescription* cacheDescription = cacheDescriptionRoot;
    while (cacheDescription) {
        printCacheStatisticsForCacheDescription(cacheDescription);
        cacheDescription = cacheDescription->next;
    }
}

int countDirtyLines(Cache* cache) {
    int n = 0;
    int i;
    for (i = 0; i < cache->entries; ++i) {
        if (cache->cacheLine[i].dirty) {
            ++n;
            if (debug) {
                fprintf(debugFile, "%s: Cache Line 0x%08X is dirty\n", cache->name, cache->cacheLine[i].tag);
            }
        }
    }
    return n;
}

void initCacheDescription(CacheDescription* cacheDescription) {
    cacheDescription->cache->cacheLine = (CacheLine*) calloc(cacheDescription->cache->entries, sizeof(CacheLine));
    cacheDescription->cache->victimCache.cacheLine = (CacheLine*) calloc(cacheDescription->cache->victimCache.entries, sizeof(CacheLine));
}

void initCaches() {
    CacheDescription* cacheDescription = cacheDescriptionRoot;
    while (cacheDescription) {
        initCacheDescription(cacheDescription);
        cacheDescription = cacheDescription->next;
    }
}

void initCacheDescriptionForTrace(CacheDescription* cacheDescription) {
    int i;
    cacheDescription->numberOfMemoryReference = 0;
    for (i = 0; i < NUMBER_OF_MEMORY_ACCESS_TYPE; ++i) {
        cacheDescription->numberOfType[i] = 0;
    }
    cacheDescription->cache->totalCacheAccess = 0;
    cacheDescription->cache->totalCacheHits = 0;
    cacheDescription->cache->totalCacheMisses = 0;
    cacheDescription->cache->totalMissReads = 0;
    cacheDescription->cache->totalMissWrites = 0;
    cacheDescription->cache->victimCache.totalCacheAccess = 0;
    cacheDescription->cache->victimCache.totalCacheHits = 0;
    cacheDescription->cache->victimCache.totalCacheMisses = 0;
    cacheDescription->cache->victimCache.totalMissReads = 0;
    cacheDescription->cache->victimCache.totalMissWrites = 0;
}

void initCachesForTrace() {
    CacheDescription* cacheDescription = cacheDescriptionRoot;
    while (cacheDescription) {
        initCacheDescriptionForTrace(cacheDescription);
        cacheDescription = cacheDescription->next;
    }
}

