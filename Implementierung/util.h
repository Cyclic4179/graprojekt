#ifndef UTIL_GUARD
#define UTIL_GUARD

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
//#define abortIfNEQ(i,j) __abort_null(i-j, "#i - \\#j")
//abortIfNEQ(i, j)


#ifdef DEBUG
#define pdebug(...) fprintf(stderr, "DEBUG:    " __VA_ARGS__)
#define pdebug_(...) fprintf(stderr, __VA_ARGS__)
//#define pdebug_var(...) fprintf(stderr, "DEBUG:    %s", #__VA_ARGS__)
#else
#define pdebug(...)
#define pdebug_(...)
#endif

#endif
