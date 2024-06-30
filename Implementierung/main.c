#include "main.h"
#include "util.h"


/// @brief main function
/// @param a matrix a
/// @param b matrix b
/// @param result result of multiplication
void matr_mult_ellpack(const void* a, const void* b, void* result) {
    // TODO
    struct ELLPACK aE = read_validate(a);
    struct ELLPACK bE = read_validate(b);
    struct ELLPACK res = multiply(aE, bE);
    write(res, result);

    // ! DEBUG
    printf("%lu\n", aE.noRows);
    printf("%lu\n", aE.noCols);
    printf("%lu\n", aE.maxNoNonZero);
    printf("%f\n", aE.values[0]);
    printf("%f\n", aE.values[1]);
    printf("%f\n", aE.values[2]);
    printf("%lu\n", aE.colPositions[0]);
    printf("%lu\n", aE.colPositions[1]);
    printf("%lu\n", aE.colPositions[2]);
}

/// @brief helper: read int from string
/// @param string string
/// @param pos current position in string
/// @param end character after int
/// @param field_for_error part of error message
/// @return read int
int helper_read_int(const char* string, long* pos, char end, char* field_for_error, int line) {
    int res = 0;
    if (line != 1 && string[(*pos)] == '*')
    {
        if (string[(*pos) + 1] != end)
        {
            fprintf(stderr, "<%s> contains illegal character after *: %c\n", field_for_error, string[(*pos + 1)]);
            exit(EXIT_FAILURE);
        }
        (*pos)++;
    }
    else {
        while (string[(*pos)] != end)
        {
            if (0 <= (string[(*pos)] - '0') && (string[(*pos)] - '0') <= 9)
            {
                res = (res * 10) + (string[(*pos)] - '0');
                (*pos)++;
            }
            else
            {
                fprintf(stderr, "<%s> contains illegal character: %c\n", field_for_error, string[(*pos)]);
                exit(EXIT_FAILURE);
            }
        }
    }
    return res;
}

/// @brief reads and validates a matrix
/// @param file pointer to the file
/// @result matrix in ELLPACK format
struct ELLPACK read_validate(const void* file) {
    // TODO
    const char* string = file;
    struct ELLPACK result = {};
    long pos = 0;

    result.noRows = helper_read_int(string, &pos, ',', "noRows", 1);
    pos++;

    result.noCols = helper_read_int(string, &pos, ',', "noCols", 1);
    pos++;

    result.maxNoNonZero = helper_read_int(string, &pos, '\n', "maxNoNonZero", 1);
    pos++;

    long itemsCount = result.noRows * result.maxNoNonZero;

    // ! NOT FOR FLOATS
    result.values = (float*)abortIfNULL(malloc(itemsCount * sizeof(float)));
    for (long i = 0; i < itemsCount; i++)
    {
        char end = i == itemsCount - 1 ? '\n' : ',';
        result.values[i] = helper_read_int(string, &pos, end, "values", 2);
        pos++;
    }

    result.colPositions = (uint64_t*)abortIfNULL(malloc(itemsCount * sizeof(uint64_t)));
    for (long i = 0; i < itemsCount; i++)
    {
        char end = i == itemsCount - 1 ? '\n' : ',';
        result.colPositions[i] = helper_read_int(string, &pos, end, "colPositions", 3);
        pos++;
    }

    return result;
}

/// @brief multiplies the matrices
/// @param left left matrix
/// @param right right matrix
/// @result multiplied matrix
struct ELLPACK multiply(struct ELLPACK left, struct ELLPACK right)
{
    // Array storing the amounts of non-zero entries per column in right    // TODO unused so far
    uint64_t* maxRowCounts = (uint64_t*) malloc(right.noRows + sizeof(uint64_t));
    if (maxRowCounts == NULL) {
        fprintf(stderr, "Error allocating memory");
        abort();
    }
    for (uint64_t i = 0; i < right.noRows; i++){
        maxRowCounts[i] = 0;
    }
    for (uint64_t i = 0; i < right.noRows * right.maxNoNonZero; i++) {
        if (right.values[i] != 0.f) {
            maxRowCounts[right.colPositions[i]]++;
        }
    }
    uint64_t maxRow = 0;                     // maximum number of non-zero entries of columns of right // TODO unused so far
    for (uint64_t i = 0; i < right.noRows; i++) {
        if (maxRowCounts[i] > maxRow) {
            maxRow = maxRowCounts[i];
        }
    }
    free(maxRowCounts);

    // ############################## Start used code ##############################

    struct ELLPACK result;
    result.noRows = left.noRows;
    result.noCols = right.noCols;

    uint64_t maxNoNonZeroLimit = (right.noCols > left.maxNoNonZero * right.maxNoNonZero) ? left.maxNoNonZero * right.maxNoNonZero : right.noCols;
    result.maxNoNonZero = maxNoNonZeroLimit;      // TODO proven that this is correct yet

    result.values = (float*) malloc(result.noRows * result.maxNoNonZero * sizeof(float));
    result.colPositions = (uint64_t*) malloc(result.noRows * result.maxNoNonZero * sizeof(uint64_t));
    if (result.values == NULL || result.colPositions == NULL) {
        if (result.values != NULL) {
            free(result.values);
        }
        if (result.colPositions != NULL) {
            free(result.colPositions);
        }
        abort();
    }

    uint64_t resultPos = 0;                  // pointer to next position to insert a value into result matrix
    uint64_t biggerDimension = (left.noRows > right.noCols) ? left.noRows : right.noCols;

    for (uint64_t i = 0; i < biggerDimension; i++) // Iterates over the rows of left
    {
        uint64_t leftRowIndex = i % left.noRows;

        for(uint64_t l = 0; l < result.noCols; l++) { // Iterates over the columns in the result matrix

            float sum = 0.f; // accumulator for an entry in result
            for (uint64_t j = 0; j < left.maxNoNonZero; j++) { // Iterates over a row of left

                // leftColRightRow is the column index of the left and row index of the right matrix
                uint64_t leftColRightRow = left.colPositions[leftRowIndex * left.maxNoNonZero + j];

                // add here: if padded (= invalid value (float of 0.0)) then break

                // Iterates over the column of right to check if value of right is not 0 / row of right has entry for that position
                for (uint64_t k = leftColRightRow * right.maxNoNonZero; k < (leftColRightRow + 1) * right.maxNoNonZero; k++) {
                    if (right.colPositions[k] == l) {
                        sum += left.values[leftRowIndex * left.maxNoNonZero + j] * right.values[k];
                    }
                }
            }
            // set value of result to calculated product
            if (sum != 0.0) {
                result.colPositions[resultPos] = l;
                result.values[resultPos++] = sum;
            }
        }
        // add padding
        for (; resultPos < (i + 1) * result.maxNoNonZero; resultPos++)
        {
            result.values[resultPos] = 0.f;
            result.colPositions[resultPos] = 0;
        }
    }

    uint64_t realRightMaxNoNonZero = 0;
    uint64_t rowCounter;
    for (uint64_t i = 0; i < result.noRows; i++) {
        rowCounter = 0;
        for (uint64_t j = 0; j < right.maxNoNonZero; ++j) {
            if (result.values[result.maxNoNonZero * i + j] != 0.f) {
                rowCounter++;
            }
        }
        if (rowCounter > realRightMaxNoNonZero) {
            realRightMaxNoNonZero = rowCounter;
        }
    }
    uint64_t realResultPointer = 0;
    for (uint64_t i = 0; i < result.noRows; i++) {
        for (uint64_t j = 0; j < result.maxNoNonZero; j++) {
            if (result.values[i * realRightMaxNoNonZero + j] != 0.f) {
                result.values[realResultPointer] = result.values[i * realRightMaxNoNonZero + j];
                result.colPositions[realResultPointer] = result.colPositions[i * realRightMaxNoNonZero + j];
                realResultPointer++;
            }
        }
        for (; realResultPointer < (i + 1) * realRightMaxNoNonZero; realResultPointer++)
        {
            result.values[realResultPointer] = 0.f;
            result.colPositions[realResultPointer] = 0;
        }
    }
    right.maxNoNonZero = realRightMaxNoNonZero;

    return result;
}

/// @brief writes the matrix to the file
/// @param matrix matrix to convert
/// @param result pointer to file
void write(struct ELLPACK matrix, void* result) {
    // TODO
}

/// @brief helper: read a file to a pointer
/// @param filePath path to file
/// @param fileSize size of file in Bytes
/// @param string pointer for result
void readFile(char* filePath, long fileSize, char* string) {
    FILE* file = abortIfNULL(fopen(filePath, "r"));
    fread(string, fileSize, 1, file);
    fclose(file);
    string[fileSize - 1] = 0;
}

/// @brief tests the function `matr_mult_ellpack`
/// @param nr test in `./sample-inputs/` to execute
void test(int nr) {
    char* filePathLeft;
    long fileSizeLeft;
    char* filePathRight;
    long fileSizeRight;

    switch (nr)
    {
    case 1:
        filePathLeft = "./Implementierung/sample-inputs/1.txt";
        fileSizeLeft = 40;
        filePathRight = "./Implementierung/sample-inputs/1.txt";
        fileSizeRight = 40;
        break;

    default:
        return;
    }

    char* leftString = abortIfNULL(malloc(fileSizeLeft));
    readFile(filePathLeft, fileSizeLeft, leftString);
    char* rightString = abortIfNULL(malloc(fileSizeRight));
    readFile(filePathRight, fileSizeRight, rightString);

    char* result = abortIfNULL(malloc(fileSizeLeft + fileSizeRight)); // TODO real values

    matr_mult_ellpack((void*)leftString, (void*)rightString, (void*)result);

    free(leftString);
    free(rightString);
    free(result);
}

int main()
{
    // TODO cli parameters
    test(1);
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
