#ifndef GUARD_FILE_IO
#define GUARD_FILE_IO

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


struct ELLPACK
{
    uint64_t noRows;
    uint64_t noCols;
    uint64_t maxNoNonZero;
    float* values; // asterisk is stored as 0.0
    uint64_t* colPositions;
};

/// @brief reads and validates a matrix
/// @param file pointer to the file
/// @result matrix in ELLPACK format
struct ELLPACK read_validate(const void* file);

/// @brief writes the matrix to the file
/// @param matrix matrix to convert
/// @param result pointer to file
void write(struct ELLPACK matrix, void* result);

/// @brief tests the function `matr_mult_ellpack`
/// @param nr test in `./sample-inputs/` to execute
//void test(int nr);

#endif
