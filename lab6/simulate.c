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
    /* we have the first character; it defined the
       memory access type.  Skip any blanks, get the
       hexadecimal address, skip the comma and get the length */

    /* skip any leading blanks */
    c = skipBlanks(traceFile);
    int a = 0;
    while (ishex(c)) {
        a = (a << 4) | hexValue(c);
        c = getc(traceFile);
    }
    if (c != ',') {
        fprintf(stderr, "bad trace file input at line %d: %c\n", traceLineNumber, c);
        exit(-1);
    }
    /* skip the comma */
    /* and get the length */
    int n = 0;
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
    reference->address = a;
    reference->length = n;
}



/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */


int readTraceFileLine(FILE* traceFile, MemoryReference *reference)
{
    int c;

    traceLineNumber = 0;

    while ((c = getc(traceFile)) != EOF)
    {
        /* start the next line */
        traceLineNumber += 1;

        /* skip any leading blanks */
        while (isspace(c) && (c != EOF)) c = getc(traceFile);

        /* what is the character ? */
        switch (c)
        {
            case 'I': /* instruction trace */
                {
                    reference->type = FETCH;
                    readReference(traceFile, reference);
                    return 'I';
                }

            case 'M': /* read/modify/write -- treat as a store */
            case 'S': /* store */
                {
                    reference->type = STORE;
                    readReference(traceFile, reference);
                    return 'S';
                }

            case 'L': /* load */
                {
                    reference->type = LOAD;
                    readReference(traceFile, reference);
                    return 'L';
                }
        }

        /* apparently not a reference line.  There are a bunch
           of other lines that valgrind puts out.  They star
           with  ====, or --, or such.  Skip the entire line. */
        /* skip to end of line */
        while ((c != '\n') && (c != EOF)) c = getc(traceFile);
    }
    return EOF;
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* LFU counts the number of times something is used.  To preven
   a large number from just sitting there, we cause it to decay
   over time.  Every "tick" time units, we shift left one bit,
   so that eventually a count will go to zero if it is not continuing
   to be used. */

void Check_For_Decay(int time, Cache *c)
{
    if (cache->replacementPolicy != LFU) return;

    if (!(time % cache->lfuDecayInterval))
    {
        int i;
        if (debug) fprintf(debugFile, "%s: LFU decay for all LFU counters\n", cache->name);
        for (i = 0; i < cache->entries; i++)
        {
            cache->cacheLine[i].replacementData = cache->cacheLine[i].replacementData/2;
        }
    }
}

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

int getBaseCacheAddress(Cache *c, int a)
{
    /* find number of low-order bits to mask off to find beginning cache
       line address */
    int number_of_low_order_bits = which_power(cache->cacheLineSize);
    int low_order_mask = mask_of(number_of_low_order_bits);
    int cache_address = a & (~low_order_mask);
    return cache_address;
}

int computeSetIndex(Cache *c, int cache_address)
{
    /* shift off low-order offset bits and find bits for
       indexing into cache table */
    /* the number of sets is the number of cache entries
       divided by the number of ways. */
    int number_of_low_order_bits = which_power(cache->cacheLineSize);
    int number_of_sets = cache->entries/cache->numberOfWays;
    int sets_bits = which_power(number_of_sets);
    int sets_bits_mask = mask_of(sets_bits);
    int cache_set_index = (cache_address >> number_of_low_order_bits) & sets_bits_mask;
    int cache_entry_index = cache_set_index * cache->numberOfWays;
    return cache_entry_index;
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* search in the cache for the particular cache address we want */
int searchCacheFor(Cache* c, int cache_address) {
    int cache_entry_index = computeSetIndex(c, cache_address);

    if (debug) fprintf(debugFile, "%s: search cache lines %d to %d for 0x%08X\n",
            cache->name, cache_entry_index,
            cache_entry_index+cache->numberOfWays-1, cache_address);

    /* index into cache table and search the number of ways to
       try to find cache line. */
    int i;
    for (i = 0; i < cache->numberOfWays; i++) {
        if (cache->cacheLine[cache_entry_index+i].valid && (cache_address == cache->cacheLine[cache_entry_index+i].tag)) {
            cache->totalCacheHits += 1;
            return cache_entry_index+i;
        }
    }
    cache->totalCacheMisses += 1;
    return -1;
}

int searchVictimCacheFor(VictimCache* victimCache, int cacheAddress) {
    int i;
    for (i = 0; i < victimCache->entries; ++i) {
        if (victimCache->cacheLine[i].valid && cacheAddress == victimCache->cacheLine[i].address) {
            ++victimCache->totalCacheHits;
            return i;
        }
    }
    ++victimCache->totalCacheMisses;
    return -1;
}

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

int findVictim(Cache *c, int cache_address)
{
    int i;
    int victim;

    int first_index = computeSetIndex(c, cache_address);
    int set_size = cache->numberOfWays;
    if (debug) fprintf(debugFile, "%s: look for victim in %d lines starting at %d\n", cache->name,  set_size, first_index);

    /* first look to see if any entry is empty */
    for (i = 0; i < set_size; i++)
    {
        if (!(cache->cacheLine[first_index+i].valid))
        {
            victim = first_index+i;
            if (debug) fprintf(debugFile, "%s: found empty cache entry at %d\n", cache->name,  victim);
            return victim;
        }
    }

    /* No empty cache line entry */
    victim = first_index; /* default victim */
    switch (cache->replacementPolicy)
    {
        case FIFO:  /* replacement data is the order we were brought in: 1, 2, 3, ... */
            /* choose the smallest */

        case LRU:  /* replacement data is the time we were last hit */
            /* choose the smallest */

        case LFU:  /* replacement data is the number of uses, so
                      choose the smallest */
            {
                int min = cache->cacheLine[first_index].replacementData;
                if (debug) fprintf(debugFile, "%s: replacement data: [%d, 0x%08X]: %d", cache->name, victim, cache->cacheLine[victim].tag, min);
                for (i = 1; i < set_size; i++)
                {
                    if (debug) fprintf(debugFile, ", [%d, 0x%08X]: %d", first_index+i, cache->cacheLine[first_index+i].tag, cache->cacheLine[first_index+i].replacementData);
                    if (cache->cacheLine[first_index+i].replacementData < min)
                    {
                        victim = first_index+i;
                        min = cache->cacheLine[victim].replacementData;
                    }
                }
                if (debug) fprintf(debugFile, "\n");
            }
            break;

        case RANDOM:
            victim = first_index + (random() % set_size);
            break;
    }

    if (debug) fprintf(debugFile, "%s: found victim in entry %d\n", cache->name,  victim);
    return victim;
}


void evictDirtyLine(Cache *c, CacheLine *victim_line)
{
    if (debug) fprintf(debugFile, "%s: Write dirty victim 0x%08X\n",
            cache->name,  victim_line->tag);
    cache->totalMissWrites += 1;
}


void setReplacementData(int time, Cache *c, CacheLine *cache_entry)
{
    switch (cache->replacementPolicy)
    {
        case FIFO:  /* replacement data is the order we were brought in: 1, 2, 3, ... */
            cache_entry->replacementData = time;
            break;

        case LRU:  /* replacement data is the time we were last hit */
            cache_entry->replacementData = time;
            break;

        case LFU:  /* replacement data is a count; starts at zero */
            cache_entry->replacement_data = 0;
            Check_For_Decay(time, c);

        case RANDOM:
            break;
    }
}

void updateReplacementData(int time, Cache *c, CacheLine *cache_entry)
{
    switch (cache->replacementPolicy)
    {
        case FIFO:  /* replacement data is the order we were brought in: 1, 2, 3, ... */
            break;

        case LRU:  /* replacement data is the time we were last hit */
            cache_entry->replacementData = time;
            break;

        case LFU:  /* replacement data is the count of the number of uses */
            cache_entry->replacementData += 1;
            Check_For_Decay(time, c);

        case RANDOM:
            break;
    }
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

#define swap1(a,b) { int t = a; a = b; b = t; }

void swapCacheLines(CacheLine *a, CacheLine *b)
{
    swap1(a->tag, b->tag);
    swap1(a->dirty, b->dirty);
}


void evictFromCache(CacheDescription* cacheDescription, CacheLine* victim_line, int cache_address) {
    /* if victim is dirty, note that this dirty line is being evicted */
    if (victim_line->dirty) {
        evictDirtyLine(cacheDescription->c, victim_line);
    }
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

void simulateReferenceToCacheLine(CacheDescription* cacheDescription, MemoryReference* reference) {
    char found = 0;
    int cacheEntryIndex;
    CacheLine* cacheEntry = 0;
    int cacheAddress;
    if (debug) {
        fprintf(debugFile, "%s: %s Reference 0x%08X of length %d\n", cacheDescription->name, memoryAccessTypeName(reference->type), reference->address, reference->length);
    }
    cacheAddress = getBaseCacheAddress(cacheDescription->c, reference->address);
    ++cacheDescription->cache->totalCacheAccess;
    if (0 <= (cacheEntryIndex = searchCacheFor(cacheDescription->c, cacheAddress))) { // Found in cache
        found = 1;
        if (debug) {
            fprintf(debugFile, "%s: Found address 0x%08X in cache line %d\n", cacheDescription->name, reference->address, cacheEntryIndex);
        }
        cacheEntry = &(cacheDescription->cache->cacheLine[cacheEntryIndex]);
        setReplacementData(cacheDescription->numberOfMemoryReference, cacheDescription->c, cacheEntry);
    } else {
        // Go into victim cache
        ++cacheDescription->cache->victimCache.totalCacheAccess;
        if (0 <= (cacheEntryIndex = searchVictimCacheFor(&cacheDescription->cache->victimCache, cacheAddress))) { // Found in victim cache
            // cacheEntry = &cacheDescription->cache->victimCache.cacheLine[cacheEntryIndex];
        } else {
        }
        /* Did not find it. */
        found = 0;
        /* Choose a victim from the set */
        cacheEntryIndex = findVictim(cacheDescription->c, cacheAddress);
        cacheEntry = &(cacheDescription->cache->cacheLine[cacheEntryIndex]);
        if (debug) {
            fprintf(debugFile, "%s: Pick victim %d to replace\n", cacheDescription->name,  cacheEntryIndex);
        }

        /* evict victim */
        if (cacheEntry->valid) {
            evictFromCache(cacheDescription, cacheEntry, cacheAddress);
        }
        if (!found) {
            /* fill in evicted cache line for this new line */
            cacheEntry->valid = 1;
            cacheEntry->tag = cacheAddress;
            cacheEntry->dirty = 0;
            /* read cache line from memory into cache table */
            if (debug) fprintf(debugFile, "%s: Read cache line 0x%08X into entry %d\n", cacheDescription->name,  cacheEntry->tag, cacheEntryIndex);
            cacheDescription->cache->totalMissReads += 1;
        }
    }
    /* update reference specific info */
    if (reference->type == STORE) {
        /* If it's not write-back, then it is write-thru.
           For write-thru, if it's a write, we write to memory. */
        if (!cacheDescription->cache->writeBack) {
            cacheDescription->cache->totalMissWrites += 1;
            if (debug) fprintf(debugFile, "%s: Write cache line 0x%08X thru to memory\n", cacheDescription->name,  cacheEntry->tag);
        }
        else {
            /* For write-back, if it's a write, it's dirty. */
            cacheEntry->dirty = 1;
        }
    }
    if (!found) {
    } else {
        updateReplacementData(cacheDescription->numberOfMemoryReference, cacheDescription->c, cacheEntry);
    }
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

void simulateReferenceToMemory(CacheDescription* cacheDescription, MemoryReference* reference) {
    cacheDescription->numberOfMemoryReference += 1;
    cacheDescription->numberOfType[reference->type] += 1;
    // Check if the entire reference fits into just one cache line
    if (getBaseCacheAddress(cacheDescription->c, reference->address) == getBaseCacheAddress(cacheDescription->c, reference->address + reference->length -1))
    {
        simulateReferenceToCacheLine(cacheDescription, reference);
    }
    else
    {
        /* reference spans two cache lines.  Convert it to two
references: the first cache line, and the second cache line */
        MemoryReference reference1;
        MemoryReference reference2;
        /* easiest to compute the second part first */
        reference2.type = reference->type;
        reference2.address = getBaseCacheAddress(cacheDescription->c, reference->address + reference->length -1);
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
    while (readTraceFileLine(traceFile, &reference) != EOF) {
        CacheDescription* cacheDescription = cacheDescriptionRoot;
        while (cacheDescription) {
            simulateReferenceToMemory(cacheDescription, &reference);
            cacheDescription = cacheDescription->next;
        }
    }
    fclose(traceFile);
}

