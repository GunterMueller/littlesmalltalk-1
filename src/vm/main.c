/*
    Little Smalltalk main program, unix version
    Written by Tim Budd, budd@cs.orst.edu
    All rights reserved, no guarantees given whatsoever.
    May be freely redistributed if not for profit.

    starting point, primitive handler for unix
    version of the little smalltalk system
*/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "err.h"
#include "globs.h"
#include "interp.h"
#include "memory.h"
#include "prim.h"

/*
    the following defaults must be set

*/
# define DefaultImageFile "lst.img"
# define DefaultStaticSize 100000
# define DefaultDynamicSize 200000
# define DefaultTmpdir "/tmp"

/*
--------------------
*/


/* # define COUNTTEMPS */

unsigned int debugging = 0;

extern int errno;


# ifdef COUNTTEMPS
FILE *tempFile;
# endif

int main(int argc, char **argv)
{
    struct object *aProcess, *aContext, *o;
    int size, i, staticSize, dynamicSize;
    FILE *fp;
    char imageFileName[120], *p;

    strcpy(imageFileName, DefaultImageFile);
    staticSize = DefaultStaticSize;
    dynamicSize = DefaultDynamicSize;
    tmpdir = DefaultTmpdir;

    /*
     * See if our environment tells us what TMPDIR to use
     */
    p = getenv("TMPDIR");
    if (p) {
        tmpdir = strdup(p);
    }

    /* first parse arguments */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            /* FIXME - this should be generated by CMake. */
            printf("Little Smalltalk, version 4.6.2\n");
        } else if (strcmp(argv[i], "-s") == 0) {
            staticSize = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-d") == 0) {
            dynamicSize = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-g") == 0) {
            debugging = 1;
        } else {
            strcpy(imageFileName, argv[i]);
        }
    }

# ifdef COUNTTEMPS
    tempFile = fopen("/usr/tmp/counts", "w");
# endif

    gcinit(staticSize, dynamicSize);

    /* read in the method from the image file */
    fp = fopen(imageFileName, "r");
    if (! fp) {
        fprintf(stderr,"cannot open image file: %s\n", imageFileName);
        exit(1);
    }

    printf("%d objects in image\n", fileIn(fp));
    fclose(fp);

    /* build a context around it */

    aProcess = staticAllocate(3);
    /* context should be dynamic */
    aContext = gcalloc(contextSize);
    aContext->class = ContextClass;


    aProcess->data[contextInProcess] = aContext;
    size = integerValue(initialMethod->data[stackSizeInMethod]);
    aContext->data[stackInContext] = staticAllocate(size);
    aContext->data[argumentsInContext] = nilObject;

    aContext->data[temporariesInContext] = staticAllocate(19);
    aContext->data[bytePointerInContext] = newInteger(0);
    aContext->data[stackTopInContext] = newInteger(0);
    aContext->data[previousContextInContext] = nilObject;
    aContext->data[methodInContext] = initialMethod;

    /* now go do it */
    rootStack[rootTop++] = aProcess;

#if defined(PROFILE)
    take_samples(1);
#endif
    switch(execute(aProcess, 0)) {
    case 2:
        printf("User defined return\n");
        break;

    case 3:
        printf("can't find method in call\n");
        aProcess = rootStack[--rootTop];
        o = aProcess->data[resultInProcess];
        printf("Unknown method: %s\n", bytePtr(o));
        aContext = aProcess->data[contextInProcess];
        backTrace(aContext);
        break;

    case 4:
        printf("\nnormal return\n");
        break;

    case 5:
        printf("time out\n");
        break;

    default:
        printf("unknown return code\n");
        break;
    }
#if defined(VSTA) && defined(PROFILE)
    dump_samples();
#endif

    printf("\nCache statistics:\n");
    printf("  %ld hit, %ld miss for %02.2f%% hit rate.\n", cache_hit, cache_miss, (float)((float)(cache_hit)*100.0/(float)(cache_hit + cache_miss)));
    printf("\nGC statistics:\n");
    printf("  %ld garbage collections\n", gc_count);
    printf("  %ld total microseconds in GC for %ld microseconds per GC pass.\n", gc_total_time, gc_total_time/gc_count);
    printf("  %ld microseconds for longest GC pause.\n", gc_max_time);
    printf("  %ld total bytes copied for %ld bytes per GC on average.\n", gc_total_mem_copied, gc_total_mem_copied/gc_count);
    printf("  %ld maximum bytes copied during GC.\n", gc_mem_max_copied);
    return(0);
}

/*
These static character strings are used for URL conversion
*/

//static char *badURLChars = ";/?:@&=+!*'(),$-_.<>#%\"\t\n\r";
//static char *hexDigits = "0123456789ABCDEF";
