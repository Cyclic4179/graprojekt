#ifndef GUARD_MAIN
#define GUARD_MAIN

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


struct ELLPACK
{
    uint64_t noRows;
    uint64_t noCols;
    uint64_t maxNoNonZero;
    uint64_t* colPositions;
    float* values;
};

/// @brief main function
/// @param a matrix a
/// @param b matrix b
/// @param result result of multiplication
void matr_mult_ellpack(const void* a, const void* b, void* result);

/// @brief reads and validates a matrix
/// @param file pointer to the file
/// @result matrix in ELLPACK format
struct ELLPACK read_validate(const void* file);

/// @brief multiplies the matrices
/// @param left left matrix
/// @param right right matrix
/// @result multiplied matrix
struct ELLPACK multiply(struct ELLPACK left, struct ELLPACK right);

/// @brief writes the matrix to the file
/// @param matrix matrix to convert
/// @param result pointer to file
void write(struct ELLPACK matrix, void* result);

/// @brief tests the function `matr_mult_ellpack`
/// @param nr test in `./sample-inputs/` to execute
void test(int nr);

#endif
