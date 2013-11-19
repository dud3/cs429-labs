#pragma once
#include "utils.h"

enum CacheReplacementPolicy {
    FIFO,
    LRU,
    LFU,
    RANDOM
};

typedef struct {
    int tag;
    int replacementData;
    char valid;
    char dirty;
} CacheLine;

typedef struct {
    int entries;
    int totalCacheAccess;
    int totalCacheHits;
    int totalCacheMisses;
    int totalMissReads;
    int totalMissWrites;
    CacheLine* cacheLine;
} VictimCache;

typedef struct {
    int cacheLineSize;
    int numberOfWays;
    int entries;
    int totalCacheAccess;
    int totalCacheHits;
    int totalCacheMisses;
    int totalMissReads;
    int totalMissWrites;
    int lfuDecayInterval;
    char* name;
    CacheLine* cacheLine;
    char writeBack;
    enum CacheReplacementPolicy replacementPolicy;
    VictimCache victimCache;
} Cache;

typedef struct CacheDescription {
    int numberOfMemoryReference;
    int numberOfType[NUMBER_OF_MEMORY_ACCESS_TYPE];
    char* name;
    Cache* cache;
    struct CacheDescription* next;
} CacheDescription;

extern CacheDescription* cacheDescriptionRoot;
const char* cacheReplacementPolicyName(Cache*, char*);
const char* printSetsAndWays(Cache*);
void printCacheStatistics();
void initCacheDescriptions();
void initCacheDescriptionsForTrace();

