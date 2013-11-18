#include "global.h"
#include "caches.h"


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

void usage(void) {
    fprintf (stderr,"usage: caches [-D] cache-descriptions-file  memory-trace-file\n");
}

void scanargs(char* s)
{
    /* check each character of the option list for
       its meaning. */

    while (*++s != '\0')
        switch (*s)
            {

            case 'D': /* debug option */
                debug = TRUE;
                if (debug)
                    {
                        debugFile = fopen("DEBUG_LOG", "w");
                        if (debugFile == NULL)
                            {
                                fprintf(stderr, "Cannot open DEBUG_LOG\n");
                                debug = FALSE;
                            }
                    }
                break;

            default:
                fprintf (stderr,"caches: Bad option %c\n", *s);
                usage();
                exit(1);
            }
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

int main(int argc, char* *argv)
{
    /* main driver program.  There are two inpu
       files.
       1. defines the caches
       2. defines the memory trace
    */

    /* Process all arguments. */
    /* skip program name */
    argc--, argv++;
    while ((argc > 1) && (**argv == '-'))
        {
            scanargs(*argv);
            argc--, argv++;
        }

    if (argc != 2)
        {
            usage();
            exit(-1);
        }

    Read_Cache_Descriptions(argv[0]);
    initCaches();
    simulateCaches(argv[1]);
    Print_Cache_Statistics();
    deleteCaches();

    exit(0);
}

