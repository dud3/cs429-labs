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
    int cacheLineSize;
    int numberOfWays;
    int entries;
    int lfuDecayInterval;
    int totalCacheAccess;
    int totalCacheHits;
    int totalCacheMisses;
    int totalMissReads;
    int totalMissWrites;
    char* name;
    CacheLine* cacheLine;
    char writeBack;
    enum CacheReplacementPolicy replacementPolicy;
} Cache;

typedef struct CacheDescription {
    int numberOfMemoryReference;
    int numberOfType[NUMBER_OF_MEMORY_ACCESS_TYPE];
    char* name;
    Cache* mainCache;
    Cache* victimCache;
    struct CacheDescription* next;
} CacheDescription;

extern CacheDescription* cacheDescriptionRoot;
void printCacheStatistics();
void initCacheDescriptions();
void initCacheDescriptionsForTrace();
void deleteCacheDescriptions();

