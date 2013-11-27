#include "simulate.h"
#include "debug.h"
#include "utils.h"
#include "cds.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int address;
    int length;
    enum MemoryAccessType type;
} MemoryReference;

static int traceLineNumber;

void readReference(FILE* traceFile, MemoryReference* reference) {
    int c;
    int address;
    int n;
    c = skipBlanks(traceFile);
    address = 0;
    while (isHex(c)) {
        address = (address << 4) | hexValue(c);
        c = getc(traceFile);
    }
    if (c != ',') {
        fprintf(stderr, "bad trace file input at line %d: %c\n", traceLineNumber, c);
        exit(-1);
    }
    n = 0;
    c = getc(traceFile);
    while (isdigit(c)) {
        n = n * 10 + decValue(c);
        c = getc(traceFile);
    }
    /* skip to end of line */
    while ((c != '\n') && (c != EOF)) {
        c = getc(traceFile);
    }
    /* define reference fields */
    reference->address = address;
    reference->length = n;
}

int readTraceFile(FILE* traceFile, MemoryReference *reference) {
    int c;
    traceLineNumber = 0;
    while ((c = getc(traceFile)) != EOF) {
        ++traceLineNumber;
        while (isspace(c) && (c != EOF)) {
            c = getc(traceFile);
        }
        switch (c) {
            case 'I':
                reference->type = FETCH;
                readReference(traceFile, reference);
                return 'I';
            case 'M':
            case 'S':
                reference->type = STORE;
                readReference(traceFile, reference);
                return 'S';
            case 'L':
                reference->type = LOAD;
                readReference(traceFile, reference);
                return 'L';
        }
        // Not a reference line, skip
        while ((c != '\n') && (c != EOF)) {
            c = getc(traceFile);
        }
    }
    return EOF;
}

void checkForDecay(int time, Cache* cache) {
    if (cache->replacementPolicy != LFU) {
        return;
    }
    if (!(time % cache->lfuDecayInterval)) {
        int i;
        int entries = cache->entries;
        CacheLine* cacheLine = cache->cacheLine;
        for (i = 0; i < entries; ++i) {
            cacheLine[i].replacementData >>= 1;
        }
    }
}

int getBaseCacheAddress(Cache* cache, int address) {
    return address & (~mask(logOfTwo(cache->cacheLineSize)));
}

int computeSetIndex(Cache* cache, int address) {
    return ((address >> logOfTwo(cache->cacheLineSize)) & mask(logOfTwo(cache->entries / cache->numberOfWays))) * cache->numberOfWays;
}

int searchCacheFor(Cache* cache, int address, char* full) {
    int i;
    int cacheEntryIndex;
    int numberOfWays = cache->numberOfWays;
    CacheLine* cacheLine = cache->cacheLine;
    address = getBaseCacheAddress(cache, address);
    cacheEntryIndex = computeSetIndex(cache, address);
    *full = 1;
    for (i = 0; i < numberOfWays; ++i) {
        if (!cacheLine[cacheEntryIndex + i].valid) {
            *full = 0;
        } else if (address == cacheLine[cacheEntryIndex+i].tag) {
            return cacheEntryIndex + i;
        }
    }
    return -1;
}

int searchVictimCacheFor(Cache* cache, int address) {
    int i;
    CacheLine* cacheLine = cache->victimCache.cacheLine;
    address = getBaseCacheAddress(cache, address);
    for (i = 0; i < cache->victimCache.entries; ++i) {
        if (cacheLine[i].valid && (address == cacheLine[i].tag)) {
            return i;
        }
    }
    return -1;
}

int findVictimInCache(Cache* cache, int address) {
    int i;
    int victim;
    int min;
    int firstIndex = computeSetIndex(cache, address);
    int numberOfWays = cache->numberOfWays;
    CacheLine* cacheLine = cache->cacheLine;
    for (i = 0; i < numberOfWays; ++i) {
        if (!cacheLine[firstIndex + i].valid) {
            victim = firstIndex + i;
            return victim;
        }
    }
    // No empty cache
    victim = firstIndex;
    switch (cache->replacementPolicy) {
        case FIFO:
        case LRU:
        case LFU:
            min = cacheLine[firstIndex].replacementData;
            for (i = 1; i < numberOfWays; ++i) {
                if (cacheLine[firstIndex + i].replacementData < min) {
                    victim = firstIndex + i;
                    min = cacheLine[firstIndex + i].replacementData;
                }
            }
            break;
        case RANDOM:
            victim = firstIndex + (random() % numberOfWays);
            break;
    }
    return victim;
}

int findVictimInVictimCache(VictimCache* victimCache) {
    int i;
    int victim;
    int min;
    int entries = victimCache->entries;
    CacheLine* cacheLine = victimCache->cacheLine;
    for (i = 0; i < entries; ++i) {
        if (!(cacheLine[i].valid)) {
            return i;
        }
    }
    // No empty cache
    victim = 0;
    min = cacheLine[0].replacementData;
    for (i = 1; i < entries; ++i) {
        if (cacheLine[i].replacementData < min) {
            victim = i;
            min = cacheLine[i].replacementData;
        }
    }
    return victim;
}

void setReplacementData(int time, Cache* cache, CacheLine* cacheEntry) {
    switch (cache->replacementPolicy) {
        case FIFO:
            cacheEntry->replacementData = time;
            break;
        case LRU:
            cacheEntry->replacementData = time;
            break;
        case LFU:
            cacheEntry->replacementData = 0;
        case RANDOM:
            break;
    }
}

void updateReplacementData(int time, Cache* cache, CacheLine* cacheEntry) {
    switch (cache->replacementPolicy) {
        case FIFO:
            break;
        case LRU:
            cacheEntry->replacementData = time;
            break;
        case LFU:
            cacheEntry->replacementData += 1;
        case RANDOM:
            break;
    }
}

void swapCacheLines(CacheLine* a, CacheLine* b) {
    CacheLine tmp;
    memcpy(&tmp, a, sizeof(CacheLine));
    memcpy(a, b, sizeof(CacheLine));
    memcpy(b, &tmp, sizeof(CacheLine));
}

void simulateReferenceToCacheLine(CacheDescription* cacheDescription, MemoryReference* reference) {
    int cacheEntryIndex;
    int victim;
    char full;
    CacheLine* cacheEntry = 0;
    Cache* cache = cacheDescription->cache;
    ++cache->totalCacheAccess;
    cacheEntryIndex = searchCacheFor(cache, reference->address, &full); // Find in cache
    if (0 <= cacheEntryIndex) { // Found
        ++cache->totalCacheHits;
        cacheEntry = &cache->cacheLine[cacheEntryIndex];
        updateReplacementData(cacheDescription->numberOfMemoryReference, cache, cacheEntry);
    } else { // Not found
        ++cache->totalCacheMisses;
        ++cache->totalMissReads;
        if (full && cache->victimCache.entries) { // Cache is full and there is victim cache
            ++cache->victimCache.totalCacheAccess;
            victim = searchVictimCacheFor(cache, reference->address); // Go into victim cache
            if (0 <= victim) { // Found in victim cache, swap
                --cache->totalMissReads;
                ++cache->victimCache.totalCacheHits;
                cacheEntryIndex = findVictimInCache(cache, reference->address);
                cacheEntry = &cache->cacheLine[cacheEntryIndex];
                if (cacheEntry->valid && cacheEntry->dirty) {
                    ++cache->totalMissWrites;
                }
                swapCacheLines(cacheEntry, &cache->victimCache.cacheLine[victim]);
                cache->victimCache.cacheLine[victim].replacementData = cacheDescription->numberOfMemoryReference;
                setReplacementData(cacheDescription->numberOfMemoryReference, cache, cacheEntry);
            } else { // Not found in victim cache
                ++cache->victimCache.totalCacheMisses;
                ++cache->victimCache.totalMissReads;
                victim = findVictimInVictimCache(&cache->victimCache);
                cacheEntryIndex = findVictimInCache(cache, reference->address);
                cacheEntry = &cache->cacheLine[cacheEntryIndex];
                if (cacheEntry->valid && cacheEntry->dirty) {
                    ++cache->totalMissWrites;
                }
                if (cache->victimCache.cacheLine[victim].valid && cache->victimCache.cacheLine[victim].dirty) {
                    ++cache->victimCache.totalMissWrites;
                }
                swapCacheLines(cacheEntry, &cache->victimCache.cacheLine[victim]);
                cache->victimCache.cacheLine[victim].replacementData = cacheDescription->numberOfMemoryReference;
                cacheEntry->tag = getBaseCacheAddress(cache, reference->address);
                cacheEntry->valid = 1;
                cacheEntry->dirty = 0;
                setReplacementData(cacheDescription->numberOfMemoryReference, cache, cacheEntry);
            }
        } else { // Do not look into victim cache
            cacheEntryIndex = findVictimInCache(cache, reference->address);
            cacheEntry = &cache->cacheLine[cacheEntryIndex];
            if (cacheEntry->valid && cacheEntry->dirty) {
                ++cache->totalMissWrites;
            }
            cacheEntry->tag = getBaseCacheAddress(cache, reference->address);
            cacheEntry->valid = 1;
            cacheEntry->dirty = 0;
            setReplacementData(cacheDescription->numberOfMemoryReference, cache, cacheEntry);
        }
    }
    if (cache->replacementPolicy == LFU) {
        checkForDecay(cacheDescription->numberOfMemoryReference, cache);
    }
    if (reference->type == STORE) {
        if (cache->writeBack) {
            cacheEntry->dirty = 1;
        } else {
            ++cache->totalMissWrites;
        }
    }
}

void simulateReferenceToMemory(CacheDescription* cacheDescription, MemoryReference* reference) {
    ++cacheDescription->numberOfMemoryReference;
    ++cacheDescription->numberOfType[reference->type];
    // Check if the entire reference fits into just one cache line
    if (getBaseCacheAddress(cacheDescription->cache, reference->address) == getBaseCacheAddress(cacheDescription->cache, reference->address + reference->length -1)) {
        simulateReferenceToCacheLine(cacheDescription, reference);
    } else {
        MemoryReference reference1;
        MemoryReference reference2;
        reference2.type = reference->type;
        reference2.address = getBaseCacheAddress(cacheDescription->cache, reference->address + reference->length -1);
        reference2.length = reference->address + reference->length - reference2.address;
        reference1.type = reference->type;
        reference1.address = reference->address;
        reference1.length = reference->length - reference2.length;
        simulateReferenceToCacheLine(cacheDescription, &reference1);
        simulateReferenceToCacheLine(cacheDescription, &reference2);
    }
}

void simulateCaches(const char* traceFileName) {
    FILE* traceFile;
    MemoryReference reference;
    traceFile = fopen(traceFileName, "r");
    if (!traceFile) {
        fprintf (stderr,"Cannot open trace file %s\n", traceFileName);
        exit(1);
    }
    while (readTraceFile(traceFile, &reference) != EOF) {
        CacheDescription* cacheDescription = cacheDescriptionRoot;
        while (cacheDescription) {
            simulateReferenceToMemory(cacheDescription, &reference);
            cacheDescription = cacheDescription->next;
        }
    }
    fclose(traceFile);
}

