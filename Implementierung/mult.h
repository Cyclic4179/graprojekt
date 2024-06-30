#ifndef GUARD_MAIN
#define GUARD_MAIN

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


/// @brief main function
/// @param a matrix a
/// @param b matrix b
/// @param result result of multiplication
void matr_mult_ellpack(const void* a, const void* b, void* result);

/// @brief multiplies the matrices
/// @param left left matrix
/// @param right right matrix
/// @result multiplied matrix
struct ELLPACK multiply(struct ELLPACK left, struct ELLPACK right);

#endif
