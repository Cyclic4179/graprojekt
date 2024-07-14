#include <math.h>
#include <stdint.h>
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
    pdebug("\tcheck_equal: '%d'\n", args.check_equal);

    void (*matr_mult_ellpack_ptr)(const void*, const void*, void*);

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
        default:
            abortIfNULL_msg(0, "fixme");
    }

    FILE *file_a, *file_b;

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
    elpk_write(a_lpk, stderr);
#endif

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
    elpk_write(b_lpk, stderr);
#endif

    if (args.a != NULL) fclose(file_a);
    if (args.b != NULL) fclose(file_b);

    if (args.check_equal) {
        pdebug("checking if equal\n");
        if (a_lpk.noCols != b_lpk.noCols ||
                a_lpk.noRows != b_lpk.noRows ||
                a_lpk.maxNoNonZero != b_lpk.maxNoNonZero) {
            puts("dimensions dont match");
            exit(EXIT_FAILURE);
        }
        for (uint64_t i = 0; i < a_lpk.noRows * a_lpk.maxNoNonZero; i++) {
            float absdiff = fabsf(a_lpk.values[i] - b_lpk.values[i]);

            if (absdiff >= ELLPACK_ROUGHLY_EQ_DIFF || a_lpk.indices[i] != b_lpk.indices[i]) {
                printf("values or index at entry '%lu' dont match: (%.50f; %lu) vs (%.50f; %lu) \n",
                        i, a_lpk.values[i], a_lpk.indices[i], b_lpk.values[i], b_lpk.indices[i]);
                exit(EXIT_FAILURE);
            }
        }
        puts("equal");
        exit(EXIT_SUCCESS);
    }

    struct ELLPACK res_lpk;

    if (args.timeit) {
#ifdef DEBUG
        fputs("WARNING:  compiled with debug output\n", stdout);
#endif
        struct timespec start, end;
        double elapsed_time;

        clock_gettime(CLOCK_MONOTONIC, &start);

        for (int i = 0; i < args.iterations; i++) {
            matr_mult_ellpack_ptr(&a_lpk, &b_lpk, &res_lpk);
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
        fclose(file_out);
    }

    free_elpk(a_lpk);
    free_elpk(b_lpk);
    free_elpk(res_lpk);
    exit(EXIT_SUCCESS);
}
