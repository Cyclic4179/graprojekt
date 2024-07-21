#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "ellpack.h"
#include "file_io.h"
#include "mult.h"
#include "parseargs.h"
#include "util.h"


/// @brief reads ellpack from path (if path is NULL from stdin); called to read a and b
struct ELLPACK helper_read_and_close(char* path);


int main(int argc, char** argv) {
    struct ARGS args = parse_args(argc, argv);

    pdebug("parsed args:\n");
    pdebug("\ta: '%s'\n", args.a);
    pdebug("\tb: '%s'\n", args.b);
    pdebug("\tout: '%s'\n", args.out);
    pdebug("\timpl_version: '%d'\n", args.impl_version);
    pdebug("\taction: '%s'\n",
            args.action == MULT ? "mult" :
            args.action == BENCH ? "bench" :
            args.action == CHECK_EQ ? "check eq" :
            "!! undefined !!");
    pdebug("\titerations: '%d'\n", args.iterations);
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
        case 5:
            matr_mult_ellpack_ptr = matr_mult_ellpack_V5;
            break;
        default:
            abortIfNULL_msg(0, "fixme: missing function for impl version");
    }

    // read a and b
    pdebug("reading a");
    const struct ELLPACK a_lpk = helper_read_and_close(args.a);
    pdebug("reading b");
    const struct ELLPACK b_lpk = helper_read_and_close(args.b);

    struct ELLPACK res_lpk;

    switch (args.action) {
        case MULT:
            pdebug("starting multiplication...\n");
            matr_mult_ellpack_ptr(&a_lpk, &b_lpk, &res_lpk);
            pdebug("finished multiplication\n");

            FILE* file_out;
            if (args.out != NULL) {
                file_out = (FILE*) abortIfNULL(fopen(args.out, "w"));
            } else {
                file_out = stdout;
            }

            pdebug("writing result\n");
            elpk_write(res_lpk, file_out);
            if (args.out != NULL) fclose(file_out);
            elpk_free(res_lpk);
            break;

        case BENCH:;
#ifdef DEBUG
            fputs("WARNING:  compiled with debug output\n", stderr);
#endif
            struct timespec start, end;
            double elapsed_time;

            clock_gettime(CLOCK_MONOTONIC, &start);

            for (int i = 0; i < args.iterations; i++) {
                matr_mult_ellpack_ptr(&a_lpk, &b_lpk, &res_lpk);
                elpk_free(res_lpk);
                sleep(1);
            }

            clock_gettime(CLOCK_MONOTONIC, &end);

            elapsed_time = (end.tv_sec - start.tv_sec - args.iterations) * 1.0e9 + (end.tv_nsec - start.tv_nsec);
            printf("Average elapsed time per iteration: %.6f seconds\n", elapsed_time / args.iterations / 1.0e9);
            break;

        case CHECK_EQ:
            pdebug("checking if equal...\n");
            elpk_check_equal(a_lpk, b_lpk, args.eq_max_diff);
            break;

        default:
            abortIfNULL_msg(0, "fixme: undefined action");
    }

    elpk_free(a_lpk);
    elpk_free(b_lpk);
    exit(EXIT_SUCCESS);
}


struct ELLPACK helper_read_and_close(char* path) {
    FILE* file;

    if (path != NULL) {
        file = (FILE*) abortIfNULL(fopen(path, "r"));
        pdebug_(" from file: '%s'\n", path);
    } else {
        file = stdin;
        pdebug_(" from stdin\n");
    }

    const struct ELLPACK lpk = elpk_read_validate(file);

    pdebug("done:\n");
#ifdef DEBUG
    if (lpk.noRows * lpk.maxNoNonZero > DEBUG_OUTPUT_MAX_SIZE) {
        fputs("...too large\n", stderr);
    } else {
        elpk_write(lpk, stderr);
    }
#endif

    if (path != NULL) fclose(file);

    return lpk;
}
