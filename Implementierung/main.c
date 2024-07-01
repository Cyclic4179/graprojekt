#include <stdbool.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "bits/time.h"
#include "mult.h"
#include "file_io.h"
#include "ellpack.h"
#include "time.h"
#include "util.h"


const char* usage_msg =
    "Usage: %s [options]\n";

const char* help_msg =
    // TODO include valid impl_numbers in help_msg
    //"\nRequired arguments:\n"
    "\n"
    "Optional arguments:\n"
    "    -a PATH\n"
    "    -b PATH     paths to ellpack matrix factor (if omitted: stdin, '\\n' separated)\n"
    "    -o PATH     path to result (if omitted: stdout)\n"
    "    -V N        impl number (non-negative integer, default: 0)\n"
    "    -B N        time execution, N (positive) iterations (default: don't time, if set, no result will be printed to file, if N omitted: 1 iteration)\n"
    "    -h, --help  Show help and exit\n"
    "\n"
    "Examples:\n"
    "%s -o result -a sample-inputs/2.txt <sample-inputs/2.txt\n"
    "%s - <sample-inputs/1.txt <sample-inputs/2.txt\n"
    "%s -V 0 -B <sample-inputs/1.txt <sample-inputs/2.txt\n"
    "%s -V 0 -B 9 -a sample-inputs/1.txt -b sample-inputs/2.txt\n";

void print_usage(const char* pname) {
    fprintf(stderr, usage_msg, pname, pname, pname);
}

void print_help(const char* pname) {
    print_usage(pname);
    fprintf(stderr, help_msg, pname, pname, pname, pname);
}

int parse_int(char opt, const char* pname) {
    char* ptr = 0;
    uint64_t res = strtol(optarg, &ptr, 10);

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
    char *a = NULL,
         *b = NULL,
         *out = NULL;
    int impl_version = 0;
    bool timeit = false;
    int iterations = 1;

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
                if (optarg != NULL) {
                    iterations = parse_int('B', pname);
                    if (iterations < 0) {
                        fprintf(stderr, "invalid number of iterations: %d\n", iterations);
                        print_help(pname);
                        exit(EXIT_SUCCESS);
                    }
                }
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

    void (*matr_mult_ellpack_ptr)(const void*, const void*, void*);

    switch (impl_version) {
        case 0:
            matr_mult_ellpack_ptr = matr_mult_ellpack;
            break;
        default:
            fprintf(stderr, "not a valid version: %d\n", impl_version);
            print_usage(pname);
            exit(EXIT_FAILURE);
    }

    FILE *file_a, *file_b;

    if (a != NULL) {
        file_a = abortIfNULL(fopen(a, "r"));
    } else {
        file_a = stdin;
    }

    if (b != NULL) {
        file_b = abortIfNULL(fopen(b, "r"));
    } else {
        file_b = stdin;
    }

    const struct ELLPACK a_lpk = elpk_read_validate(file_a);
    const struct ELLPACK b_lpk = elpk_read_validate(file_b);

    if (a != NULL) fclose(file_a);
    if (b != NULL) fclose(file_b);

    /*FILE* file_a = (fopen(a, "r")); // alternative from Simon for debugging
    FILE* file_b = (fopen(b, "r"));
    if (file_a == NULL || file_b == NULL) {
        abort();
    }*/

    struct ELLPACK res_lpk;

    if (timeit) {
        struct timespec start, end;
        double elapsed_time;

        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int i = 0; i < iterations; i++) {
            matr_mult_ellpack_ptr(&a_lpk, &b_lpk, &res_lpk);
            sleep(1);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        elapsed_time = (end.tv_sec - start.tv_sec - iterations) * 1.0e9 +
            (end.tv_nsec - start.tv_nsec);
        printf("Average elapsed time per iteration: %f seconds\n", elapsed_time / iterations / 1.0e9);

    } else {
        matr_mult_ellpack_ptr(&a_lpk, &b_lpk, &res_lpk);

        FILE* file_out;
        if (out != NULL) {
            file_out = abortIfNULL(fopen(out, "w"));
        } else {
            file_out = stdout;
        }

        /*FILE* file_out = fopen(out, "w"); // alternative from Simon for debugging
          if (file_out == NULL) {
          abort();
          }*/
        elpk_write(res_lpk, file_out);
        fclose(file_out);
    }

    free_ellpack(a_lpk);
    free_ellpack(b_lpk);
    free_ellpack(res_lpk);
    exit(EXIT_SUCCESS);
}
