#include "debug.h"
#include "cds.h"
#include "read_cds.h"
#include "simulate.h"
#include <stdlib.h>
#include <string.h>

void usage(const char* programName) {
    fprintf(stderr,"Usage: %s [-D] cache-descriptions-file  memory-trace-file\n", programName);
}

int main(int argc, char** argv) {
    char* descriptionsFileName;
    char* traceFileName;
    if (!(argc == 3) && !(argc == 4 && !strcmp(argv[1], "-D"))) {
        usage(argv[0]);
        exit(-1);
    }
    if (argc == 3) {
        descriptionsFileName = argv[1];
        traceFileName = argv[2];
    } else if (argc == 4) {
        debug = 1;
        debugFile = fopen("DEBUG_LOG", "w");
        if (!debugFile) {
            fprintf(stderr, "Cannot open DEBUG_LOG\n");
            debug = 0;
        }
        descriptionsFileName = argv[2];
        traceFileName = argv[3];
    }
    readCacheDescriptions(descriptionsFileName);
    initCacheDescriptions();
    initCacheDescriptionsForTrace();
    simulateCaches(traceFileName);
    printCacheStatistics();
    deleteCacheDescriptions();
    return 0;
}

