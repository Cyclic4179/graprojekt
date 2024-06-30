#include <stdbool.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "mult.h"
#include "file_io.h"
#include "ellpack.h"
#include "util.h"


const char* usage_msg =
    "Usage: %s [options]\n";

const char* help_msg =
    //"\nRequired arguments:\n"
    "\n"
    "Optional arguments:\n"
    "    -a PATH\n"
    "    -b PATH     paths to ellpack matrix factor (if omitted: stdin, '\\n' separated)\n"
    "    -o PATH     path to result (if omitted: stdout)\n"
    "    -V N        impl number (non-negative integer, default: 0)\n"
    "    -B N        time execution, N iterations (default: don't time, if N omitted: 1 iteration)\n"
    "    -h, --help  Show help and exit\n";

void print_usage(const char* pname) {
    fprintf(stderr, usage_msg, pname, pname, pname);
}

void print_help(const char* pname) {
    print_usage(pname);
    fprintf(stderr, "\n%s", help_msg);
}

uint64_t parse_int(char opt, const char* pname) {
    char* ptr = 0;
    uint64_t res = strtoll(optarg, &ptr, 10);
    if (*ptr != '\0' || errno == ERANGE) {
        fprintf(stderr, "not a valid integer for option -%c: '%s'", opt, optarg);
        if (errno == ERANGE) {
            fprintf(stderr, " (%s)\n", strerror(errno));
        } else {
            fputs("\n", stderr);
        }
        print_help(pname);
        exit(EXIT_SUCCESS);
    }
    return res;
}

int main(int argc, char** argv) {
    const char* pname = argv[0];

    if (argc == 1) {
        print_usage(pname);
        return EXIT_FAILURE;
    }

    int opt;
    char *a,
         *b,
         *out;
    int impl_version = 0;
    bool timeit = false;
    int rounds = 1;

    static struct option long_opts[] = {
        {"help",    no_argument,    NULL,   'h'},
        {0, 0, 0, 0} // required (man 3 getopt_long)
    };

    while ((opt = getopt_long(argc, argv, "V:B::a:b:o:h", long_opts, NULL)) != -1) {
        switch (opt) {
            case 'V':
                impl_version = parse_int('B', pname);
                break;
            case 'B':
                timeit = true;
                rounds = parse_int('B', pname);
                break;
            case 'a':
                a = optarg;
                break;
            case 'b':
                b = optarg;
                break;
            case 'o':
                out = optarg;
                break;
            case 'h':
                print_help(pname);
                exit(EXIT_SUCCESS);
            default: /* '?' */
                print_usage(pname);
                exit(EXIT_FAILURE);
        }
    }

    abortIfNULL_msg(a, "(fixmelater) for now, a must be set");
    abortIfNULL_msg(b, "(fixmelater) for now, b must be set");
    abortIfNULL_msg(out, "(fixmelater) for now, out must be set");

    // TODO things
    FILE* file_a = abortIfNULL(fopen(a, "r"));
    FILE* file_b = abortIfNULL(fopen(b, "r"));
    const struct ELLPACK a_lpk = read_validate(file_a);
    const struct ELLPACK b_lpk = read_validate(file_b);
    fclose(file_a);
    fclose(file_b);

    struct ELLPACK res_lpk;
    matr_mult_ellpack(&a_lpk, &b_lpk, &res_lpk);

    FILE* file_out = abortIfNULL(fopen(out, "w"));
    write(res_lpk, file_out);
    fclose(file_out);

    exit(EXIT_SUCCESS);
}

