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

#endif
