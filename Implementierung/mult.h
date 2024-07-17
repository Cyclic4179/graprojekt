#ifndef GUARD_MULT
#define GUARD_MULT

#define MAX_IMPL_VERSION 5

#include "ellpack.h"

/// @brief second version, searching corresponding values in right matrix for every entry in left matrix
/// @param a Pointer to left matrix
/// @param b Pointer to right matrix
/// @param res Pointer to result of multiplication
void matr_mult_ellpack_V1(const void* a, const void* b, void* res);

/// @brief first and main version, optimized seach for corresponding value in right matrix compared to second version
void matr_mult_ellpack(const void* a, const void* b, void* res);

/// @brief third version, working on transposed right matrix for better cache compatibility,
void matr_mult_ellpack_V2(const void* a, const void* b, void* res);

/// @brief fourth version, working on a dense matrix, for almost dense matrices more memory efficient and simpler
void matr_mult_ellpack_V3(const void* a, const void* b, void* res);

/// @brief fifth version, optimized for fast almost-dense matrices multiplication by using SIMD with Intrinsics
void matr_mult_ellpack_V4(const void* a, const void* b, void* res);

/// @brief sixth version, reduced seach cost on normal Ellpack matrices
void matr_mult_ellpack_V5(const void* a, const void* b, void* res);

/// @brief check for valid inputs: multiplicable dimensions
/// @param left left matrix
/// @param right right matrix
void validate_inputs(struct ELLPACK left, struct ELLPACK right);

/// @brief initialize the result matrix
/// @param left left matrix
/// @param right right matrix
/// @param result result matrix
/// @result initialized result matrix
struct ELLPACK initialize_result(const struct ELLPACK left, const struct ELLPACK right, struct ELLPACK result);

/// @brief remove unnecessary padding in the result matrix and free the unused memory
/// @param result result matrix
/// @result smaller matrix
struct ELLPACK remove_unnecessary_padding(struct ELLPACK result);

/// @brief transposes the given matrix and returns the result
struct ELLPACK transpose(const struct ELLPACK matrix);

/// @brief transforms a sparse matrix of ELLPACK format to a dense matrix and returns it
struct DENSE_MATRIX to_dense(const struct ELLPACK matrix);

/// @brief transforms the values of the given dense matrix into XMM float values
struct DENSE_MATRIX_XMM to_XMM(const struct DENSE_MATRIX matrix);

#ifdef DEBUG
/// @brief output debug information
/// @param left left matrix
/// @param right right matrix
/// @param result result matrix
void debug_info(struct ELLPACK left, struct ELLPACK right, struct ELLPACK result);

/// @brief output debug information
/// @param c name of matrix: 'l' -> "left"; 'r' -> "right"; 'o' -> "result"; everything else -> "matrix"
/// @param matrix matrix to output
void debug_info_single(char c, struct ELLPACK matrix);

/// @brief output debug information of a XMM matrix
/// @param matrix the matrix to debug
void debug_XMM(struct DENSE_MATRIX_XMM matrix);
#endif

#endif
