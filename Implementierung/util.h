#ifndef GUARD_UTIL
#define GUARD_UTIL

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/// @brief print pretty error message and abort if pointer is NULL
__attribute__((always_inline)) inline void* __abort_null(void* value, const char* desc, const char* func, const char* file, int line, const char* msg) {
    if (!value) {
        fputs("ERROR:    ", stderr);
        if (*msg != 0) {
            fprintf(stderr, "%s: ", msg);
        }
        fprintf(stderr, "'%s' was NULL", desc);
        if (errno != 0) {
            fprintf(stderr, " (error: %s)", strerror(errno));
        }
        fprintf(stderr, " in %s at %s, line %d\n", func, file, line);
        abort();
    }
    return value;
}


#define abortIfNULL(i) __abort_null(i, #i, __func__, __FILE__, __LINE__, "")
#define abortIfNULL_msg(i, j) __abort_null(i, #i, __func__, __FILE__, __LINE__, j)


#ifdef DEBUG
#   define pdebug(...) fprintf(stderr, "DEBUG:    " __VA_ARGS__)
#   define pdebug_(...) fprintf(stderr, __VA_ARGS__)
#else
#   define pdebug(...)
#   define pdebug_(...)
#endif

/// @brief write into string from float without trailing zeros and without scientific notation
/// should do the same as snprintf(s, "%g", f) for small values
/// @param n max-size of string
/// @param s writable string
/// @param f float value to convert
void ftostr(size_t n, char s[n], float f);

#endif
