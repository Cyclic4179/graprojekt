#include "parseargs.h"

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mult.h"
#include "time.h"

void print_usage(const char* pname) {
    const char* usage_msg = "Usage: %s [options] [-]\n";

    fprintf(stderr, usage_msg, pname, pname, pname);
}

void print_help(const char* pname) {
    // clang-format off
    const char* help_msg =
        "\n"
        "Optional arguments:\n"
        "    -a PATH\n"
        "    -b PATH     paths to ellpack matrix factors (if omitted: stdin, '\\n' separated)\n"
        "    -o PATH     path to result (if omitted: stdout)\n"
        "    -V N        impl number (integer between 0 and %d, default: %d)\n"
        "    -B\n"
        "    -BN         time execution, N (positive) iterations (default: don't time; if set, no result will be printed to file; if N omitted: %d iterations)\n"
        "    -e\n"
        "    -eF         parse files and print true if they are roughly equal (diff of entries < %d or F (float))\n"
        "    -x          print max impl version to stdout and exit\n"
        "    -h, --help  Show help and exit\n"
        "\n"
        "Examples:\n"
        "    %s -o result -a sample-inputs/2.txt <sample-inputs/2.txt\n"
        "    %s - <sample-inputs/1.txt <sample-inputs/2.txt\n"
        "    %s -V 0 -B <sample-inputs/1.txt <sample-inputs/2.txt\n"
        "    %s -B9 -a sample-inputs/1.txt -b sample-inputs/2.txt\n";
    // clang-format on

    print_usage(pname);
    fprintf(stderr, help_msg, MAX_IMPL_VERSION, DEFAULT_IMPL_VERSION, DEFAULT_ITERATIONS, DEFAULT_EQ_MAX_DIFF, pname,
            pname, pname, pname);
}

float parse_float(char opt, const char* pname) {
    char* ptr = 0;
    float res = strtof(optarg, &ptr);

    if (*ptr != '\0' || errno == ERANGE) {
        fprintf(stderr, "not a valid float for option -%c: '%s'", opt, optarg);
        if (errno == ERANGE) {
            fprintf(stderr, " (%s)\n", strerror(errno));
        } else {
            fputs("\n", stderr);
        }
        print_help(pname);
        exit(EXIT_FAILURE);
    }

    return res;
}

int parse_int(char opt, const char* pname) {
    char* ptr = 0;
    int res = strtol(optarg, &ptr, 10);

    if (*ptr != '\0' || errno == ERANGE) {
        fprintf(stderr, "not a valid integer for option -%c: '%s'", opt, optarg);
        if (errno == ERANGE) {
            fprintf(stderr, " (%s)\n", strerror(errno));
        } else {
            fputs("\n", stderr);
        }
        print_help(pname);
        exit(EXIT_FAILURE);
    }

    return res;
}

struct ARGS parse_args(int argc, char** argv) {
    const char* pname = argv[0];

    if (argc == 1) {
        print_usage(pname);
        exit(EXIT_FAILURE);
    }

    int opt;
    struct ARGS parsed_args = {.a = NULL,
                               .b = NULL,
                               .out = NULL,
                               .impl_version = 0,
                               .action = MULT,
                               .iterations = 3,
                               .eq_max_diff = DEFAULT_EQ_MAX_DIFF};

    static struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'}, {0, 0, 0, 0}  // required (man 3 getopt_long)
    };

    while ((opt = getopt_long(argc, argv, "V:B::a:b:o:he::x", long_opts, NULL)) != -1) {
        switch (opt) {
            case 'V':
                parsed_args.impl_version = parse_int('V', pname);
                if (parsed_args.impl_version < 0 || parsed_args.impl_version > MAX_IMPL_VERSION) {
                    fprintf(stderr, "invalid impl version: %d\n", parsed_args.impl_version);
                    print_usage(pname);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'B':
                parsed_args.action = BENCH;
                // parsed_args.timeit = true;
                if (optarg) {
                    parsed_args.iterations = parse_int('B', pname);
                    if (parsed_args.iterations < 0) {
                        fprintf(stderr, "invalid number of iterations: %d\n", parsed_args.iterations);
                        print_usage(pname);
                        exit(EXIT_FAILURE);
                    }
                }
                break;
            case 'a':
                parsed_args.a = optarg;
                break;
            case 'b':
                parsed_args.b = optarg;
                break;
            case 'o':
                parsed_args.out = optarg;
                break;
            case 'e':
                parsed_args.action = CHECK_EQ;
                if (optarg) {
                    parsed_args.eq_max_diff = parse_float('e', pname);
                    if (parsed_args.eq_max_diff < 0) {
                        fprintf(stderr, "invalid max diff: %d\n", parsed_args.iterations);
                        print_usage(pname);
                        exit(EXIT_FAILURE);
                    }
                }
                break;
            case 'x':
                printf("%d\n", MAX_IMPL_VERSION);
                exit(EXIT_SUCCESS);
            case 'h':
                print_help(pname);
                exit(EXIT_SUCCESS);
            default: /* '?' */
                print_usage(pname);
                exit(EXIT_FAILURE);
        }
    }

    return parsed_args;
}
