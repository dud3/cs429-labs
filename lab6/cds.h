#ifndef _CDS_H_
#define _CDS_H_

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* The Cache Description Structure */
/* The description of a cache. */

/* define the Cache Replacement Policy */
enum CRP
{
    CRP_FIFO,
    CRP_LRU,
    CRP_LFU,
    CRP_RANDOM
};



/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */
/* Each cache is defined by a number of integers:

   1. The number of entries.
   2. The number of ways.  If we have n entries and m ways, we have n/m sets.
   3. The cache line size.
   4. If the cache is write-back, or write-thru.

   Not that adjusting the number of ways allows us to get the three
   main types (assume the number of entries is n):

   1. Fully-associative -- number of ways = n.
   2. Direct-mapped  -- number of ways = 1.
   3. Set associative -- any other value of n.

   Note that we have to have (number of entries) be evenly divisible by the
   number of ways.  We can't have 15 lines and 4 ways.

   And we need the cache line size as a power of two.
   And the number of ways must be a power of two.
*/

/* With this description, a cache is then a dynamically allocated
   array of "number of entries" cache lines.  Each cache line is
   (a) an array of bytes (the cache line),
   (b) an address tag (let's just keep the full address of the first byte),
   (c) a dirty bi
   (d) maybe some info for the replacement policy.
*/

struct cache_line
{
    char valid;
    char dirty;
    char* actual_data;
    int tag;
    int     replacement_data;
};
typedef struct cache_line cache_line;

// Victim Cache
typedef struct {
    char valid;
    char dirty;
    int address;
    int replacementData;
} VictimCacheLine;

typedef struct {
    int entries;
    int totalCacheAccess;
    int totalCacheHits;
    int totalCacheMisses;
    int totalMissReads;
    int totalMissWrites;
    VictimCacheLine* cacheLine;
} VictimCache;

struct cache
{
    char*      name;

    int         cache_line_size;
    int         number_of_ways;
    char     write_back;
    enum CRP    replacement_policy;

    /* how often to decrease the counts for LFU */
    int         LFU_Decay_Interval;

    /* array of cache lines */
    cache_line* c_line;
    int         number_of_cache_entries;

    int number_total_cache_access;
    int number_cache_hits;
    int number_cache_misses;

    int number_miss_reads;
    int number_miss_writes;
    VictimCache victimCache;
};


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */
/* The actual cache description structure */

struct CDS {
    struct CDS *next;  /* linked list of all the CDS */

    char*      name;

    struct cache* c;

    /* statistics for each cache policy */
    int number_of_memory_reference;
    int number_of_type[NUMBER_OF_MEMORY_ACCESS_TYPE];
};
typedef struct CDS CDS;



/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* a linked list of all the cache descriptors */
struct CDS *CDS_root;

extern char* memory_reference_type_name(enum memory_access_type type);

#endif
