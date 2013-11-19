#include "simulate.h"
#include "global.h"
#include "utils.h"
#include "cds.h"

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
        if (debug) {
            fprintf(debugFile, "%s: LFU decay for all LFU counters\n", cache->name);
        }
        for (i = 0; i < cache->entries; ++i) {
            cache->cacheLine[i].replacementData >>= 1;
        }
    }
}

int getBaseCacheAddress(Cache* cache, int address) {
    return address & (~mask(logOfTwo(cache->cacheLineSize)));
}

int computeSetIndex(Cache* cache, int address) {
    return (address >> logOfTwo(cache->cacheLineSize)) & mask(logOfTwo(cache->entries / cache->numberOfWays)) * cache->numberOfWays;
}

int searchCacheFor(Cache* cache, int address) {
    int cacheEntryIndex;
    int i;
    address = getBaseCacheAddress(cache, address);
    cacheEntryIndex = computeSetIndex(cache, address);
    if (debug) {
        fprintf(debugFile, "%s: search cache lines %d to %d for 0x%08X\n", cache->name, cacheEntryIndex, cacheEntryIndex + cache->numberOfWays - 1, address);
    }
    for (i = 0; i < cache->numberOfWays; ++i) {
        if (cache->cacheLine[cacheEntryIndex + i].valid && (address == cache->cacheLine[cacheEntryIndex+i].tag)) {
            return cacheEntryIndex + i;
        }
    }
    return -1;
}

int searchVictimCacheFor(Cache* cache, int address) {
    int i;
    address = getBaseCacheAddress(cache, address);
    for (i = 0; i < cache->victimCache.entries; ++i) {
        if (cache->victimCache.cacheLine[i].valid && address == cache->victimCache.cacheLine[i].tag) {
            return i;
        }
    }
    return -1;
}

int findVictimInCache(Cache* cache, int address) {
    int i;
    int victim;
    int firstIndex = computeSetIndex(cache, address);
    int setSize = cache->numberOfWays;
    if (debug) {
        fprintf(debugFile, "%s: look for victim in %d lines starting at %d\n", cache->name,  setSize, firstIndex);
    }
    for (i = 0; i < setSize; ++i) {
        if (!cache->cacheLine[firstIndex+i].valid) {
            victim = firstIndex + i;
            if (debug) fprintf(debugFile, "%s: found empty cache entry at %d\n", cache->name,  victim);
            return victim;
        }
    }
    // No empty cache
    victim = firstIndex;
    switch (cache->replacementPolicy) {
        case FIFO:
        case LRU:
        case LFU: {
            int min = cache->cacheLine[firstIndex].replacementData;
            if (debug) {
                fprintf(debugFile, "%s: replacement data: [%d, 0x%08X]: %d", cache->name, victim, cache->cacheLine[victim].tag, min);
            }
            for (i = 1; i < setSize; ++i) {
                if (debug) {
                    fprintf(debugFile, ", [%d, 0x%08X]: %d", firstIndex+i, cache->cacheLine[firstIndex+i].tag, cache->cacheLine[firstIndex+i].replacementData);
                }
                if (cache->cacheLine[firstIndex + i].replacementData < min) {
                    victim = firstIndex + i;
                    min = cache->cacheLine[firstIndex + i].replacementData;
                }
            }
            if (debug) {
                fprintf(debugFile, "\n");
            }
            break;
        }
        case RANDOM:
            victim = firstIndex + (random() % setSize);
            break;
    }
    if (debug) {
        fprintf(debugFile, "%s: found victim in entry %d\n", cache->name,  victim);
    }
    return victim;
}

int findVictimInVictimCache(VictimCache* victimCache) {
    int i;
    int victim;
    int min;
    for (i = 0; i < victimCache->entries; ++i) {
        if (!(victimCache->cacheLine[i].valid)) {
            return i;
        }
    }
    // No empty cache
    victim = 0;
    min = victimCache->cacheLine[0].replacementData;
    for (i = 1; i < victimCache->entries; ++i) {
        if (victimCache->cacheLine[i].replacementData < min) {
            victim = i;
            min = victimCache->cacheLine[i].replacementData;
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
            checkForDecay(time, cache);
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
            checkForDecay(time, cache);
        case RANDOM:
            break;
    }
}

void swapCacheLines(CacheLine *a, CacheLine *b) {
    int t = a->tag;
    a->tag = b->tag;
    b->tag = t;
    t = a->dirty;
    a->dirty = b->dirty;
    b->dirty = t;
}

// TODO
// void evictFromCache(CacheDescription* cacheDescription, CacheLine* victimLine) {
//     int victim;
//     if (victimLine->dirty) {
//         if (debug) {
//             fprintf(debugFile, "%s: Write dirty victim 0x%08X\n", cacheDescription->cache->name,  victimLine->tag);
//         }
//         ++cacheDescription->cache->totalMissWrites;
//     }
//     victim = findVictimInVictimCache(&cacheDescription->cache->victimCache);
//     if (cacheDescription->cache->victimCache.cacheLine[victim].dirty) {
//         ++cacheDescription->cache->victimCache.totalMissWrites;
//     }
// }

void simulateReferenceToCacheLine(CacheDescription* cacheDescription, MemoryReference* reference) {
    char found = 0;
    int cacheEntryIndex;
    CacheLine* cacheEntry = 0;
    if (debug) {
        fprintf(debugFile, "%s: %s Reference 0x%08X of length %d\n", cacheDescription->name, memoryAccessTypeName(reference->type), reference->address, reference->length);
    }
    ++cacheDescription->cache->totalCacheAccess;
    cacheEntryIndex = searchCacheFor(cacheDescription->cache, reference->address); // Find in cache
    if (0 <= cacheEntryIndex) { // Found
        ++cacheDescription->cache->totalCacheHits;
        found = 1;
        if (debug) {
            fprintf(debugFile, "%s: Found address 0x%08X in cache line %d\n", cacheDescription->name, reference->address, cacheEntryIndex);
        }
        cacheEntry = &cacheDescription->cache->cacheLine[cacheEntryIndex];
        setReplacementData(cacheDescription->numberOfMemoryReference, cacheDescription->cache, cacheEntry);
    } else { // Not found
        found = 0;
        ++cacheDescription->cache->totalCacheMisses;
        ++cacheDescription->cache->victimCache.totalCacheAccess;
        cacheEntryIndex = searchVictimCacheFor(cacheDescription->cache, reference->address); // Go into victim cache
        if (0 <= cacheEntryIndex) { // Found in victim cache
            int victim;
            ++cacheDescription->cache->victimCache.totalCacheHits;
            victim = findVictimInCache(cacheDescription->cache, reference->address);
            swapCacheLines(&cacheDescription->cache->cacheLine[victim], &cacheDescription->cache->victimCache.cacheLine[cacheEntryIndex]);
            cacheDescription->cache->victimCache.cacheLine[cacheEntryIndex].replacementData = cacheDescription->numberOfMemoryReference;
            // TODO cacheEntry = &cacheDescription->cache->victimCache.cacheLine[cacheEntryIndex];
        } else { // Not found
            int victim;
            ++cacheDescription->cache->totalMissReads;
            ++cacheDescription->cache->victimCache.totalCacheMisses;
            ++cacheDescription->cache->victimCache.totalMissReads;
            cacheEntryIndex = findVictimInCache(cacheDescription->cache, reference->address);
            cacheEntry = &cacheDescription->cache->cacheLine[cacheEntryIndex];
            if (debug) {
                fprintf(debugFile, "%s: Pick victim %d to replace\n", cacheDescription->name,  cacheEntryIndex);
            }
            victim = findVictimInVictimCache(&cacheDescription->cache->victimCache);
            if (cacheDescription->cache->victimCache.cacheLine[victim].valid && cacheDescription->cache->victimCache.cacheLine[victim].dirty) {
                ++cacheDescription->cache->victimCache.totalMissWrites;
            }
        }
    }
//         if (!found) {
//             /* fill in evicted cache line for this new line */
//             cacheEntry->valid = 1;
//             cacheEntry->tag = baseCacheAddress;
//             cacheEntry->dirty = 0;
//             /* read cache line from memory into cache table */
//             if (debug) fprintf(debugFile, "%s: Read cache line 0x%08X into entry %d\n", cacheDescription->name,  cacheEntry->tag, cacheEntryIndex);
//             cacheDescription->cache->totalMissReads += 1;
//         }
//     }
//     /* update reference specific info */
//     if (reference->type == STORE) {
//         /* If it's not write-back, then it is write-thru.
//            For write-thru, if it's a write, we write to memory. */
//         if (!cacheDescription->cache->writeBack) {
//             cacheDescription->cache->totalMissWrites += 1;
//             if (debug) fprintf(debugFile, "%s: Write cache line 0x%08X thru to memory\n", cacheDescription->name,  cacheEntry->tag);
//         } else {
//             /* For write-back, if it's a write, it's dirty. */
//             cacheEntry->dirty = 1;
//         }
//     }
//     if (!found) {
//     } else {
//         updateReplacementData(cacheDescription->numberOfMemoryReference, cacheDescription->c, cacheEntry);
//     }
}

void simulateReferenceToMemory(CacheDescription* cacheDescription, MemoryReference* reference) {
    cacheDescription->numberOfMemoryReference += 1;
    cacheDescription->numberOfType[reference->type] += 1;
    // Check if the entire reference fits into just one cache line
    if (getBaseCacheAddress(cacheDescription->cache, reference->address) == getBaseCacheAddress(cacheDescription->cache, reference->address + reference->length -1)) {
        simulateReferenceToCacheLine(cacheDescription, reference);
    } else {
        /* reference spans two cache lines.  Convert it to two
references: the first cache line, and the second cache line */
        MemoryReference reference1;
        MemoryReference reference2;
        /* easiest to compute the second part first */
        reference2.type = reference->type;
        reference2.address = getBaseCacheAddress(cacheDescription->cache, reference->address + reference->length -1);
        reference2.length = reference->address + reference->length - reference2.address;
        reference1.type = reference->type;
        reference1.address = reference->address;
        reference1.length = reference->length - reference2.length;

        /* but we do the references first, then second */
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
    initCachesForTrace();
    while (readTraceFile(traceFile, &reference) != EOF) {
        CacheDescription* cacheDescription = cacheDescriptionRoot;
        while (cacheDescription) {
            simulateReferenceToMemory(cacheDescription, &reference);
            cacheDescription = cacheDescription->next;
        }
    }
    fclose(traceFile);
}

