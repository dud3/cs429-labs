#pragma once

enum CacheReplacementPolicy {
    FIFO,
    LRU,
    LFU,
    RANDOM
};

typedef struct {
    char valid;
    char dirty;
    char* actualData;
    int tag;
    int replacementData;
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
    char* name;
    char writeBack;
    int cacheLineSize;
    int numberOfWays;
    int entries;
    int totalCacheAccess;
    int totalCacheHits;
    int totalCacheMisses;
    int totalMissReads;
    int totalMissWrites;
    int lfuDecayInterval;
    enum CacheReplacementPolicy replacementPolicy;
    CacheLine* cacheLine;
    VictimCache victimCache;
} Cache;

typedef struct {
    char* name;
    int numberOfMemoryReference;
    int numberOfType[NUMBER_OF_MEMORY_ACCESS_TYPE];
    struct CacheDescription* next;
    Cache* cache;
} CacheDescription;

extern CacheDescription* cacheDescriptionRoot;
const char* memoryAccessTypeName(enum MemoryAccessType);

