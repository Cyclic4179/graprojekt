#include "file_io.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "ellpack.h"
#include "util.h"

/// @brief helper: read int from string
/// @param string string
/// @param pos current position in string
/// @param end character after int
/// @param field_for_error part of error message
/// @return read int
uint64_t helper_read_int(const char* string, long* pos, char end, char* field_for_error, int line) {
    if (line != 1 && string[(*pos)] == '*') {
        if (string[(*pos) + 1] != end) {
            fprintf(stderr, "<%s> contains illegal character after *: '%c'\n", field_for_error, string[(*pos + 1)]);
            exit(EXIT_FAILURE);
        }
        (*pos)++;
        return 0;
    } else {
        uint64_t res = 0;
        while (string[(*pos)] != end) {
            if (0 <= (string[(*pos)] - '0') && (string[(*pos)] - '0') <= 9) {
                res = (res * 10) + (string[(*pos)] - '0');
                (*pos)++;
            } else {
                fprintf(stderr, "<%s> contains illegal character: '%c'\n", field_for_error, string[(*pos)]);
                exit(EXIT_FAILURE);
            }
        }
        return res;
    }
}

/// @brief helper: read float from string
/// @param string string
/// @param pos current position in string
/// @param end character after float
/// @param field_for_error part of error message
/// @return read float
float helper_read_float(const char* string, long* pos, char end, char* field_for_error) {
    if (string[(*pos)] == '*') {
        if (string[(*pos) + 1] != end) {
            fprintf(stderr, "<%s> contains illegal character after *: '%c'\n", field_for_error, string[(*pos + 1)]);
            exit(EXIT_FAILURE);
        }
        (*pos)++;
        return 0.0f;
    } else {
        long start = *pos;
        if (string[(*pos)] == '-') {
            (*pos)++;
        }
        bool decimal_point = false;
        while (string[(*pos)] != end) {
            if (0 <= (string[(*pos)] - '0') && (string[(*pos)] - '0') <= 9) {
                (*pos)++;
            } else if (string[(*pos)] == '.') {
                if (decimal_point) {
                    fprintf(stderr, "<%s> contains illegal second decimal point\n", field_for_error);
                    exit(EXIT_FAILURE);
                }
                decimal_point = true;
                (*pos)++;
            } else {
                fprintf(stderr, "<%s> contains illegal character: '%c'\n", field_for_error, string[(*pos)]);
                exit(EXIT_FAILURE);
            }
        }
        return strtof(&string[start], NULL);
    }
}

/// @brief check for no indices larger than matrix dimensions and only ascending indices
/// @param matrix matrix to check
void validate_matrix(const struct ELLPACK matrix) {
    uint64_t index = 0;
    int padding;  // works as boolean to check if padded value was found
    for (uint64_t i = 0; i < matrix.noRows; i++) {
        padding = 0;
        for (uint64_t j = 0; j < matrix.maxNoNonZero; j++) {
            uint64_t accessIndex = i * matrix.maxNoNonZero + j;
            if (matrix.indices[accessIndex] >= matrix.noCols) {
                fprintf(stderr, "ERROR: Index %lu too large for a %lux%lu matrix.\n", matrix.indices[accessIndex],
                        matrix.noRows, matrix.noCols);
                exit(EXIT_FAILURE);
            }
            if (padding == 0 && matrix.indices[accessIndex] == 0 && matrix.values[accessIndex] == 0.f) {
                padding = 1;
            }
            if (matrix.indices[accessIndex] <= index && j != 0 && padding == 0) {
                fprintf(stderr,
                        "ERROR: Indices not in ascending order in row %lu at index %lu: index %lu not greater than "
                        "previous index %lu.\n",
                        i, j, matrix.indices[accessIndex], matrix.indices[accessIndex - 1]);
                exit(EXIT_FAILURE);
            }
            index = matrix.indices[accessIndex];
        }
    }
}

/// @brief reads and validates a matrix
/// @param file pointer to the file
/// @result matrix in ELLPACK format
struct ELLPACK elpk_read_validate(FILE* file) {
    char* string = NULL;
    size_t len = 0;
    struct ELLPACK result;
    long pos = 0;

    abortIfNULL_msg((void*)(getline(&string, &len, file) + 1), "could not read from file");

    result.noRows = helper_read_int(string, &pos, ',', "noRows", 1);
    pos++;

    result.noCols = helper_read_int(string, &pos, ',', "noCols", 1);
    pos++;

    result.maxNoNonZero = helper_read_int(string, &pos, '\n', "maxNoNonZero", 1);

    abortIfNULL_msg((void*)(getline(&string, &len, file) + 1), "could not read from file");
    pos = 0;

    long itemsCount = result.noRows * result.maxNoNonZero;

    result.values = (float*)abortIfNULL_msg(malloc(itemsCount * sizeof(float)), "could not allocate memory");
    for (long i = 0; i < itemsCount; i++) {
        char end = i == itemsCount - 1 ? '\n' : ',';
        result.values[i] = helper_read_float(string, &pos, end, "values");
        pos++;
    }

    abortIfNULL_msg((void*)(getline(&string, &len, file) + 1), "could not read from file");
    pos = 0;

    result.indices = (uint64_t*)abortIfNULL_msg(malloc(itemsCount * sizeof(uint64_t)), "could not allocate memory");
    for (long i = 0; i < itemsCount; i++) {
        char end = i == itemsCount - 1 ? '\n' : ',';
        result.indices[i] = helper_read_int(string, &pos, end, "indices", 3);
        pos++;
    }

    free(string);

    validate_matrix(result);

    return result;
}

/// @brief writes the matrix to the file
/// @param matrix matrix to convert
/// @param result pointer to file
void elpk_write(struct ELLPACK matrix, FILE* file) {
    uint64_t index, i, j;
    float val, absval;

    // first line info
    fprintf(file, "%lu,%lu,%lu\n", matrix.noRows, matrix.noCols, matrix.maxNoNonZero);

    index = 0;
    bool first = true;  // just a helper bool to not print ',' at start; not that nice ik but whatever
    char s[256];        // buffer for storing a single float in nice format

    // print values
    for (i = 0; i < matrix.noRows; i++) {
        for (j = 0; j < matrix.maxNoNonZero; j++) {
            val = matrix.values[index];
            absval = fabsf(val);

            if (absval < 0.000001) {
#ifdef DEBUG
                if (val != 0) {
                    fputs("\n", stderr);
                    pdebug(
                        "elpk_write: detected value not eq 0 but smaller"
                        " than 0.000001 at index %lu: %.*f\n",
                        index, 50, matrix.values[index]);
                }
#endif

                if (first) {
                    fputs("*", file);
                    first = false;
                } else {
                    fputs(",*", file);
                }

                index++;

            } else {
                ftostr(sizeof(s), s, matrix.values[index++]);

                if (first) {
                    fprintf(file, "%s", s);
                    first = false;
                } else {
                    fprintf(file, ",%s", s);
                }
            }
        }
    }

    fputs("\n", file);

    // reset
    index = 0;
    first = true;

    // print indices
    for (i = 0; i < matrix.noRows; i++) {
        for (j = 0; j < matrix.maxNoNonZero; j++) {
            val = matrix.values[index];
            absval = val > 0 ? val : -val;

            if (absval < 0.000001) {
                if (first) {
                    fputs("*", file);
                    first = false;
                } else {
                    fputs(",*", file);
                }

            } else if (first) {
                fprintf(file, "%lu", matrix.indices[index]);
                first = false;

            } else {
                fprintf(file, ",%lu", matrix.indices[index]);
            }

            index++;
        }
    }

    fputs("\n", file);

    fseek(file, 0, SEEK_SET);
}
