#include "file_io.h"
#include "util.h"
#include "ellpack.h"
#include <stdio.h>

/// @brief helper: read int from string
/// @param string string
/// @param pos current position in string
/// @param end character after int
/// @param field_for_error part of error message
/// @return read int
uint64_t helper_read_int(const char* string, long* pos, char end, char* field_for_error, int line) {
    uint64_t res = 0;
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

/// @brief helper: read float from string
/// @param string string
/// @param pos current position in string
/// @param end character after float
/// @param field_for_error part of error message
/// @return read float
float helper_read_float(const char* string, long* pos, char end, char* field_for_error) {
    float res = 0.0f;
    if (string[(*pos)] == '*')
    {
        if (string[(*pos) + 1] != end)
        {
            fprintf(stderr, "<%s> contains illegal character after *: %c\n", field_for_error, string[(*pos + 1)]);
            exit(EXIT_FAILURE);
        }
        (*pos)++;
    }
    else {
        float negative = 1.0f;
        if (string[(*pos)] == '-') {
            negative = -1.0f;
            (*pos)++;
        }
        int decimal_place = 0;
        while (string[(*pos)] != end)
        {
            if (0 <= (string[(*pos)] - '0') && (string[(*pos)] - '0') <= 9)
            {
                if (0 == decimal_place)
                {
                    res = (res * 10) + (string[(*pos)] - '0');
                }
                else
                {
                    int decimal_place_tmp = decimal_place;
                    float ten = 10.0f;
                    while (decimal_place_tmp > 1) {
                        ten *= 10;
                        decimal_place_tmp--;
                    }
                    res += (string[(*pos)] - '0') / ten;
                    decimal_place++;
                }
                (*pos)++;
            }
            else if (string[(*pos)] == '.') {
                if (0 < decimal_place)
                {
                    fprintf(stderr, "<%s> contains illegal second decimal point\n", field_for_error);
                    exit(EXIT_FAILURE);
                }
                decimal_place = 1;
                (*pos)++;
            }
            else
            {
                fprintf(stderr, "<%s> contains illegal character: %c\n", field_for_error, string[(*pos)]);
                exit(EXIT_FAILURE);
            }
        }
        res = res * negative;
    }
    return res;
}

/// @brief reads and validates a matrix
/// @param file pointer to the file
/// @result matrix in ELLPACK format
struct ELLPACK read_validate(FILE* file) {
    // TODO change file to FILE*
    // TODO better version later
    char string[1024];
    struct ELLPACK result = {};
    long pos = 0;

    int last = fread(string, sizeof(*string), sizeof(string), file);
    string[last] = 0;
    //printf("%s", string);

    result.noRows = helper_read_int(string, &pos, ',', "noRows", 1);
    pos++;

    result.noCols = helper_read_int(string, &pos, ',', "noCols", 1);
    pos++;

    result.maxNoNonZero = helper_read_int(string, &pos, '\n', "maxNoNonZero", 1);
    pos++;

    long itemsCount = result.noRows * result.maxNoNonZero;

    result.values = (float*)abortIfNULL(malloc(itemsCount * sizeof(float)));
    for (long i = 0; i < itemsCount; i++)
    {
        char end = i == itemsCount - 1 ? '\n' : ',';
        result.values[i] = helper_read_float(string, &pos, end, "values");
        pos++;
    }

    result.indices = (uint64_t*)abortIfNULL(malloc(itemsCount * sizeof(uint64_t)));
    for (long i = 0; i < itemsCount; i++)
    {
        char end = i == itemsCount - 1 ? '\n' : ',';
        result.indices[i] = helper_read_int(string, &pos, end, "indices", 3);
        pos++;
    }

    return result;
}

/// @brief writes the matrix to the file
/// @param matrix matrix to convert
/// @param result pointer to file
void write(struct ELLPACK matrix, FILE* file) {
    uint64_t index, i, j;

    fprintf(file, "%lu,%lu,%lu\n", matrix.noRows, matrix.noCols, matrix.maxNoNonZero);

    index = 0;
    for (i = 0; i < matrix.noRows; i++) {
        for (j = 0; j < matrix.maxNoNonZero; j++) {
            if (matrix.values[index] == 0.) {
                fputs("*,", file);
                index++;
                break;
            }
            fprintf(file, "%g,", matrix.values[index++]);
        }
    }
    if (file == stdout) {
        fputs("\33[D \n", file);
    } else {
        fseek(file, -1, SEEK_CUR);
        fputs("\n", file);
    }

    index = 0;
    for (i = 0; i < matrix.noRows; i++) {
        for (j = 0; j < matrix.maxNoNonZero; j++) {
            if (matrix.values[index] == 0.) {
                fputs("*,", file);
                index++;
                break;
            }
            fprintf(file, "%lu,", matrix.indices[index++]);
        }
    }
    if (file == stdout) {
        fputs("\33[D \n", file);
    } else {
        fseek(file, -1, SEEK_CUR);
        fputs("\n", file);
    }
}

// /// @brief helper: read a file to a pointer
// /// @param filePath path to file
// /// @param fileSize size of file in Bytes
// /// @param string pointer for result
// void readFile(char* filePath, long fileSize, char* string) {
//     FILE* file = abortIfNULL(fopen(filePath, "r"));
//     fread(string, fileSize, 1, file);
//     fclose(file);
//     string[fileSize - 1] = 0;
// }

// /// @brief tests the function `matr_mult_ellpack`
// /// @param nr test in `./sample-inputs/` to execute
// void test(int nr) {
//     char* filePathLeft;
//     long fileSizeLeft;
//     char* filePathRight;
//     long fileSizeRight;

//     switch (nr)
//     {
//     case 1:
//         filePathLeft = "./Implementierung/sample-inputs/1.txt";
//         fileSizeLeft = 40;
//         filePathRight = "./Implementierung/sample-inputs/1.txt";
//         fileSizeRight = 40;
//         break;

//     default:
//         return;
//     }

//     char* leftString = abortIfNULL(malloc(fileSizeLeft));
//     readFile(filePathLeft, fileSizeLeft, leftString);
//     char* rightString = abortIfNULL(malloc(fileSizeRight));
//     readFile(filePathRight, fileSizeRight, rightString);

//     char* result = abortIfNULL(malloc(fileSizeLeft + fileSizeRight)); // TODO real values

//     matr_mult_ellpack((void*)leftString, (void*)rightString, (void*)result);

//     free(leftString);
//     free(rightString);
//     free(result);
// }
