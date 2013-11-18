#include "global.h"
#include "cds.h"
#include "caches.h"

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

const char* cacheReplacementPolicyName(Cache* cache, char* buffer) {
    switch(cache->replacement_policy) {
        case FIFO:
            return "FIFO";
        case LRU:
            return "LRU";
        case RANDOM:
            return "RANDOM";
        case LFU: {
            sprintf(buffer, "LFU (decay=%d)", cache->LFU_Decay_Interval);
            return buffer;
        }
    };
    return "Invalid policy";
}

void debugPrintCache(Cache *cache) {
    char buffer[1024];
    fprintf(debugFile, "%s: Total number of entries: %d\n", cache->name,  cache->entries);
    fprintf(debugFile, "%s: %s\n", cache->name,  printSetsAndWays(cache));
    fprintf(debugFile, "%s: Each cache line is %d bytes\n", cache->name,  cache->cacheLineSize);
    fprintf(debugFile, "%s: Cache is %s\n", cache->name,  cache->writeBack ? "write-back" : "write-thru");
    fprintf(debugFile, "%s: With a %s replacement policy\n", cache->name, cacheReplacementPolicyName(cache, buffer));
}


void debugPrintCacheDescription(CacheDescription* cacheDescription) {
    debugPrintCache(cds->cache);
}

int percent(int a, int b) {
    if (!b) {
        return 0;
    }
    return a * 100 / b;
}

void printCacheStatisticsForCache(Cache* cache) {
    printf("%s: %d entries of lines of %d bytes; %s, %s, %s\n", cache->name, cache->entries, cache->cacheLineSize, printSetsAndWays(cache), cache->writeBack ? "write-back" : "write-thru", cacheReplacementPolicyName(c));
    printf("%s: %d accesses, %d hits (%d%%), %d misses, %d miss reads, %d miss writes\n", cache->name, cache->totalCacheAccess, cache->totalCacheHits, percent(cache->totalCacheHits, cache->totalCacheAccess), cache->totalCacheMisses, cache->totalMissReads, cache->totalMissWrites);
    if (cache->writeBack) {
        printf("%s: %d dirty cache lines remain\n", cache->name, countDirtyLines(c));
    }
}

void printCacheStatisticsForCacheDescription(CacheDescription* cacheDescription) {
    printf("      %d addresses (%d %s, %d %s, %d %s)\n", cds->numberOfMemoryReference, cds->numberOfType[FETCH], memoryAccessTypeName(FETCH), cds->numberOfType[LOAD], memoryAccessTypeName(LOAD), cds->numberOfType[STORE], memoryAccessTypeName(STORE));
    printCacheStatisticsForCache(cacheDescription->cache);
    printf("\n");
}

void printCacheStatistics() {
    CacheDescription *cds = cacheDescriptionRoot;
    while (cds != 0)
        {
            printCacheStatistics_for_one_cds(cds);
            cds = cds->next;
        }
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */


void initCache(CacheDescription* cds) {
    /* we need one cache line for every entry */
    cds->c->c_line = (cache_line*) calloc(cds->c->entries, sizeof(cache_line));
    cds->c->victimCache.cacheLine = (VictimCacheLine*) calloc(cds->c->victimCache.entries, sizeof(VictimCacheLine));
}


void initCaches() {
    CacheDescription* cds = cacheDescriptionRoot;
    while (cds != 0) {
        initCache(cds);
        cds = cds->next;
    }
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

void initCacheForTrace(CacheDescription* cds) {
    int i;
    for (i = 0; i < NUMBER_OF_MEMORY_ACCESS_TYPE; ++i) {
        cds->numberOfType[i] = 0;
    }
    cds->numberOfMemoryReference = 0;
    cds->c->totalMissReads = 0;
    cds->c->totalMissWrites = 0;
    cds->c->totalCacheAccess = 0;
    cds->c->totalCacheHits = 0;
    cds->c->totalCacheMisses = 0;
    cds->c->victimCache.totalCacheAccess = 0;
    cds->c->victimCache.totalCacheHits = 0;
    cds->c->victimCache.totalCacheMisses = 0;
    cds->c->victimCache.totalMissReads = 0;
    cds->c->victimCache.totalMissWrites = 0;
}

void initCachesForTrace() {
    CacheDescription* cds = cacheDescriptionRoot;
    while (cds != 0) {
        initCacheForTrace(cds);
        cds = cds->next;
    }
}



/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */


void deleteCacheLine(struct cache* c) {
    free(c->c_line);
    free(c->name);
    if (c->victimCache.entries) {
        free(c->victimCache.cacheLine);
    }
}

void deleteCache(CacheDescription* cds) {
    deleteCacheLine(cds->c);
    free(cds->c);
    free(cds->name);
    free(cds);
}

void deleteCaches() {
    CacheDescription* cds = cacheDescriptionRoot;
    while (cds != 0) {
        CacheDescription* old = cds;
        cds = cds->next;
        deleteCache(old);
    }
}

