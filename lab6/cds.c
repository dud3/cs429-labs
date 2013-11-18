/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

/* cache description support code */

#include "global.h"
#include "cds.h"
#include "caches.h"


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

char* print_sets_and_ways(struct cache *c)
{
    if (c->number_of_ways == 1) return("direct-mapped");
    if (c->number_of_ways == c->number_of_cache_entries) return("fully associative");

    static char buffer[64];
    sprintf (buffer, "%d sets of %d ways",
             c->number_of_cache_entries/c->number_of_ways, c->number_of_ways);
    return(buffer);
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

char* memory_reference_type_name(enum memory_access_type type)
{
    switch(type)
        {
        case MAT_FETCH: return("Fetch");
        case MAT_LOAD:  return("Load");
        case MAT_STORE: return("Store");
        }
    return("invalid");
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

char* CRP_name(struct cache *c)
{
    switch(c->replacement_policy)
        {
        case CRP_FIFO:   return("FIFO");
        case CRP_LRU:    return("LRU");
        case CRP_RANDOM: return("RANDOM");

        case CRP_LFU:
            {
                static char buffer[64];
                sprintf(buffer, "LFU (decay=%d)", c->LFU_Decay_Interval);
                return(buffer);
            }

        };
    return("Invalid policy");
}



void debug_print_cache(struct cache *c)
{
    fprintf(debugFile, "%s: Total number of entries: %d\n", c->name,  c->number_of_cache_entries);
    fprintf(debugFile, "%s: %s\n", c->name,  print_sets_and_ways(c));
    fprintf(debugFile, "%s: Each cache line is %d bytes\n", c->name,  c->cache_line_size);
    fprintf(debugFile, "%s: Cache is %s\n", c->name,  c->write_back ? "write-back" : "write-thru");
    fprintf(debugFile, "%s: With a %s replacement policy\n", c->name, CRP_name(c));
}


void debug_print_cds(CDS *cds)
{
    debug_print_cache(cds->c);
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

int percent(int a, int b)
{
    if (b == 0) return(0);
    int n = (a*100/b);
    return(n);
}


void Print_Cache_Statistics_for_one_cache(struct cache *c)
{
    fprintf(stdout, "%s: %d entries of lines of %d bytes; %s, %s, %s\n",
            c->name, c->number_of_cache_entries, c->cache_line_size,
            print_sets_and_ways(c),
            c->write_back ? "write-back" : "write-thru",
            CRP_name(c));

    fprintf(stdout, "%s: %d accesses, %d hits (%d%%), %d misses, %d miss reads, %d miss writes\n",
            c->name, c->number_total_cache_access,
            c->number_cache_hits, percent(c->number_cache_hits, c->number_total_cache_access),
            c->number_cache_misses, c->number_miss_reads, c->number_miss_writes);

    if (c->write_back)
        fprintf(stdout, "%s: %d dirty cache lines remain\n", c->name, number_dirty_lines(c));
}


void Print_Cache_Statistics_for_one_cds(CDS *cds)
{
    fprintf(stdout, "      %d addresses (%d %s, %d %s, %d %s)\n",
            cds->number_of_memory_reference,
            cds->number_of_type[MAT_FETCH], memory_reference_type_name(MAT_FETCH),
            cds->number_of_type[MAT_LOAD], memory_reference_type_name(MAT_LOAD),
            cds->number_of_type[MAT_STORE], memory_reference_type_name(MAT_STORE));

    Print_Cache_Statistics_for_one_cache(cds->c);

    fprintf(stdout, "\n");
}


void Print_Cache_Statistics(void)
{
    CDS *cds = CDS_root;
    while (cds != NULL)
        {
            Print_Cache_Statistics_for_one_cds(cds);
            cds = cds->next;
        }
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */


void initCache(CDS* cds) {
    /* we need one cache line for every entry */
    cds->c->c_line = (cache_line*) calloc(cds->c->number_of_cache_entries, sizeof(cache_line));
    cds->c->victimCache.cacheLine = (VictimCacheLine*) calloc(cds->c->victimCache.entries, sizeof(VictimCacheLine));
}


void initCaches() {
    CDS* cds = CDS_root;
    while (cds != NULL) {
        initCache(cds);
        cds = cds->next;
    }
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

void initCacheForTrace(CDS* cds) {
    int i;
    for (i = 0; i < NUMBER_OF_MEMORY_ACCESS_TYPE; ++i) {
        cds->number_of_type[i] = 0;
    }
    cds->number_of_memory_reference = 0;
    cds->c->number_miss_reads = 0;
    cds->c->number_miss_writes = 0;
    cds->c->number_total_cache_access = 0;
    cds->c->number_cache_hits = 0;
    cds->c->number_cache_misses = 0;
    cds->c->victimCache.totalCacheAccess = 0;
    cds->c->victimCache.totalCacheHits = 0;
    cds->c->victimCache.totalCacheMisses = 0;
    cds->c->victimCache.totalMissReads = 0;
    cds->c->victimCache.totalMissWrites = 0;
}

void initCachesForTrace() {
    CDS* cds = CDS_root;
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

void deleteCache(CDS* cds) {
    deleteCacheLine(cds->c);
    free(cds->c);
    free(cds->name);
    free(cds);
}

void deleteCaches() {
    CDS* cds = CDS_root;
    while (cds != 0) {
        CDS* old = cds;
        cds = cds->next;
        deleteCache(old);
    }
}

