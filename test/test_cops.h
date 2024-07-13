#ifndef TEST_COPS_H
#define TEST_COPS_H

#include <stdio.h>
#include <stdlib.h>
#include "cops.h"

#define MATCHES(x, v) strcmp(x, v) == 0

#define TP_ASSERT(expr)                                                                       \
        printf("\tASSERT %s\n", #expr);                                                       \
        if (!(expr)) {                                                                        \
                fprintf(stderr, "%s:%d Assertion '%s' failed.\n", __FILE__, __LINE__, #expr); \
                abort();                                                                      \
        }

int test_runner(void);

#endif /* ifndef TEST_COPS_H */
