/// if you guys want to move this elsewhere i dont mind, just didnt know where to put

#ifndef UTIL_GUARD
#define UTIL_GUARD

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

inline void* __abort_null(void* value, char* desc) {
    if (!value) {
        fprintf(stderr, "ERROR:\t\t'%s' was NULL (error: %s)", desc, strerror(errno));
        abort();
    }
    return value;
}

#define abortIfNULL(i) __abort_null(i, #i)

#endif
