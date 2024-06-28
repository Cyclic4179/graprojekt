//
// Created by Simon on 20.06.2024.
//
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


struct ELLPACK{
    uint64_t noRows;
    uint64_t noCols;
    uint64_t maxNoNonZero;
    uint64_t* colPositions;
    float* values;
};

// this is the main method
void matr_mult_ellpack(const void* a, const void* b, void* result) {}

void multiply(struct ELLPACK left, struct ELLPACK right, struct ELLPACK result) {
    result.noRows = left.noRows;                        // number of Rows in result matrix
    result.noCols = right.noCols;                       // number of Columns in result matrix
    result.maxNoNonZero = left.maxNoNonZero;            // maximum number of non-zero entries in result TODO not correct yet
    uint64_t resultPos = 0;                             // pointer to next position to insert a value into result matrix
    
    uint64_t biggerDimension = (left.noRows > right.noCols) ? left.noRows : right.noCols;
    for (uint64_t i = 0; i < biggerDimension; i++) {        // Iterates over the rows of left
        float sum = 0.f;                                // accumulator for product of      
        for (int j = 0; j < left.maxNoNonZero; j++) {   // Iterates over a row of left
            uint64_t leftIndex = i % left.noRows;
            // leftColRightRow is the column index of the left and row index of the right matrix
            uint64_t leftColRightRow = left.colPositions[leftIndex * left.maxNoNonZero + j];
            
            //add here: if padded (= invalid value (float of 0.0)) then break

            // Iterates over the column of right to check if value of right is not 0 / row of right has entry for that position
            for (uint64_t k = leftColRightRow * right.maxNoNonZero; k < (leftColRightRow + 1) * right.maxNoNonZero; k++) {
                if (right.colPositions[k] == i % right.noCols) { // TODO: i % right.noCols correct?
                    sum += left.values[leftColRightRow * left.maxNoNonZero + j] * right.values[k];
                }
            }
            // set value of result to calculated product
            result.colPositions[resultPos] = leftColRightRow;
            result.values[resultPos++] = sum;
        }
        // add padding
        for (;resultPos < (i + 1) * result.maxNoNonZero; resultPos++) {
            result.values[resultPos] = 0.f;
            result.colPositions[resultPos] = result.noCols - 1;
        }
    }
}

int main () {
    uint64_t leftColPos[] = {1, 2, 3, 1, 2, 3};
    float leftVals[] = {1.f, 2.f, 3.f, 4.f, 5.f, 6.f};
    uint64_t rightColPos[] = {1, 1, 1};
    float rightVals[] = {1.f, 2.f, 3.f};
    struct ELLPACK left = {.noRows = 2, .noCols = 3, .maxNoNonZero = 3, .colPositions = leftColPos, .values = leftVals};
    struct ELLPACK right = {.noRows = 3, .noCols = 1, .maxNoNonZero = 1, .colPositions = rightColPos, .values = rightVals};
    struct ELLPACK result = {.noRows = 0, .noCols = 0, .maxNoNonZero = 0, .colPositions = malloc(10*sizeof(uint64_t)), .values = malloc(10*sizeof(float ))};
    if (result.colPositions == NULL || result.values == NULL) {
        if (result.colPositions != NULL) {
            free(result.colPositions);
        }
        if (result.values != NULL) {
            free(result.values);
        }
        fprintf(stderr, "Error allocating memory");
        EXIT_FAILURE;
    } else {
        multiply(left, right, result);
        printf("%lu, %lu, %lu", result.noRows, result.noCols, result.maxNoNonZero);
        for (int i = 0; i < 2; i++) {
            printf("%f, %lu", result.values[i], result.colPositions[i]);
        }
        free(result.colPositions);
        free(result.values);
        EXIT_SUCCESS;
    }
}


    // Step 1: read matrices
    /* Step 2: make sure everything is valid
     *      Sample Format:
     *      LINE|   CONTENT
     *      1   |   <noRows>,<noCols>,<noNonZero>
     *      2   |   <values>
     *      3   |   <indices>
     *
     *      The input Files are in a right format: values comma seperated, only floats (no int, letters, etc.)
     *      The output File is large enough to store the result
     *      The matrices are have valid dimensions that are >= 1 and can be multiplied
     *      The matrices fulfill the conditions:
     *          noRows * noNonZeros = values.length     (amount of Floats in values)
     *          noRows * noNonZeros = indices.length     (amount of Floats in indices)
     *          at every index where values has padding (*), indices must have padding and vice versa
     *          every row starts with (noNonZero - x) non-paddings (valid floats), followed by x paddings
     *              no padding is followed by a value in the same row
    */
    // Step 3: the actual matrix multiplication




