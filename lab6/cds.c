#include "cds.h"
#include "read_cds.h"
#include "utils.h"
#include <stdlib.h>

CacheDescription* cacheDescriptionRoot = 0;

const char* cacheReplacementPolicyName(Cache* cache, char* buffer) {
    switch(cache->replacementPolicy) {
        case FIFO:
            return "FIFO";
        case LRU:
            return "LRU";
        case RANDOM:
            return "RANDOM";
        case LFU: {
            sprintf(buffer, "LFU (decay=%d)", cache->lfuDecayInterval);
            return buffer;
        }
    };
    return "Invalid policy";
}

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

int percent(int a, int b) {
    if (!b) {
        return 0;
    }
    return a * 100 / b;
}

int countDirtyLinesForCache(Cache* cache) {
    int n = 0;
    int i;
    int entries = cache->entries;
    for (i = 0; i < entries; ++i) {
        if (cache->cacheLine[i].dirty) {
            ++n;
            }
        }
    }
    return n;
}

void printCacheStatisticsForCache(Cache* cache, const char* name) {
    char buffer[1024];
    printf("%s%s: %d entries of lines of %d bytes; %s, %s, %s\n", cache->name, name, cache->entries, cache->cacheLineSize, printSetsAndWays(cache), cache->writeBack ? "write-back" : "write-thru", cacheReplacementPolicyName(cache, buffer));
    printf("%s%s: %d accesses, %d hits (%d%%), %d misses, %d miss reads, %d miss writes\n", cache->name, cache->victimCache.entries ? " main cache" : "", cache->totalCacheAccess, cache->totalCacheHits, percent(cache->totalCacheHits, cache->totalCacheAccess), cache->totalCacheMisses, cache->totalMissReads, cache->totalMissWrites);
    if (cache->writeBack) {
        printf("%s%s: %d dirty cache lines remain\n", cache->name, cache->victimCache.entries ? " main cache" : "", countDirtyLinesForCache(cache));
    }
}

void printCacheStatisticsForCacheDescription(CacheDescription* cacheDescription) {
    printf("      %d addresses (%d %s, %d %s, %d %s)\n", cacheDescription->numberOfMemoryReference, cacheDescription->numberOfType[FETCH], memoryAccessTypeName(FETCH), cacheDescription->numberOfType[LOAD], memoryAccessTypeName(LOAD), cacheDescription->numberOfType[STORE], memoryAccessTypeName(STORE));
    if (cacheDescription->victimCache) {
        printCacheStatisticsForCache(cacheDescription->mainCache, " main cache");
        printCacheStatisticsForCache(cacheDescription->victimCache, " victim cache");
    } else {
        printCacheStatisticsForCache(cacheDescription->mainCache, "");
    }
    printf("\n");
}

void printCacheStatistics() {
    CacheDescription* cacheDescription = cacheDescriptionRoot;
    while (cacheDescription) {
        printCacheStatisticsForCacheDescription(cacheDescription);
        cacheDescription = cacheDescription->next;
    }
}

void initCacheDescription(CacheDescription* cacheDescription) {
    cacheDescription->mainCache->cacheLine = (CacheLine*) calloc(cacheDescription->mainCache->entries, sizeof(CacheLine));
    if (cacheDescription->victimCache) {
        cacheDescription->victimCache->cacheLine = (CacheLine*) calloc(cacheDescription->victimCache->entries, sizeof(CacheLine));
    }
}

void initCacheDescriptions() {
    CacheDescription* cacheDescription = cacheDescriptionRoot;
    while (cacheDescription) {
        initCacheDescription(cacheDescription);
        cacheDescription = cacheDescription->next;
    }
}

void initCacheDescriptionForTrace(CacheDescription* cacheDescription) {
    int i;
    cacheDescription->numberOfMemoryReference = 0;
    memset(cacheDescription->numberOfType, 0, sizeof(cacheDescription->numberOfType));
    for (i = 0; i < NUMBER_OF_MEMORY_ACCESS_TYPE; ++i) {
        cacheDescription->numberOfType[i] = 0;
    }
    cacheDescription->mainCache->totalCacheAccess = 0;
    cacheDescription->mainCache->totalCacheHits = 0;
    cacheDescription->mainCache->totalCacheMisses = 0;
    cacheDescription->mainCache->totalMissReads = 0;
    cacheDescription->mainCache->totalMissWrites = 0;
    if (cacheDescription->victimCache) {
        cacheDescription->victimCache->totalCacheAccess = 0;
        cacheDescription->victimCache->totalCacheHits = 0;
        cacheDescription->victimCache->totalCacheMisses = 0;
        cacheDescription->victimCache->totalMissReads = 0;
        cacheDescription->victimCache->totalMissWrites = 0;
    }
}

void initCacheDescriptionsForTrace() {
    CacheDescription* cacheDescription = cacheDescriptionRoot;
    while (cacheDescription) {
        initCacheDescriptionForTrace(cacheDescription);
        cacheDescription = cacheDescription->next;
    }
}

void deleteCache(Cache* cache) {
    free(cache->cacheLine);
    free(cache->name);
    free(cache);
}

void deleteCacheDescription(CacheDescription* cacheDescription) {
    free(cacheDescription->name);
    deleteCache(cacheDescription->mainCache);
    if (cacheDescription->victimCache) {
        deleteCache(cacheDescription->victimCache);
    }
    free(cacheDescription);
}

void deleteCacheDescriptions() {
    CacheDescription* cacheDescription = cacheDescriptionRoot;
    while (cacheDescription) {
        CacheDescription* old = cacheDescription;
        cacheDescription = cacheDescription->next;
        deleteCacheDescription(old);
    }
}

