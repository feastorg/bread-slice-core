// CHECK() prints the failing expression and counts it; main() returns the
// count's status.
#ifndef CHECK_H
#define CHECK_H

#include <stdio.h>

static int check_failures = 0;

#define CHECK(cond)                                                                 \
    do                                                                              \
    {                                                                               \
        if (!(cond))                                                                \
        {                                                                           \
            fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond); \
            check_failures++;                                                       \
        }                                                                           \
    } while (0)

#define CHECK_DONE() return check_failures ? 1 : 0

#endif
