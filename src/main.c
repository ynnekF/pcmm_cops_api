#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cops.h"
#include "test_cops.h"

static uint32_t
pcmm_cops_api_version(void) {
        FILE* bid = fopen(BUILD_UUID_FILE, "r");
        int ret = 0;

        if (bid != NULL)
                if (fscanf(bid, "%d", &ret) <= 0)
                        perror("failed to read build version");

        fclose(bid);
        return ret;
}

__attribute__((constructor(102))) static void
preprocess(void) {
        printf("PCMM COPS API Version v%d, GCC Version %s ", pcmm_cops_api_version(), __VERSION__);
        switch (__STDC_VERSION__) {
                case 199409L: printf("__STDC_VERSION__ (C94)\n"); break;
                case 199901L: printf("__STDC_VERSION__ (C99)\n"); break;
                case 201112L: printf("__STDC_VERSION__ (C11)\n"); break;
                case 201710L: printf("__STDC_VERSION__ (C17)\n"); break;
                default:
                        printf("__STDC_VERSION__ ");
                        __STDC_VERSION__ > 201710L ? printf(" (std=c++2a)") : printf(" Unknown standard");
        }
}

int
main(int argc, char const** argv) {
        return test_runner();
}
