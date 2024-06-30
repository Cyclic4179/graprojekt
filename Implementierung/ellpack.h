#include <stdint.h>
#include <stdlib.h>

struct ELLPACK {
    uint64_t noRows;
    uint64_t noCols;
    uint64_t maxNoNonZero;
    float* values; // asterisk is stored as 0.0
    uint64_t* colPositions;
};

inline void free_ellpack(struct ELLPACK e) {
    free(e.values);
    free(e.colPositions);
}
