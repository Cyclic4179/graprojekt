#ifndef GUARD_FILE_IO
#define GUARD_FILE_IO

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


/// @brief reads and validates a matrix
/// @param file pointer to the file
/// @result matrix in ELLPACK format
struct ELLPACK read_validate(FILE* file);

/// @brief writes the matrix to the file
/// @param matrix matrix to convert
/// @param result pointer to file
void write(struct ELLPACK matrix, FILE* file);

/// @brief tests the function `matr_mult_ellpack`
/// @param nr test in `./sample-inputs/` to execute
//void test(int nr);

#endif
