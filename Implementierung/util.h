/// if you guys want to move this elsewhere i dont mind, just didnt know where to put

#ifndef UTIL_GUARD
#define UTIL_GUARD

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

inline void* __abort_null(void* value, char* desc, char* file, int line, char* msg) {
    if (!value) {
        fputs("ERROR:    ", stderr);
        if (*msg != 0) {
            fprintf(stderr, "%s: ", msg);
        }
        fprintf(stderr, "'%s' was NULL", desc);
        if (errno != 0) {
            fprintf(stderr, " (error: %s)", strerror(errno));
        }
        fprintf(stderr, " at %s, line %d\n", file, line);
        abort();
    }
    return value;
}

#define abortIfNULL(i) __abort_null(i, #i, __FILE__, __LINE__, "")
#define abortIfNULL_msg(i, j) __abort_null(i, #i, __FILE__, __LINE__, j)
//#define abortIfNEQ(i,j) __abort_null(i-j, "#i - \\#j")
//abortIfNEQ(i, j)

#endif
