#include "cds.h"
#include "read_cds.h"
#include "simulate.h"
#include <stdlib.h>
#include <string.h>

void usage(const char* programName) {
    fprintf(stderr,"Usage: %s cache-descriptions-file  memory-trace-file\n", programName);
}

int main(int argc, char** argv) {
    char* descriptionsFileName;
    char* traceFileName;
    if (!(argc == 3)) {
        usage(argv[0]);
        exit(-1);
    }
    if (argc == 3) {
        descriptionsFileName = argv[1];
        traceFileName = argv[2];
    }
    readCacheDescriptions(descriptionsFileName);
    initCacheDescriptions();
    initCacheDescriptionsForTrace();
    simulateCaches(traceFileName);
    printCacheStatistics();
    deleteCacheDescriptions();
    return 0;
}

