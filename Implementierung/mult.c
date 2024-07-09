#include "mult.h"
#include "util.h"
#include "ellpack.h"


/// @brief main function
/// @param a matrix a
/// @param b matrix b
/// @param result result of multiplication
void matr_mult_ellpack(const void* a, const void* b, void* result) {
    *(struct ELLPACK*)result = multiply_V1(*(struct ELLPACK*)a, *(struct ELLPACK*)b);

    // ! DEBUG
    const struct ELLPACK aE = *(struct ELLPACK*)a;
    const struct ELLPACK bE = *(struct ELLPACK*)b;
    struct ELLPACK res = *(struct ELLPACK*)result;

    pdebug("== a ==\n");
    pdebug("%lu\n", aE.noRows);
    pdebug("%lu\n", aE.noCols);
    pdebug("%lu\n", aE.maxNoNonZero);
    pdebug("");
    for (uint64_t i = 0; i < aE.noRows * aE.maxNoNonZero; i++) {
        pdebug_("%f, ", aE.values[i]);
    }
    pdebug_("\n");
    pdebug("");
    for (uint64_t i = 0; i < aE.noRows * aE.maxNoNonZero; i++) {
        pdebug_("%lu, ", aE.indices[i]);
    }
    pdebug_("\n\n");

    pdebug("== b ==\n");
    pdebug("%lu\n", bE.noRows);
    pdebug("%lu\n", bE.noCols);
    pdebug("%lu\n", bE.maxNoNonZero);
    pdebug("");
    for (uint64_t i = 0; i < bE.noRows * bE.maxNoNonZero; i++) {
        pdebug_("%f, ", bE.values[i]);
    }
    pdebug("\n");
    pdebug("");
    for (uint64_t i = 0; i < bE.noRows * bE.maxNoNonZero; i++) {
        pdebug_("%lu, ", bE.indices[i]);
    }
    pdebug_("\n\n");

    pdebug("== o ==\n");
    pdebug("%lu\n", res.noRows);
    pdebug("%lu\n", res.noCols);
    pdebug("%lu\n", res.maxNoNonZero);
    pdebug("");
    for (uint64_t i = 0; i < res.noRows * res.maxNoNonZero; i++) {
        pdebug_("%f, ", res.values[i]);
    }
    pdebug_("\n");
    pdebug("");
    for (uint64_t i = 0; i < res.noRows * res.maxNoNonZero; i++) {
        pdebug_("%lu, ", res.indices[i]);
    }
    pdebug_("\n");
}

/// @brief multiplies the matrices
/// @param left left matrix
/// @param right right matrix
/// @result multiplied matrix
struct ELLPACK multiply(struct ELLPACK left, struct ELLPACK right)
{
    // Array storing the amounts of non-zero entries per column in right    // TODO unused so far
    /*
    uint64_t* maxRowCounts = (uint64_t*)malloc(right.noRows * sizeof(uint64_t));
    if (maxRowCounts == NULL) {
        fprintf(stderr, "Error allocating memory");
        abort();
    }
    for (uint64_t i = 0; i < right.noRows; i++) {
        maxRowCounts[i] = 0;
    }
    for (uint64_t i = 0; i < right.noRows * right.maxNoNonZero; i++) {
        if (right.values[i] != 0.f) {
            maxRowCounts[right.indices[i]]++;
        }
    }
    uint64_t maxRow = 0;                     // maximum number of non-zero entries of columns of right // TODO unused so far
    for (uint64_t i = 0; i < right.noRows; i++) {
        if (maxRowCounts[i] > maxRow) {
            maxRow = maxRowCounts[i];
        }
    }
    free(maxRowCounts);
    */
    // ############################## Start used code ##############################
    // ############################## Setup: instantiating and memory allocation ##############################

    struct ELLPACK result;
    result.noRows = left.noRows;
    result.noCols = right.noCols;

    uint64_t maxNoNonZeroLimit = (right.noCols > left.maxNoNonZero * right.maxNoNonZero) ? left.maxNoNonZero * right.maxNoNonZero : right.noCols;
    result.maxNoNonZero = maxNoNonZeroLimit;      // TODO proven that this is correct yet

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

    uint64_t resultPos = 0;                  // pointer to next position to insert a value into result matrix
    uint64_t biggerDimension = (left.noRows > right.noCols) ? left.noRows : right.noCols;

    // ############################## calculation of actual values ##############################

    for (uint64_t i = 0; i < biggerDimension; i++) // Iterates over the rows of left
    {
        uint64_t leftRowIndex = i % left.noRows;

        for (uint64_t l = 0; l < right.noCols; l++) { // Iterates over the columns in the right matrix

            float sum = 0.f; // accumulator for an entry in result
            for (uint64_t j = 0; j < left.maxNoNonZero; j++) { // Iterates over a row of left

                // leftColRightRow is the column index of the left and row index of the right matrix
                uint64_t leftColRightRow = left.indices[leftRowIndex * left.maxNoNonZero + j];

                // add here: if padded (= invalid value (float of 0.0)) then break

                // Iterates over the column of right to check if value of right is not 0 / row of right has entry for that position
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
        for (; resultPos < (i + 1) * result.maxNoNonZero; resultPos++)
        {
            result.values[resultPos] = 0.f;
            result.indices[resultPos] = 0;
        }
    }

    // ############################## shrink result matrix by removing too long padding ##############################

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
        for (; realResultPointer < (i + 1) * realResultMaxNoNonZero; realResultPointer++)
        {
            result.values[realResultPointer] = 0.f;
            result.indices[realResultPointer] = 0;
        }
    }
    result.maxNoNonZero = realResultMaxNoNonZero;

    return result;
}

struct ELLPACK multiply_V1(struct ELLPACK left, struct ELLPACK right)
{
    // ############################## Setup: instantiating and memory allocation ##############################

    struct ELLPACK result;
    result.noRows = left.noRows;
    result.noCols = right.noCols;

    uint64_t maxNoNonZeroLimit = (right.noCols > left.maxNoNonZero * right.maxNoNonZero) ? left.maxNoNonZero * right.maxNoNonZero : right.noCols;
    result.maxNoNonZero = maxNoNonZeroLimit;      // TODO proven that this is correct yet

    result.values = (float*)malloc(result.noRows * result.maxNoNonZero * sizeof(float));
    result.indices = (uint64_t*)malloc(result.noRows * result.maxNoNonZero * sizeof(uint64_t));
    float* sum = malloc(right.noCols * sizeof(float));  // stores the products af a row of left with all columns of right
    uint64_t* positions = malloc(right.noCols * sizeof(uint64_t));
    if (result.values == NULL || result.indices == NULL || sum == NULL || positions == NULL) {
        if (result.values != NULL) {
            free(result.values);
            fprintf(stderr, "Error allocating memory");
        }
        if (result.indices != NULL) {
            free(result.indices);
            fprintf(stderr, "Error allocating memory");
        }
        if (sum != NULL) {
            free(sum);
            fprintf(stderr, "Error allocating memory");
        }
        if (positions != NULL) {
            free(positions);
            fprintf(stderr, "Error allocating memory");
        }
        exit(EXIT_FAILURE);
    }

    uint64_t resultPos = 0;                  // pointer to next position to insert a value into result matrix
    uint64_t biggerDimension = (left.noRows > right.noCols) ? left.noRows : right.noCols;
    for (uint64_t j = 0; j < right.noCols; j++) {
        sum[j] = 0.0;
    }

    // ############################## calculation of actual values ##############################

    for (uint64_t i = 0; i < biggerDimension; i++) // Iterates over the rows of left
    {
        uint64_t leftRowIndex = i % left.noRows;

        for (uint64_t j = 0; j < left.maxNoNonZero; j++) { // Iterates over a row of left
            uint64_t leftAccessIndex = leftRowIndex * left.maxNoNonZero + j;
            // leftColRightRow is the column index of the left and row index of the right matrix
            uint64_t leftColRightRow = left.indices[leftAccessIndex];

            // Iterates over the row of right
            for (uint64_t k = 0; k < right.maxNoNonZero; k++) {
                sum[j] += left.values[leftAccessIndex] * right.values[k + leftColRightRow * right.maxNoNonZero];
                positions[j] = leftColRightRow;
            }
        }
        // set value of result to calculated product
        for (uint64_t j = 0; j < right.noCols; j++)
        {
            if (sum[j] != 0.0) {
                result.indices[resultPos] = positions[j];
                result.values[resultPos++] = sum[j];
                sum[j] = 0.0;
            }
        }
        // add padding
        for (; resultPos < (i + 1) * result.maxNoNonZero; resultPos++)
        {
            result.values[resultPos] = 0.f;
            result.indices[resultPos] = 0;
        }
    }
    free(sum);
    free(positions);

    // ############################## shrink result matrix by removing too long padding ##############################

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
        for (; realResultPointer < (i + 1) * realResultMaxNoNonZero; realResultPointer++)
        {
            result.values[realResultPointer] = 0.f;
            result.indices[realResultPointer] = 0;
        }
    }
    result.maxNoNonZero = realResultMaxNoNonZero;
    return result;
}
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
