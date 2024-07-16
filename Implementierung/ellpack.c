#include "ellpack.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void elpk_check_equal(struct ELLPACK a, struct ELLPACK b, float max_diff) {
    // buffer for storing floats in nice format
    char s1[256];
    char s2[256];
    char s_max_diff[256];

    if (a.noCols != b.noCols || a.noRows != b.noRows || a.maxNoNonZero != b.maxNoNonZero) {
        printf("dimensions not equal: a(%lu x %lu) vs b(%lu x %lu)\n", a.noRows, a.noCols, b.noRows, b.noCols);
        exit(EXIT_FAILURE);
    }

    for (uint64_t i = 0; i < a.noRows * a.maxNoNonZero; i++) {
        float absdiff = fabsf(a.values[i] - b.values[i]);

        if (absdiff > max_diff || a.indices[i] != b.indices[i]) {
            ftostr(sizeof(s1), s1, a.values[i]);
            ftostr(sizeof(s2), s2, b.values[i]);
            ftostr(sizeof(s_max_diff), s_max_diff, max_diff);

            printf(
                "values or index at entry '%lu' greater than "
                "tolerated error (%s): a(val: %s, rowind: %lu) vs b(val: %s, rowind: %lu) \n",
                i, s_max_diff, s1, a.indices[i], s2, b.indices[i]);

            exit(EXIT_FAILURE);
        }
    }

    puts("equal");
    exit(EXIT_SUCCESS);
}
