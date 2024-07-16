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

    //########################################## calculation of actual values ##########################################

    for (uint64_t i = 0; i < left.noRows; i++) {// Iterates over the rows of left
        for (uint64_t l = 0; l < right.noCols; l++) { // Iterates over the columns in the right matrix

            float sum = 0.f; // accumulator for an entry in result
            for (uint64_t j = 0; j < left.maxNoNonZero; j++) { // Iterates over a row of left

                // leftColRightRow is the column index of the left and row index of the right matrix
                uint64_t leftColRightRow = left.indices[i * left.maxNoNonZero + j];
                // Iterates over the column of right to check if row of right has entry for that position
                for (uint64_t k = leftColRightRow * right.maxNoNonZero; k < (leftColRightRow + 1) * right.maxNoNonZero; k++) {
                    if (right.indices[k] == l) {
                        sum += left.values[i * left.maxNoNonZero + j] * right.values[k];
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
    for (uint64_t j = 0; j < right.noCols; j++) {
        sum[j] = 0.0;
    }

    //########################################## calculation of actual values ##########################################

    for (uint64_t i = 0; i < left.noRows; i++) { // Iterates over the rows of left
        for (uint64_t j = 0; j < left.maxNoNonZero; j++) { // Iterates over a row of left
            uint64_t leftAccessIndex = i * left.maxNoNonZero + j;
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

void matr_mult_ellpack_V2(const void* a, const void* b, void* res) {
    const struct ELLPACK left = *(struct ELLPACK*)a;
    struct ELLPACK right = *(struct ELLPACK*)b;
    validate_inputs(left, right);
    struct ELLPACK result;
    result = initialize_result(left, right, result);
    right = transpose(right);
    uint64_t resultPos = 0;                  // pointer to next position to insert a value into result matrix

    //########################################## calculation of actual values ##########################################

    for (uint64_t i = 0; i < left.noRows; i++) {// Iterates over the rows of left
        for (uint64_t j = 0; j < right.noRows; j++) { // Iterates over the columns in the right matrix
            float sum = 0.f; // accumulator for an entry in result
            uint64_t leftRowPointer = left.maxNoNonZero * i;
            uint64_t rightRowPointer = right.maxNoNonZero * j;

            while (leftRowPointer < left.maxNoNonZero * (i + 1) && rightRowPointer < right.maxNoNonZero * (j + 1)) {
                if (left.indices[leftRowPointer] < right.indices[rightRowPointer]) {
                    leftRowPointer++;
                }
                else if (left.indices[leftRowPointer] > right.indices[rightRowPointer]) {
                    rightRowPointer++;
                }
                else {
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

void matr_mult_ellpack_V3(const void* a, const void* b, void* res) {
    validate_inputs(*(struct ELLPACK*)a, *(struct ELLPACK*)b);
    struct ELLPACK result;
    result = initialize_result(*(struct ELLPACK*)a, *(struct ELLPACK*)b, result);
    struct DENSE_MATRIX left = to_dense(*(struct ELLPACK*)a);
    struct DENSE_MATRIX right = to_dense(*(struct ELLPACK*)b);
    uint64_t resultPos = 0;                  // pointer to next position to insert a value into result matrix

    //########################################## calculation of actual values ##########################################

    for (uint64_t i = 0; i < left.noRows; i++) {// Iterates over the rows of left
        for (uint64_t j = 0; j < right.noCols; j++) { // Iterates over the columns in the right matrix
            float sum = 0.f; // accumulator for an entry in result

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

void matr_mult_ellpack_V4(const void* a, const void* b, void* res) {
    validate_inputs(*(struct ELLPACK*)a, *(struct ELLPACK*)b);
    struct ELLPACK result;
    result = initialize_result(*(struct ELLPACK*)a, *(struct ELLPACK*)b, result);
    struct DENSE_MATRIX_XMM left = to_XMM(to_dense(*(struct ELLPACK*)a));
    struct DENSE_MATRIX_XMM right = to_XMM(to_dense(transpose(*(struct ELLPACK*)b)));
    uint64_t resultPos = 0;                  // pointer to next position to insert a value into result matrix

    //########################################## calculation of actual values ##########################################

    for (uint64_t i = 0; i < left.noRows; i++) {// Iterates over the rows of left
        for (uint64_t j = 0; j < right.noRows; j++) { // Iterates over the columns in the right matrix
            __m128 sum = _mm_setzero_ps(); // accumulator for an entry in result

            for (uint64_t k = 0; k < left.noQuadCols; k++) {
                sum += left.values[i * left.noQuadCols + k] * right.values[j * right.noQuadCols + k];
            }

            sum = _mm_hadd_ps(sum, sum);
            sum = _mm_hadd_ps(sum, sum);
            float fsum = _mm_cvtss_f32(sum);
            /*sum = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(sum), 4));
            fsum += _mm_cvtss_f32(sum);
            sum = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(sum), 4));
            fsum += _mm_cvtss_f32(sum);
            sum = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(sum), 4));
            fsum += _mm_cvtss_f32(sum);*/
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

// TODO: free matrices when invalid
/// @brief check for valid inputs: multiplicable dimensions, no indices larger than matrix dimensions and only ascending indices
/// @param left left matrix
/// @param right right matrix
void validate_inputs(struct ELLPACK left, struct ELLPACK right) {
    if (left.noCols != right.noRows) {
        fprintf(stderr, "Error: the given matrices do not have multiplicable dimensions: %lux%lu * %lux%lu\n", left.noRows, left.noCols, right.noRows, right.noCols);
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

/// @brief transposes the given matrix and returns the result
struct ELLPACK transpose(struct ELLPACK matrix) {
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
    trans.values = (float*)malloc(trans.noRows * trans.maxNoNonZero * sizeof(float));
    trans.indices = (uint64_t*)malloc(trans.noRows * trans.maxNoNonZero * sizeof(uint64_t));
    if (trans.values == NULL || trans.indices == NULL) {
        if (trans.values != NULL) {
            free(trans.values);
            fprintf(stderr, "Error allocating memory");
        }
        if (trans.indices != NULL) {
            free(trans.indices);
            fprintf(stderr, "Error allocating memory");
        }
        exit(EXIT_FAILURE);
    }

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
struct DENSE_MATRIX to_dense(struct ELLPACK matrix) {
    struct DENSE_MATRIX result;
    result.noRows = matrix.noRows;
    result.noCols = matrix.noCols;
    result.values = (float*)malloc(result.noRows * result.noCols * sizeof(float));
    if (result.values == NULL) {
        fprintf(stderr, "Error allocating memory");
        exit(EXIT_FAILURE);
    }
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
struct DENSE_MATRIX_XMM to_XMM(struct DENSE_MATRIX matrix) {
    struct DENSE_MATRIX_XMM result;
    result.noRows = matrix.noRows;
    result.noCols = matrix.noCols;
    uint64_t noQuadCol = (matrix.noCols + 3) / 4;  // (x + 3) / 4 = ceil(x/4)
    result.noQuadCols = noQuadCol;
    result.values = (__m128*)malloc(result.noRows * noQuadCol * sizeof(__m128));
    if (result.values == NULL) {
        fprintf(stderr, "Error allocating memory");
        exit(EXIT_FAILURE);
    }
    for (uint64_t i = 0; i < result.noRows; i++) {
        for (uint64_t j = 0; j < noQuadCol - 1; j++) {
            result.values[i * noQuadCol + j] = _mm_set_ps(matrix.values[i * matrix.noCols + 4 * j], matrix.values[i * matrix.noCols + 4 * j + 1], matrix.values[i * matrix.noCols + 4 * j + 2], matrix.values[i * matrix.noCols + 4 * j + 3]);
        }
        float f1 = matrix.values[i * matrix.noCols + 4 * noQuadCol - 4];
        float f2 = (4 * noQuadCol - 3 >= matrix.noCols) ? 0.f : matrix.values[i * matrix.noCols + 4 * noQuadCol - 3];
        float f3 = (4 * noQuadCol - 2 >= matrix.noCols) ? 0.f : matrix.values[i * matrix.noCols + 4 * noQuadCol - 2];
        float f4 = (4 * noQuadCol - 1 >= matrix.noCols) ? 0.f : matrix.values[i * matrix.noCols + 4 * noQuadCol - 1];
        result.values[i * noQuadCol + noQuadCol - 1] = _mm_set_ps(f1, f2, f3, f4);
    }
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

void debug_XMM(struct DENSE_MATRIX_XMM matrix){
    pdebug("%lu, %lu, %lu\n", matrix.noRows, matrix.noCols, matrix.noQuadCols);
    for(uint64_t i = 0; i < matrix.noRows; i++) {
        for(uint64_t j = 0; j < matrix.noQuadCols; j++) {
            __m128 value = matrix.values[i * matrix.noQuadCols + j];
            float f1 = _mm_cvtss_f32(value);
            value = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(value), 4));
            float f2 = _mm_cvtss_f32(value);
            value = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(value), 4));
            float f3= _mm_cvtss_f32(value);
            value = _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(value), 4));
            float f4 = _mm_cvtss_f32(value);
            pdebug("%f, %f, %f, %f, ", f4, f3, f2, f1);
        }
        pdebug_("\n");
    }
}

/*
 *  Notes for Simon: commands for testing
 *      make test/run/debug/...
 *      ./main -a tests/generated/1/a -b tests/generated/1/b
 *      ./main -h
 *      ./tests/generate_input.py
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
