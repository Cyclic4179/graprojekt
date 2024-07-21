#include "ellpack.h"
#include "util.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


/// @brief checks wether two ELLPACK matrices are equal (enough)
/// @param a matrix a
/// @param b matrix b
/// @param max_diff maximum absolute error
void elpk_check_equal(struct ELLPACK a, struct ELLPACK b, float max_diff) {
    // buffer for storing floats in nice format
    char s1[256];
    char s2[256];
    char s_max_diff[256];

    if (a.noCols != b.noCols || a.noRows != b.noRows || a.maxNoNonZero != b.maxNoNonZero) {
        printf("dimensions or max number of non-zero not equal: "
               "a(%lu x %lu - nonz: %lu) vs b(%lu x %lu - nonz: %lu)\n",
               a.noRows, a.noCols, a.maxNoNonZero, b.noRows, b.noCols, b.maxNoNonZero);
        exit(EXIT_FAILURE);
    }

    for (uint64_t i = 0; i < a.noRows * a.maxNoNonZero; i++) {
        float absdiff = fabsf(a.values[i] - b.values[i]);
        float abssum = fabsf(a.values[i] + b.values[i]);

        float reldeviation = 1e6 * absdiff / abssum;

        //if (absdiff > max_diff || a.indices[i] != b.indices[i]) {
        if (reldeviation > max_diff || a.indices[i] != b.indices[i]) {
            ftostr(sizeof(s1), s1, a.values[i]);
            ftostr(sizeof(s2), s2, b.values[i]);
            ftostr(sizeof(s_max_diff), s_max_diff, max_diff);

            printf(
                "values or index at entry '%lu' greater than "
                "tolerated error (%s): reldev %f: a(val: %s, rowind: %lu) vs b(val: %s, rowind: %lu) \n",
                i, s_max_diff, reldeviation, s1, a.indices[i], s2, b.indices[i]);

            exit(EXIT_FAILURE);
        }
    }

    puts("equal");
}
