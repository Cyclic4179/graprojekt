#ifndef GUARD_ELLPACK
#define GUARD_ELLPACK

#include <stdint.h>
#include <stdlib.h>
#include <xmmintrin.h>

struct ELLPACK {
    uint64_t noRows;
    uint64_t noCols;
    uint64_t maxNoNonZero;
    float* values; // asterisk is stored as 0.0
    uint64_t* indices;
};

struct DENSE_MATRIX_XMM {
    uint64_t noRows;
    uint64_t noCols;
    uint64_t noQuadCols;
    __m128* values;
};

struct DENSE_MATRIX {
    uint64_t noRows;
    uint64_t noCols;
    float* values;
};


void elpk_check_equal(struct ELLPACK a, struct ELLPACK b, float max_diff);

__attribute__((always_inline)) inline void elpk_free(struct ELLPACK e) {
    free(e.values);
    free(e.indices);
}

#endif
