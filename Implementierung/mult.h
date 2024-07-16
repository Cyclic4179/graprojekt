#ifndef GUARD_MULT
#define GUARD_MULT

#include <pmmintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>

#define MAX_IMPL_VERSION 4

/// @brief main function
/// @param a matrix a
/// @param b matrix b
/// @param result result of multiplication
void matr_mult_ellpack(const void* a, const void* b, void* result);
void matr_mult_ellpack_V1(const void* a, const void* b, void* result);
void matr_mult_ellpack_V2(const void* a, const void* b, void* result);
void matr_mult_ellpack_V3(const void* a, const void* b, void* result);
void matr_mult_ellpack_V4(const void* a, const void* b, void* result);

/// @brief initialize the result matrix, check for multiplicable dimensions
/// @param left left matrix
/// @param right right matrix
/// @param result matrix to initialize
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

/// @brief check for valid inputs: multiplicable dimensions
/// @param left left matrix
/// @param right right matrix
void validate_inputs(struct ELLPACK left, struct ELLPACK right);

/// @brief transposes the given matrix and returns the result
struct ELLPACK transpose(struct ELLPACK matrix);

/// @brief transforms a sparse matrix of ELLPACK format to a dense matrix and returns it
struct DENSE_MATRIX to_dense(struct ELLPACK matrix);

/// @brief transforms the values of the given dense matrix into XMM float values
struct DENSE_MATRIX_XMM to_XMM(struct DENSE_MATRIX);

/// @brief output debug information of a XMM matrix
/// @param matrix the matrix to debug
void debug_XMM(struct DENSE_MATRIX_XMM matrix);

#endif
