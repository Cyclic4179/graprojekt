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
    //char* ptr = "4,4,2\n5,*,6,*,0.5,7,3,*\n0,*,1,*,0,1,3,*\n";
    //int len = strlen(ptr);
    //float values[len/2];
    //uint64_t indices[len/2];
    //struct ELLPACK ppp;
    //ppp.values = values;
    //ppp.colPositions = indices;

    //ppp.noRows = strtoll(ptr, &ptr, 10);
    //ptr++;
    ////abortIfNEQ(*ptr, ",");
    //ppp.noCols = strtoll(ptr, &ptr, 10);
    //ptr++;
    //ppp.maxNoNonZero = strtoll(ptr, &ptr, 10);
    //ptr++;

    //uint64_t index = 0;
    //while (*ptr != '\n') {
    //    switch (*ptr) {
    //        case 0:
    //            exit(EXIT_FAILURE);
    //        case '*':
    //            if (*(++ptr) != ',') {
    //                printf("invalid char: %c\n", *ptr);
    //                exit(EXIT_FAILURE);
    //            }
    //            ptr++;
    //            break;
    //    }
    //    if (*ptr != '.' && *ptr < '0' || *ptr > '9') {
    //        printf("invalid char: %c\n", *ptr);
    //        exit(EXIT_FAILURE);
    //    }
    //    //if (*ptr == '\0') {
    //    //}
    //    values[index++] = strtof(ptr, &ptr);
    //    ptr++;
    //}

    //index = 0;
    //while (*ptr != '\n') {
    //    if (*ptr == '\0') {
    //        exit(EXIT_FAILURE);
    //    }
    //    indices[index++] = strtol(ptr, &ptr, 10);
    //    ptr++;
    //}

    //write(ppp, stdout);
    //exit(0);

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
                if (optarg) {
                    rounds = parse_int('B', pname);
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

    //abortIfNULL_msg(a, "(fixmelater) for now, '-a' must be set");
    //abortIfNULL_msg(b, "(fixmelater) for now, '-b' must be set");
    //abortIfNULL_msg(out, "(fixmelater) for now, '-out' must be set");

    // TODO things
    FILE* file_a = abortIfNULL(fopen(a, "r"));
    FILE* file_b = abortIfNULL(fopen(b, "r"));
    /*FILE* file_a = (fopen(a, "r")); // alternative from Simon for debugging
    FILE* file_b = (fopen(b, "r"));
    if (file_a == NULL || file_b == NULL) {
        abort();
    }*/
    const struct ELLPACK a_lpk = read_validate(file_a);
    const struct ELLPACK b_lpk = read_validate(file_b);
    fclose(file_a);
    fclose(file_b);
    //printf("%f\n", a_lpk.values[4]);
    //puts("start asdf");
    //write(a_lpk, stdout);
    //puts("fini asdf");
    //write(b_lpk, stdout);

    struct ELLPACK res_lpk;
    matr_mult_ellpack(&a_lpk, &b_lpk, &res_lpk);

    FILE* file_out = abortIfNULL(fopen(out, "w"));
    /*FILE* file_out = fopen(out, "w"); // alternative from Simon for debugging
    if (file_out == NULL) {
        abort();
    }*/
    write(res_lpk, file_out);
    fclose(file_out);

    free_ellpack(a_lpk);
    free_ellpack(b_lpk);
    free_ellpack(res_lpk);
    exit(EXIT_SUCCESS);
}
