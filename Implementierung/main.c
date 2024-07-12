#include <stdio.h>
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

    void (*matr_mult_ellpack_ptr)(const void*, const void*, void*);

    switch (args.impl_version) {
        case 0:
            matr_mult_ellpack_ptr = matr_mult_ellpack;
            break;
    }

    FILE *file_a, *file_b;

    if (args.a != NULL) {
        file_a = abortIfNULL(fopen(args.a, "r"));
    } else {
        file_a = stdin;
    }

    if (args.b != NULL) {
        file_b = abortIfNULL(fopen(args.b, "r"));
    } else {
        file_b = stdin;
    }

    const struct ELLPACK a_lpk = elpk_read_validate(file_a);
    const struct ELLPACK b_lpk = elpk_read_validate(file_b);

    if (args.a != NULL) fclose(file_a);
    if (args.b != NULL) fclose(file_b);

    /*FILE* file_a = (fopen(a, "r")); // alternative from Simon for debugging
    FILE* file_b = (fopen(b, "r"));
    if (file_a == NULL || file_b == NULL) {
        abort();
    }*/

    struct ELLPACK res_lpk;

    if (args.timeit) {
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
        matr_mult_ellpack_ptr(&a_lpk, &b_lpk, &res_lpk);

        FILE* file_out;
        if (args.out != NULL) {
            file_out = abortIfNULL(fopen(args.out, "w"));
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

    free_elpk(a_lpk);
    free_elpk(b_lpk);
    free_elpk(res_lpk);
    exit(EXIT_SUCCESS);
}
