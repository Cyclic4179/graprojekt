#ifndef GUARD_MAIN
#define GUARD_MAIN

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


#define MAX_IMPL_VERSION 1

/// @brief main function
/// @param a matrix a
/// @param b matrix b
/// @param result result of multiplication
void matr_mult_ellpack(const void* a, const void* b, void* result);
void matr_mult_ellpack_V1(const void* a, const void* b, void* result);

/// @brief initialize the result matrix, check for multiplicable dimensions
/// @param left left matrix
/// @param right right matrix
/// @param result result matrix
/// @result initialized result matrix
struct ELLPACK initialize_result(struct ELLPACK left, struct ELLPACK right, struct ELLPACK result);

/// @brief remove unnecessary padding in the result matrix and free the unused memory
/// @param result result matrix
struct ELLPACK remove_unnecessary_padding(struct ELLPACK result);

/// @brief output debug information
/// @param left left matrix
/// @param right right matrix
/// @param result result matrix
void debug_info(struct ELLPACK left, struct ELLPACK right, struct ELLPACK result);

/// @brief output debug information
/// @param c name of matrix: 'l' -> "left"; 'r' -> "right"; 'o' -> "result"; everything else -> "matrix"
/// @param matrix matrix to output
void debug_info_single(char c, struct ELLPACK matrix);

/// @brief check for valid inputs: multiplicable dimensions, no indices larger than matrix dimensions
/// @param left left matrix
/// @param right right matrix
void validate_inputs(struct ELLPACK left, struct ELLPACK right);
void validate_matrix(struct ELLPACK matrix);
#endif
