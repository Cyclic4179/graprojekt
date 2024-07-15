#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "mult.h"
#include "file_io.h"
#include "ellpack.h"
#include "util.h"
#include "parseargs.h"


int main(int argc, char** argv) {
    struct ARGS args = parse_args(argc, argv);

    pdebug("parsed args:\n");
    pdebug("\ta: '%s'\n", args.a);
    pdebug("\tb: '%s'\n", args.b);
    pdebug("\tout: '%s'\n", args.out);
    pdebug("\timpl_version: '%d'\n", args.impl_version);
    pdebug("\ttimeit: '%s'\n", args.timeit ? "true" : "false");
    pdebug("\titerations: '%d'\n", args.iterations);
    pdebug("\tcheck_equal: '%s'\n", args.check_equal ? "true" : "false");
    pdebug("\tmax_diff: '%f'\n", args.eq_max_diff);

    void (*matr_mult_ellpack_ptr)(const void*, const void*, void*);

    // map impl_version to correct function
    switch (args.impl_version) {
        case 0:
            matr_mult_ellpack_ptr = matr_mult_ellpack;
            break;
        case 1:
            matr_mult_ellpack_ptr = matr_mult_ellpack_V1;
            break;
        case 2:
            matr_mult_ellpack_ptr = matr_mult_ellpack_V2;
            break;
        case 3:
            matr_mult_ellpack_ptr = matr_mult_ellpack_V3;
            break;
        case 4:
            matr_mult_ellpack_ptr = matr_mult_ellpack_V4;
            break;
        default:
            abortIfNULL_msg(0, "fixme: missing function for impl version");
    }

    FILE *file_a, *file_b;

    // read a
    if (args.a != NULL) {
        file_a = abortIfNULL(fopen(args.a, "r"));
        pdebug("reading file a: '%s'\n", args.a);
    } else {
        file_a = stdin;
        pdebug("reading a from stdin\n");
    }
    const struct ELLPACK a_lpk = elpk_read_validate(file_a);
    pdebug("done:\n");
#ifdef DEBUG
    if (a_lpk.noRows * a_lpk.maxNoNonZero > DEBUG_OUTPUT_MAX_SIZE) {
        fputs("...too large\n", stderr);
    } else {
        elpk_write(a_lpk, stderr);
    }
#endif

    // read b
    if (args.b != NULL) {
        file_b = abortIfNULL(fopen(args.b, "r"));
        pdebug("start reading file b: '%s'\n", args.b);
    } else {
        file_b = stdin;
        pdebug("reading b from stdin\n");
    }
    const struct ELLPACK b_lpk = elpk_read_validate(file_b);
    pdebug("done:\n");
#ifdef DEBUG
    if (b_lpk.noRows * b_lpk.maxNoNonZero > DEBUG_OUTPUT_MAX_SIZE) {
        fputs("...too large\n", stderr);
    } else {
        elpk_write(b_lpk, stderr);
    }
#endif

    if (args.a != NULL) fclose(file_a);
    if (args.b != NULL) fclose(file_b);

    if (args.check_equal) {
        pdebug("checking if equal\n");
        elpk_check_equal(a_lpk, b_lpk, args.eq_max_diff);
        abortIfNULL_msg(0, "this should never be reached");
    }

    struct ELLPACK res_lpk;

    if (args.timeit) {
#ifdef DEBUG
        fputs("WARNING:  compiled with debug output\n", stderr);
#endif
        struct timespec start, end;
        double elapsed_time;
        //struct ELLPACK* res_lpks = abortIfNULL(calloc(args.iterations, sizeof(struct ELLPACK)));

        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int i = 0; i < args.iterations; i++) {
            matr_mult_ellpack_ptr(&a_lpk, &b_lpk, &res_lpk);
            elpk_free(res_lpk);
            sleep(1);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        elapsed_time = (end.tv_sec - start.tv_sec - args.iterations) * 1.0e9 +
            (end.tv_nsec - start.tv_nsec);
        printf("Average elapsed time per iteration: %.6f seconds\n", elapsed_time / args.iterations / 1.0e9);

    } else {
        pdebug("starting multiplication\n");
        matr_mult_ellpack_ptr(&a_lpk, &b_lpk, &res_lpk);
        pdebug("finished multiplication\n");

        FILE* file_out;
        if (args.out != NULL) {
            file_out = abortIfNULL(fopen(args.out, "w"));
        } else {
            file_out = stdout;
        }

        pdebug("writing result\n");
        elpk_write(res_lpk, file_out);
        if (args.out != NULL) fclose(file_out);
        elpk_free(res_lpk);
    }

    elpk_free(a_lpk);
    elpk_free(b_lpk);
    exit(EXIT_SUCCESS);
}
