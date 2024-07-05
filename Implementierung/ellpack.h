#include <stdint.h>
#include <stdlib.h>

struct ELLPACK {
    uint64_t noRows;
    uint64_t noCols;
    uint64_t maxNoNonZero;
    float* values; // asterisk is stored as 0.0
    uint64_t* indices;
};

__attribute__((always_inline)) inline void free_elpk(struct ELLPACK e) {
    free(e.values);
    free(e.indices);
}
