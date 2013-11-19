#include "global.h"
#include "cds.h"
#include "read_cds.h"
#include "simulate.h"
#include <stdlib.h>
#include <string.h>

void usage(const char* programName) {
    fprintf(stderr,"Usage: %s [-D] cache-descriptions-file  memory-trace-file\n", programName);
}

int main(int argc, char** argv) {
    if (!(argc == 3) && !(argc == 4 && !strcmp(argv[1], "-D"))) {
        usage(argv[0]);
        exit(-1);
    }
    if (argc == 4) {
        debug = 1;
        debugFile = fopen("DEBUG_LOG", "w");
        if (!debugFile) {
            fprintf(stderr, "Cannot open DEBUG_LOG\n");
            debug = 0;
        }
    }
    readCacheDescriptions(argv[1]);
    // initCaches();
    // simulateCaches(argv[1]);
    // printCacheStatistics();
    deleteCacheDescriptions();
    return 0;
}

