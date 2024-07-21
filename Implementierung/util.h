#ifndef GUARD_UTIL
#define GUARD_UTIL

#include <stdlib.h>

/// @brief print error msg and exit with EXIT_FAILURE, used by __abort_null for smaller
/// program size (and better program cache as not whole error printing is inlined)
__attribute__((noreturn)) void __abort(const char* desc, const char* func, const char* file, int line, const char* msg);

/// @brief print pretty error message and abort if pointer is NULL
__attribute__((always_inline)) inline void* __abort_null(void* value, const char* desc, const char* func,
                                                         const char* file, int line, const char* msg) {
    if (!value) {
        __abort(desc, func, file, line, msg);
    }
    return value;
}

#define abortIfNULL(i) __abort_null(i, #i, __func__, __FILE__, __LINE__, "")
#define abortIfNULL_msg(i, j) __abort_null(i, #i, __func__, __FILE__, __LINE__, j)

#define DEBUG_OUTPUT_MAX_SIZE 200

#ifdef DEBUG
#define pdebug(...) fprintf(stderr, "DEBUG:    " __VA_ARGS__)
#define pdebug_(...) fprintf(stderr, __VA_ARGS__)
#else
#define pdebug(...)
#define pdebug_(...)
#endif

/// @brief write into string from float without trailing zeros and without scientific notation
/// should do the same as snprintf(s, "%g", f) for small values
/// @param n max-size of string
/// @param s writable string
/// @param f float value to convert
void ftostr(size_t n, char s[n], float f);

/// @brief function that prints error msg and exits
__attribute__((noreturn)) void __abort(const char* desc, const char* func, const char* file, int line, const char* msg);

#endif
