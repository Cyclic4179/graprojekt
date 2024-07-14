#ifndef GUARD_PARSEARGS
#define GUARD_PARSEARGS

#include <stdbool.h>

// struct that stores validated and parsed argument info
struct ARGS {
    char* a;
    char* b;
    char* out;
    int impl_version;
    bool timeit;
    int iterations;
    bool check_equal;
};


struct ARGS parse_args(int argc, char** argv);

#endif
