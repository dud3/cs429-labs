#pragma once
#include "global.h"
#include "cds.h"

extern void Read_Cache_Descriptions(char* CDS_file_name);
extern void debug_print_cds(CDS *cds);
extern void Print_Cache_Statistics(void);
extern int number_dirty_lines(struct cache *c);
extern void initCaches();
extern void initCachesForTrace();
extern void deleteCaches();
extern void simulateCaches(char*);

