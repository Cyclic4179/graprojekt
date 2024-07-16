#include "mult.h"

#include <pmmintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <xmmintrin.h>

#include "ellpack.h"
#include "util.h"

/// @brief second version, searching corresponding values in right matrix for every entry in left matrix
/// @param a Pointer to left matrix
/// @param b Pointer to right matrix
/// @param res Pointer to result of multiplication
void matr_mult_ellpack_V1(const void* a, const void* b, void* res) {
    const struct ELLPACK left = *(struct ELLPACK*)a;
    const struct ELLPACK right = *(struct ELLPACK*)b;
    validate_inputs(left, right);
    struct ELLPACK result;
    result = initialize_result(left, right, result);
    uint64_t resultPos = 0;  // pointer to next position to insert a value into result matrix

    /* -------------------- calculation of actual values -------------------- */

    for (uint64_t i = 0; i < left.noRows; i++) {       // Iterates over the rows of left
        for (uint64_t l = 0; l < right.noCols; l++) {  // Iterates over the columns in the right matrix

            float sum = 0.f;                                    // accumulator for an entry in result
            for (uint64_t j = 0; j < left.maxNoNonZero; j++) {  // Iterates over a row of left

                // leftColRightRow is the column index of the left and row index of the right matrix
                uint64_t leftColRightRow = left.indices[i * left.maxNoNonZero + j];
                // Iterates over the column of right to check if row of right has entry for that position
                for (uint64_t k = leftColRightRow * right.maxNoNonZero; k < (leftColRightRow + 1) * right.maxNoNonZero;
                     k++) {
                    if (right.indices[k] == l) {
                        sum += left.values[i * left.maxNoNonZero + j] * right.values[k];
                    }
                }
            }
            // set the value of result to calculated product
            if (sum != 0.0) {
                result.indices[resultPos] = l;
                result.values[resultPos++] = sum;
            }
        }
        // add padding
        for (; resultPos < (i + 1) * result.maxNoNonZero; resultPos++) {
            result.values[resultPos] = 0.f;
            result.indices[resultPos] = 0;
        }
    }
    *(struct ELLPACK*)res = remove_unnecessary_padding(result);
}

/// @brief first and main version, optimized seach for corresponding value in right matrix compared to second version
void matr_mult_ellpack(const void* a, const void* b, void* res) {
    const struct ELLPACK left = *(struct ELLPACK*)a;
    const struct ELLPACK right = *(struct ELLPACK*)b;
    validate_inputs(left, right);
    struct ELLPACK result;
    result = initialize_result(left, right, result);

    // stores the products of a row of left with all columns of right
    float* sum = (float*)abortIfNULL(malloc(right.noCols * sizeof(float)));

    uint64_t resultPos = 0;                        // pointer to next position to insert a value into result matrix
    for (uint64_t j = 0; j < right.noCols; j++) {  // initialize all values with 0
        sum[j] = 0.0;
    }

    /* -------------------- calculation of actual values -------------------- */

    for (uint64_t i = 0; i < left.noRows; i++) {            // Iterates over the rows of left
        for (uint64_t j = 0; j < left.maxNoNonZero; j++) {  // Iterates over a row of left
            uint64_t leftAccessIndex = i * left.maxNoNonZero + j;
            // leftColRightRow is the column index of the left and row index of the right matrix
            uint64_t leftColRightRow = left.indices[leftAccessIndex];

            // Iterates over the row of right for which left has a non-zero entry and adds the product to the array
            for (uint64_t k = leftColRightRow * right.maxNoNonZero; k < (leftColRightRow + 1) * right.maxNoNonZero;
                 k++) {
                sum[right.indices[k]] += left.values[leftAccessIndex] * right.values[k];
            }
        }
        // set the values of result to calculated products: iterates over and fills a complete row in result
        for (uint64_t j = 0; j < right.noCols; j++) {
            if (sum[j] != 0.0) {
                result.indices[resultPos] = j;
                result.values[resultPos++] = sum[j];
                sum[j] = 0.0;
            }
        }
        // add padding
        for (; resultPos < (i + 1) * result.maxNoNonZero; resultPos++) {
            result.values[resultPos] = 0.f;
            result.indices[resultPos] = 0;
        }
    }
    free(sum);
    *(struct ELLPACK*)res = remove_unnecessary_padding(result);
}

/// @brief third version, working on transposed right matrix for better cache compatibility,
void matr_mult_ellpack_V2(const void* a, const void* b, void* res) {
    const struct ELLPACK left = *(struct ELLPACK*)a;
    struct ELLPACK right = *(struct ELLPACK*)b;
    validate_inputs(left, right);
    struct ELLPACK result;
    result = initialize_result(left, right, result);
    right = transpose(right);
    uint64_t resultPos = 0;  // pointer to next position to insert a value into result matrix

    /* -------------------- calculation of actual values -------------------- */

    for (uint64_t i = 0; i < left.noRows; i++) {       // Iterates over the rows of left
        for (uint64_t j = 0; j < right.noRows; j++) {  // Iterates over the columns in the right matrix
            float sum = 0.f;                           // accumulator for an entry in result
            uint64_t leftRowPointer = left.maxNoNonZero * i;
            uint64_t rightRowPointer = right.maxNoNonZero * j;

            // Iterates over the rows of the (transposed) matrices, incrementing the pointer with the lower Index
            while (leftRowPointer < left.maxNoNonZero * (i + 1) && rightRowPointer < right.maxNoNonZero * (j + 1)) {
                if (left.indices[leftRowPointer] < right.indices[rightRowPointer]) {
                    leftRowPointer++;
                } else if (left.indices[leftRowPointer] > right.indices[rightRowPointer]) {
                    rightRowPointer++;
                } else {
                    sum += left.values[leftRowPointer++] * right.values[rightRowPointer++];
                }
            }

            // set value of result to calculated product
            if (sum != 0.0) {
                result.indices[resultPos] = j;
                result.values[resultPos++] = sum;
            }
        }
        // add padding
        for (; resultPos < (i + 1) * result.maxNoNonZero; resultPos++) {
            result.values[resultPos] = 0.f;
            result.indices[resultPos] = 0;
        }
    }
    *(struct ELLPACK*)res = remove_unnecessary_padding(result);
}

/// @brief fourth version, working on a dense matrix, for almost dense matrices more memory efficient and simpler
void matr_mult_ellpack_V3(const void* a, const void* b, void* res) {
    validate_inputs(*(struct ELLPACK*)a, *(struct ELLPACK*)b);
    struct ELLPACK result;
    result = initialize_result(*(struct ELLPACK*)a, *(struct ELLPACK*)b, result);
    const struct DENSE_MATRIX left = to_dense(*(struct ELLPACK*)a);
    const struct DENSE_MATRIX right = to_dense(*(struct ELLPACK*)b);
    uint64_t resultPos = 0;  // pointer to next position to insert a value into result matrix

    /* -------------------- calculation of actual values -------------------- */

    for (uint64_t i = 0; i < left.noRows; i++) {       // Iterates over the rows of left
        for (uint64_t j = 0; j < right.noCols; j++) {  // Iterates over the columns in the right matrix
            float sum = 0.f;                           // accumulator for an entry in result

            for (uint64_t k = 0; k < left.noCols; k++) {
                sum += left.values[i * left.noCols + k] * right.values[k * right.noCols + j];
            }

            // set value of result to calculated product
            if (sum != 0.0) {
                result.indices[resultPos] = j;
                result.values[resultPos++] = sum;
            }
        }
        // add padding
        for (; resultPos < (i + 1) * result.maxNoNonZero; resultPos++) {
            result.values[resultPos] = 0.f;
            result.indices[resultPos] = 0;
        }
    }
    *(struct ELLPACK*)res = remove_unnecessary_padding(result);
}

/// @brief fifth version, optimized for fast almost-dense matrices multiplication by using SIMD with Intrinsics
void matr_mult_ellpack_V4(const void* a, const void* b, void* res) {
    validate_inputs(*(struct ELLPACK*)a, *(struct ELLPACK*)b);
    struct ELLPACK result;
    result = initialize_result(*(struct ELLPACK*)a, *(struct ELLPACK*)b, result);
    const struct DENSE_MATRIX_XMM left = to_XMM(to_dense(*(struct ELLPACK*)a));
    const struct DENSE_MATRIX_XMM right = to_XMM(to_dense(transpose(*(struct ELLPACK*)b)));
    uint64_t resultPos = 0;  // pointer to next position to insert a value into result matrix

    /* -------------------- calculation of actual values -------------------- */

    for (uint64_t i = 0; i < left.noRows; i++) {       // Iterates over the rows of left
        for (uint64_t j = 0; j < right.noRows; j++) {  // Iterates over the columns in the right matrix
            __m128 sum = _mm_setzero_ps();             // accumulator for an entry in result

            for (uint64_t k = 0; k < left.noQuadCols; k++) {
                sum += left.values[i * left.noQuadCols + k] * right.values[j * right.noQuadCols + k];
            }

            sum = _mm_hadd_ps(sum, sum);
            sum = _mm_hadd_ps(sum, sum);
            float fsum = _mm_cvtss_f32(sum);
            // set value of result to calculated product
            if (fsum != 0.0) {
                result.indices[resultPos] = j;
                result.values[resultPos++] = fsum;
            }
        }
        // add padding
        for (; resultPos < (i + 1) * result.maxNoNonZero; resultPos++) {
            result.values[resultPos] = 0.f;
            result.indices[resultPos] = 0;
        }
    }
    *(struct ELLPACK*)res = remove_unnecessary_padding(result);
}

/// @brief check for valid inputs: multiplicable dimensions
/// @param left left matrix
/// @param right right matrix
void validate_inputs(struct ELLPACK left, struct ELLPACK right) {
    if (left.noCols != right.noRows) {
        fprintf(stderr, "Error: the given matrices do not have multiplicable dimensions: %lux%lu * %lux%lu\n",
                left.noRows, left.noCols, right.noRows, right.noCols);
        exit(EXIT_FAILURE);
    }
}

/// @brief initialize the result matrix
/// @param left left matrix
/// @param right right matrix
/// @param result result matrix
/// @result initialized result matrix
struct ELLPACK initialize_result(const struct ELLPACK left, const struct ELLPACK right, struct ELLPACK result) {
    result.noRows = left.noRows;
    result.noCols = right.noCols;
    result.maxNoNonZero = (right.noCols > left.maxNoNonZero * right.maxNoNonZero)
                              ? left.maxNoNonZero * right.maxNoNonZero
                              : right.noCols;  // Proven by Pierre that this limit is correct
    result.values = (float*)abortIfNULL(malloc(result.noRows * result.maxNoNonZero * sizeof(float)));
    result.indices = (uint64_t*)abortIfNULL(malloc(result.noRows * result.maxNoNonZero * sizeof(uint64_t)));
    return result;
}

/// @brief remove unnecessary padding in the result matrix and // still TODO: free the unused memory
/// @param result result matrix
/// @result smaller matrix
struct ELLPACK remove_unnecessary_padding(struct ELLPACK result) {
    uint64_t realResultMaxNoNonZero = 0;
    uint64_t rowCounter;

    for (uint64_t i = 0; i < result.noRows; i++) {
        rowCounter = 0;
        for (uint64_t j = 0; j < result.maxNoNonZero; ++j) {
            if (result.values[result.maxNoNonZero * i + j] != 0.f) {
                rowCounter++;
            }
        }
        if (rowCounter > realResultMaxNoNonZero) {
            realResultMaxNoNonZero = rowCounter;
        }
    }

    uint64_t realResultPointer = 0;
    for (uint64_t i = 0; i < result.noRows; i++) {
        for (uint64_t j = 0; j < result.maxNoNonZero; j++) {
            if (result.values[i * result.maxNoNonZero + j] != 0.f) {
                result.values[realResultPointer] = result.values[i * result.maxNoNonZero + j];
                result.indices[realResultPointer] = result.indices[i * result.maxNoNonZero + j];
                realResultPointer++;
            }
        }
        for (; realResultPointer < (i + 1) * realResultMaxNoNonZero; realResultPointer++) {
            result.values[realResultPointer] = 0.f;
            result.indices[realResultPointer] = 0;
        }
    }
    result.maxNoNonZero = realResultMaxNoNonZero;
    return result;
}

/// @brief transposes the given matrix and returns the result
struct ELLPACK transpose(const struct ELLPACK matrix) {
    struct ELLPACK trans;
    trans.noRows = matrix.noCols;
    trans.noCols = matrix.noRows;
    uint64_t max = 0;
    uint64_t counter;
    for (uint64_t i = 0; i < matrix.noCols; i++) {
        counter = 0;
        for (uint64_t j = 0; j < matrix.noRows * matrix.maxNoNonZero; j++) {
            if (matrix.indices[j] == i && matrix.values[j] != 0.f) {
                counter++;
            }
        }
        if (counter > max) {
            max = counter;
        }
    }
    trans.maxNoNonZero = max;
    trans.values = (float*)abortIfNULL(malloc(trans.noRows * trans.maxNoNonZero * sizeof(float)));
    trans.indices = (uint64_t*)abortIfNULL(malloc(trans.noRows * trans.maxNoNonZero * sizeof(uint64_t)));

    uint64_t tpointer = 0;
    uint64_t index;
    for (uint64_t i = 0; i < matrix.noCols; i++) {
        for (uint64_t j = 0; j < matrix.noRows; j++) {
            for (uint64_t k = 0; k < matrix.maxNoNonZero; k++) {
                index = j * matrix.maxNoNonZero + k;
                if (matrix.indices[index] == i && matrix.values[index] != 0.f) {
                    trans.values[tpointer] = matrix.values[index];
                    trans.indices[tpointer++] = j;
                }
            }
        }
        for (; tpointer < (i + 1) * trans.maxNoNonZero; tpointer++) {
            trans.values[tpointer] = 0.f;
            trans.indices[tpointer] = 0;
        }
    }
    return trans;
}

/// @brief transforms a sparse matrix of ELLPACK format to a dense matrix and returns it
struct DENSE_MATRIX to_dense(const struct ELLPACK matrix) {
    struct DENSE_MATRIX result;
    result.noRows = matrix.noRows;
    result.noCols = matrix.noCols;
    result.values = (float*)abortIfNULL(malloc(result.noRows * result.noCols * sizeof(float)));
    for (uint64_t i = 0; i < result.noRows; i++) {
        uint64_t matrixPointer = i * matrix.maxNoNonZero;
        uint64_t matrixPLimit = matrixPointer + matrix.maxNoNonZero;
        for (uint64_t j = 0; j < result.noCols; j++) {
            if (matrix.indices[matrixPointer] == j && matrixPointer < matrixPLimit) {
                result.values[i * result.noCols + j] = matrix.values[matrixPointer++];
            } else {
                result.values[i * result.noCols + j] = 0.f;
            }
        }
    }
    return result;
}

/// @brief transforms the values of the given dense matrix into XMM float values
struct DENSE_MATRIX_XMM to_XMM(const struct DENSE_MATRIX matrix) {
    struct DENSE_MATRIX_XMM result;
    result.noRows = matrix.noRows;
    result.noCols = matrix.noCols;
    uint64_t noQuadCol = (matrix.noCols + 3) / 4;  // (x + 3) / 4 = ceil(x/4)
    result.noQuadCols = noQuadCol;
    result.values = (__m128*)abortIfNULL(malloc(result.noRows * noQuadCol * sizeof(__m128)));
    for (uint64_t i = 0; i < result.noRows; i++) {
        for (uint64_t j = 0; j < noQuadCol - 1; j++) {
            result.values[i * noQuadCol + j] =
                _mm_set_ps(matrix.values[i * matrix.noCols + 4 * j], matrix.values[i * matrix.noCols + 4 * j + 1],
                           matrix.values[i * matrix.noCols + 4 * j + 2], matrix.values[i * matrix.noCols + 4 * j + 3]);
        }
        float f1 = matrix.values[i * matrix.noCols + 4 * noQuadCol - 4];
        float f2 = (4 * noQuadCol - 3 >= matrix.noCols) ? 0.f : matrix.values[i * matrix.noCols + 4 * noQuadCol - 3];
        float f3 = (4 * noQuadCol - 2 >= matrix.noCols) ? 0.f : matrix.values[i * matrix.noCols + 4 * noQuadCol - 2];
        float f4 = (4 * noQuadCol - 1 >= matrix.noCols) ? 0.f : matrix.values[i * matrix.noCols + 4 * noQuadCol - 1];
        result.values[i * noQuadCol + noQuadCol - 1] = _mm_set_ps(f1, f2, f3, f4);
    }
    return result;
}

#ifdef DEBUG
/// @brief output debug information
/// @param left left matrix
/// @param right right matrix
/// @param result result matrix
void debug_info(struct ELLPACK left, struct ELLPACK right, struct ELLPACK result) {
    debug_info_single('l', left);
    debug_info_single('r', right);
    debug_info_single('o', result);
}

/// @brief output debug information
/// @param c name of matrix: 'l' -> "left"; 'r' -> "right"; 'o' -> "result"; everything else -> "matrix"
/// @param matrix matrix to output
void debug_info_single(char c, struct ELLPACK matrix) {
    switch (c) {
        case 'l':
            pdebug("== left ==\n");
            break;
        case 'r':
            pdebug("== right ==\n");
            break;
        case 'o':
            pdebug("== result ==\n");
            break;
        default:
            pdebug("== matrix ==\n");
            break;
    }
    pdebug("%lu, %lu, %lu\n", matrix.noRows, matrix.noCols, matrix.maxNoNonZero);
    for (uint64_t i = 0; i < matrix.noRows * matrix.maxNoNonZero; i++) {
        pdebug_("%f, ", matrix.values[i]);
    }
    pdebug_("\n");
    for (uint64_t i = 0; i < matrix.noRows * matrix.maxNoNonZero; i++) {
        pdebug_("%lu, ", matrix.indices[i]);
    }
    pdebug_("\n");
}

/// @brief output debug information of a XMM matrix
/// @param matrix the matrix to debug
void debug_XMM(struct DENSE_MATRIX_XMM matrix) {
    pdebug("%lu, %lu, %lu\n", matrix.noRows, matrix.noCols, matrix.noQuadCols);
    for (uint64_t i = 0; i < matrix.noRows; i++) {
        for (uint64_t j = 0; j < matrix.noQuadCols; j++) {
            __m128 value = matrix.values[i * matrix.noQuadCols + j];
            float f1 = _mm_cvtss_f32(value);
            value = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(value), 4));
            float f2 = _mm_cvtss_f32(value);
            value = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(value), 4));
            float f3 = _mm_cvtss_f32(value);
            value = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(value), 4));
            float f4 = _mm_cvtss_f32(value);
            pdebug("%f, %f, %f, %f, ", f4, f3, f2, f1);
        }
        pdebug_("\n");
    }
}
#endif

/*
 *  Notes for Simon: commands for testing
 *      make test/run/debug/...
 *      ./main -a tests/generated/1/a -b tests/generated/1/b
 *      ./main -h
 *      ./tests/generate_input.py
 */
