#ifndef GUARD_FILE_IO
#define GUARD_FILE_IO

#include <stdint.h>
#include <stdio.h>

#include "ellpack.h"

void validate_matrix(const struct ELLPACK matrix);

/// @brief helper: read int from string
/// @param string string
/// @param pos current position in string
/// @param end character after int
/// @param field_for_error part of error message
/// @return read int
uint64_t helper_read_int(const char* string, long* pos, char end, char* field_for_error, int line);

/// @brief helper: read float from string
/// @param string string
/// @param pos current position in string
/// @param end character after float
/// @param field_for_error part of error message
/// @return read float
float helper_read_float(const char* string, long* pos, char end, char* field_for_error);

/// @brief check for no indices larger than matrix dimensions and only ascending indices
/// @param matrix matrix to check
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
