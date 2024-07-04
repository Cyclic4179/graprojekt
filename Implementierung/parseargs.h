#ifndef PARSEARGS_GUARD
#define PARSEARGS_GUARD

#include <stdbool.h>

// struct that stores validated and parsed argument info
struct ARGS {
    char* a;
    char* b;
    char* out;
    int impl_version;
    bool timeit;
    int iterations;
};


struct ARGS parse_args(int argc, char** argv);

#endif
