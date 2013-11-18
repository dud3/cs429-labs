#include "global.h"
#include "caches.h"

void usage(const char* programName) {
    fprintf(stderr,"Usage: %s [-D] cache-descriptions-file  memory-trace-file\n", programName);
}

void scanargs(const char* s) {
    while (*++s) {
        switch (*s) {
            case 'D': // Debug
                debug = 1;
                if (debug) {
                    debugFile = fopen("DEBUG_LOG", "w");
                    if (!debugFile) {
                        fprintf(stderr, "Cannot open DEBUG_LOG\n");
                        debug = 0;
                    }
                }
                break;
            default:
                fprintf(stderr,"Caches: Bad option %c\n", *s);
                usage();
                exit(1);
        }
    }
}

int main(int argc, char** argv) {
    int i;
    if (argc != 2) {
        usage(argv[0]);
        exit(-1);
    }
    for (i = 1; i < argc; ++i) {
        while (**argv == '-') {
            scanargs(*argv);
        }
    }
    readCacheDescriptions(argv[0]);
    initCaches();
    simulateCaches(argv[1]);
    printCacheStatistics();
    deleteCaches();
    return 0;
}

