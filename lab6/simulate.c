#include "simulate.h"
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

int searchCacheFor(Cache* cache, int address) {
    int i;
    int cacheEntryIndex;
    int numberOfWays = cache->numberOfWays;
    CacheLine* cacheLine = cache->cacheLine;
    address = getBaseCacheAddress(cache, address);
    cacheEntryIndex = computeSetIndex(cache, address);
    for (i = 0; i < numberOfWays; ++i) {
        if (cacheLine[cacheEntryIndex + i].valid && address == cacheLine[cacheEntryIndex + i].tag) {
            return cacheEntryIndex + i;
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
            return firstIndex + i;
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
    CacheLine* cacheEntry = 0;
    Cache* mainCache = cacheDescription->mainCache;
    Cache* victimCache = cacheDescription->victimCache;
    ++mainCache->totalCacheAccess;
    cacheEntryIndex = searchCacheFor(mainCache, reference->address); // Find in cache
    cacheEntry = &mainCache->cacheLine[cacheEntryIndex];
    if (0 <= cacheEntryIndex) { // Found
        ++mainCache->totalCacheHits;
        updateReplacementData(cacheDescription->numberOfMemoryReference, mainCache, cacheEntry);
    } else { // Not found
        ++mainCache->totalCacheMisses;
        ++mainCache->totalMissReads;
        cacheEntryIndex = findVictimInCache(mainCache, reference->address);
        cacheEntry = &mainCache->cacheLine[cacheEntryIndex];
        if (cacheEntry->valid && victimCache) { // Cache is full and there is victim cache
            ++victimCache->totalCacheAccess;
            victim = searchCacheFor(victimCache, reference->address); // Go into victim cache
            if (cacheEntry->dirty) {
                ++mainCache->totalMissWrites;
            }
            if (0 <= victim) { // Found in victim cache, swap
                --mainCache->totalMissReads;
                ++victimCache->totalCacheHits;
                swapCacheLines(cacheEntry, &victimCache->cacheLine[victim]);
            } else { // Not found in victim cache
                ++victimCache->totalCacheMisses;
                ++victimCache->totalMissReads;
                victim = findVictimInCache(victimCache, reference->address);
                if (victimCache->cacheLine[victim].valid && victimCache->cacheLine[victim].dirty) {
                    ++victimCache->totalMissWrites;
                }
                swapCacheLines(cacheEntry, &victimCache->cacheLine[victim]);
                cacheEntry->tag = getBaseCacheAddress(mainCache, reference->address);
                cacheEntry->valid = 1;
                cacheEntry->dirty = 0;
            }
            setReplacementData(cacheDescription->numberOfMemoryReference, victimCache, &victimCache->cacheLine[victim]);
        } else { // Do not look into victim cache
            if (cacheEntry->valid && cacheEntry->dirty) {
                ++mainCache->totalMissWrites;
            }
            cacheEntry->tag = getBaseCacheAddress(mainCache, reference->address);
            cacheEntry->valid = 1;
            cacheEntry->dirty = 0;
        }
        setReplacementData(cacheDescription->numberOfMemoryReference, mainCache, cacheEntry);
    }
    if (mainCache->replacementPolicy == LFU) {
        checkForDecay(cacheDescription->numberOfMemoryReference, mainCache);
    }
    if (reference->type == STORE) {
        if (mainCache->writeBack) {
            cacheEntry->dirty = 1;
        } else {
            ++mainCache->totalMissWrites;
        }
    }
}

void simulateReferenceToMemory(CacheDescription* cacheDescription, MemoryReference* reference) {
    ++cacheDescription->numberOfMemoryReference;
    ++cacheDescription->numberOfType[reference->type];
    // Check if the entire reference fits into just one cache line
    if (getBaseCacheAddress(cacheDescription->mainCache, reference->address) == getBaseCacheAddress(cacheDescription->mainCache, reference->address + reference->length -1)) {
        simulateReferenceToCacheLine(cacheDescription, reference);
    } else {
        MemoryReference reference1;
        MemoryReference reference2;
        reference2.type = reference->type;
        reference2.address = getBaseCacheAddress(cacheDescription->mainCache, reference->address + reference->length -1);
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

