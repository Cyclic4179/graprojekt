#ifndef GUARD_PARSEARGS
#define GUARD_PARSEARGS

#include <stdbool.h>

// struct that stores validated and parsed argument info
struct ARGS {
    // pointer to files, if NULL -> stdin/out will be used
    char* a;
    char* b;
    char* out;

    int impl_version;

    // benchmarking options
    bool timeit;
    int iterations;

    // check if max pointwise difference of a, b < eq_max_diff
    bool check_equal;
    float eq_max_diff;
};

#define DEFAULT_EQ_MAX_DIFF 1

void print_usage(const char* pname);

void print_help(const char* pname);

float parse_float(char opt, const char* pname);

int parse_int(char opt, const char* pname);

struct ARGS parse_args(int argc, char** argv);

#endif
