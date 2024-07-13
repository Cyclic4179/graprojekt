#include "mult.h"
#include "util.h"
#include "ellpack.h"

/// @brief main multiplication, first version
/// @param a Pointer to left matrix
/// @param b Pointer to right matrix
/// @param res Pointer to result of multiplication
void matr_mult_ellpack(const void* a, const void* b, void* res) {
    const struct ELLPACK left = *(struct ELLPACK*)a;
    const struct ELLPACK right = *(struct ELLPACK*)b;
    validate_inputs(left, right);
    struct ELLPACK result;
    result = initialize_result(left, right, result);

    uint64_t resultPos = 0;                  // pointer to next position to insert a value into result matrix
    uint64_t biggerDimension = (left.noRows > right.noCols) ? left.noRows : right.noCols;

    //########################################## calculation of actual values ##########################################

    for (uint64_t i = 0; i < biggerDimension; i++) {// Iterates over the rows of left

        uint64_t leftRowIndex = i % left.noRows;
        for (uint64_t l = 0; l < right.noCols; l++) { // Iterates over the columns in the right matrix

            float sum = 0.f; // accumulator for an entry in result
            for (uint64_t j = 0; j < left.maxNoNonZero; j++) { // Iterates over a row of left

                // leftColRightRow is the column index of the left and row index of the right matrix
                uint64_t leftColRightRow = left.indices[leftRowIndex * left.maxNoNonZero + j];
                // Iterates over the column of right to check if row of right has entry for that position
                for (uint64_t k = leftColRightRow * right.maxNoNonZero; k < (leftColRightRow + 1) * right.maxNoNonZero; k++) {
                    if (right.indices[k] == l) {
                        sum += left.values[leftRowIndex * left.maxNoNonZero + j] * right.values[k];
                    }
                }
            }
            // set value of result to calculated product
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

/// @brief main multiplication, second version
/// @param a Pointer to left matrix
/// @param b Pointer to right matrix
/// @param res Pointer to result of multiplication
void matr_mult_ellpack_V1(const void* a, const void* b, void* res) {
    const struct ELLPACK left = *(struct ELLPACK*)a;
    const struct ELLPACK right = *(struct ELLPACK*)b;
    validate_inputs(left, right);
    struct ELLPACK result;
    result = initialize_result(left, right, result);
    *(struct ELLPACK*)res = result;

    float* sum = malloc(right.noCols * sizeof(float));  // stores the products af a row of left with all columns of right
    if (sum == NULL) {
        free(result.values);
        free(result.indices);
        fprintf(stderr, "Error allocating memory");
        exit(EXIT_FAILURE);
    }

    uint64_t resultPos = 0;                  // pointer to next position to insert a value into result matrix
    uint64_t biggerDimension = (left.noRows > right.noCols) ? left.noRows : right.noCols;
    for (uint64_t j = 0; j < right.noCols; j++) {
        sum[j] = 0.0;
    }

    //########################################## calculation of actual values ##########################################

    for (uint64_t i = 0; i < biggerDimension; i++) { // Iterates over the rows of left
        uint64_t leftRowIndex = i % left.noRows;

        for (uint64_t j = 0; j < left.maxNoNonZero; j++) { // Iterates over a row of left
            uint64_t leftAccessIndex = leftRowIndex * left.maxNoNonZero + j;
            // leftColRightRow is the column index of the left and row index of the right matrix
            uint64_t leftColRightRow = left.indices[leftAccessIndex];

            // Iterates over the row of right
            for (uint64_t k = leftColRightRow * right.maxNoNonZero; k < (leftColRightRow + 1) * right.maxNoNonZero; k++) {
                sum[right.indices[k]] += left.values[leftAccessIndex] * right.values[k];
            }
        }
        // set value of result to calculated product
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

/// @brief check for valid inputs: multiplicable dimensions, no indices larger than matrix dimensions and only ascending indices
/// @param left left matrix
/// @param right right matrix
void validate_inputs(struct ELLPACK left, struct ELLPACK right) {
    if (left.noCols != right.noRows) {
        fprintf(stderr, "Error: the given matrices do not have multiplicable dimensions.\n");
        exit(EXIT_FAILURE);
    }
    validate_matrix(left);
    validate_matrix(right);
}

/// @brief check for no indices larger than matrix dimensions and only ascending indices
/// @param matrix matrix to check
void validate_matrix(struct ELLPACK matrix) {
    uint64_t index = 0;
    int padding;
    for (uint64_t i = 0; i < matrix.noRows; i++) {
        padding = 0;
        for (uint64_t j = 0; j < matrix.maxNoNonZero; j++) {
            uint64_t accessIndex = i * matrix.maxNoNonZero + j;
            if (matrix.indices[accessIndex] >= matrix.noCols) {
                fprintf(stderr, "ERROR: Index %lu too large for a %lux%lu matrix.\n", matrix.indices[accessIndex], matrix.noRows, matrix.noCols);
                exit(EXIT_FAILURE);
            }
            if (padding == 0 && matrix.indices[accessIndex] == 0 && matrix.values[accessIndex] == 0.f) {
                padding = 1;
            }
            if (matrix.indices[accessIndex] <= index && j != 0 && padding == 0) {
                fprintf(stderr, "ERROR: Indices not in ascending order in row %lu at index %lu: index %lu not greater than previous index %lu.\n", i, j, matrix.indices[accessIndex], matrix.indices[accessIndex - 1]);
                exit(EXIT_FAILURE);
            }
            index = matrix.indices[accessIndex];
        }
    }
}

/// @brief initialize the result matrix
/// @param left left matrix
/// @param right right matrix
/// @param result result matrix
/// @result initialized result matrix
struct ELLPACK initialize_result(struct ELLPACK left, struct ELLPACK right, struct ELLPACK result) {
    result.noRows = left.noRows;
    result.noCols = right.noCols;
    result.maxNoNonZero = (right.noCols > left.maxNoNonZero * right.maxNoNonZero) ? left.maxNoNonZero * right.maxNoNonZero : right.noCols; //Proven by Pierre that this limit is correct
    result.values = (float*)malloc(result.noRows * result.maxNoNonZero * sizeof(float));
    result.indices = (uint64_t*)malloc(result.noRows * result.maxNoNonZero * sizeof(uint64_t));
    if (result.values == NULL || result.indices == NULL) {
        if (result.values != NULL) {
            free(result.values);
            fprintf(stderr, "Error allocating memory");
        }
        if (result.indices != NULL) {
            free(result.indices);
            fprintf(stderr, "Error allocating memory");
        }
        exit(EXIT_FAILURE);
    }
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
/*
 *  Notes for Simon: commands for testing
 *      make test/run/debug/...
 *      ./main -a tests/generated/1/a -b tests/generated/1/b
 *      ./main -h
 *      ./generate_input.py
 */

 /*
  * Step 1: read matrices
  * Step 2: make sure everything is valid
  *      Sample Format:
  *      LINE|   CONTENT
  *      1   |   <noRows>,<noCols>,<noNonZero>
  *      2   |   <values>
  *      3   |   <indices>
  *
  *      The input Files are in a right format: values comma separated, only floats (no int, letters, etc.)
  *      The output File is large enough to store the result
  *      The matrices are have valid dimensions that are >= 1 and can be multiplied
  *      The matrices fulfill the conditions:
  *          noRows * noNonZeros = values.length     (amount of Floats in values)
  *          noRows * noNonZeros = indices.length     (amount of Floats in indices)
  *          at every index where values has padding (*), indices must have padding and vice versa
  *          every row starts with (noNonZero - x) non-paddings (valid floats), followed by x paddings
  *              no padding is followed by a value in the same row
  * Step 3: the actual matrix multiplication
  */
