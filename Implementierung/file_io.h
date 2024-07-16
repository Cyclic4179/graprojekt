#ifndef GUARD_FILE_IO
#define GUARD_FILE_IO

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ellpack.h"

void validate_matrix(const struct ELLPACK matrix);

/// @brief reads and validates a matrix
/// @param file pointer to the file
/// @result matrix in ELLPACK format
struct ELLPACK elpk_read_validate(FILE* file);

/// @brief writes the matrix to the file
/// @param matrix matrix to convert
/// @param result pointer to file
void elpk_write(struct ELLPACK matrix, FILE* file);

#endif
