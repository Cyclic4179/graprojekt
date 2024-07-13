#!/usr/bin/env python3
#
# $1: path for dir to gen into

import sys
from pathlib import Path
from random import randint, uniform


MIN_VAL = 0
MAX_VAL = pow(2, 64) - 1

gen_dir = Path(sys.argv[1])


def maybe_true(prop: int) -> bool:
    """Returns maybe zero with `1/prop` propability

    Args:
        prop (int): must be >0
    """

    return randint(1, prop) == 1


def gen_random_value(max_val: int, ret_floats: bool) -> str:
    """
    Randomly generates a value
    """

    if ret_floats:
        return "{0:.10f}".format(uniform(MIN_VAL, max_val))
    return str(randint(MIN_VAL, max_val))


def create_a(
    folder: str,
    *args, **kvargs
):
    '''
    calls create with dest=folder/a
    '''
    create(Path(folder, "a"), *args, **kvargs)


def create_b(
    folder: str,
    *args, **kvargs
):
    '''
    calls create with dest=folder/b
    '''
    create(Path(folder, "b"), *args, **kvargs)


def create(
    dest: str | Path,
    no_rows: int,
    no_cols: int,
    no_non_zero: int,
    max_val: int,
    use_floats: bool,
):
    """Creates a random sparse matrix

    Args:
        dest (str): File to generate into
        no_rows (int): number of rows
        no_cols (int): number of columns
        no_non_zero (int): maximal number of non zero values per row
        max_val (int): highest possible value
        use_floats (bool): should floats be used as values
    """
    # TODO: maybe some argument to control sparsity
    assert no_non_zero <= no_cols

    dest = gen_dir.joinpath(dest)
    dest.parent.mkdir(parents=True, exist_ok=True)

    s_float = ""
    s_index = ""

    # index of row that will have max nonzero entries
    row_with_max = randint(0, no_rows - 1)

    # strategy: start at random index, go forth with random steps
    # and if too large or already filled no_non_zero entries go to
    # next line, repeat
    # also, ensure there exists at least one line with no_non_zero nonzero entries
    for i in range(no_rows):

        if i == row_with_max:
            # row will be max
            for k in range(no_non_zero):
                s_float += f"{gen_random_value(max_val, use_floats)},"
                s_index += f"{k},"
            continue

        cur_row = randint(0, no_cols)
        j = 0

        # this is one row
        while True:
            if cur_row > no_cols - 1 or j == no_non_zero:
                rem_cells = no_non_zero - j

                if rem_cells > 0:
                    s = "*," * rem_cells # fill with remaining '*,'
                    s_float += s
                    s_index += s

                break

            s_float += f"{gen_random_value(max_val, use_floats)},"
            s_index += f"{cur_row},"
            j += 1

            if cur_row == no_cols - 1:
                rem_cells = no_non_zero - j

                if rem_cells > 0:
                    s = "*," * rem_cells # fill with remaining '*,'
                    s_float += s
                    s_index += s

                break

            cur_row += randint(1, no_cols - cur_row) # next index

    s_float = s_float[:-1]
    s_index = s_index[:-1]

    with open(str(dest), "w", encoding="ascii") as f:
        f.write(f"{no_rows},{no_cols},{no_non_zero}\n"
                f"{s_float}\n"
                f"{s_index}\n")


def main():
    create_a("1", 1, 100, 80, 99, False)
    create_b("1", 100, 4, 2, 99, False)

    #create_a("2", 10000, 10000, 400, 99, False)
    #create_b("2", 10000, 10000, 200, 99, False)

    create_a("3", 100, 100, 80, 99, False)
    create_b("3", 100, 100, 80, 99, False)

    create_a("4", 9, 9, 4, MAX_VAL, False)
    create_b("4", 9, 9, 4, MAX_VAL, False)

    create_a("5", 4, 4, 2, 99, True)
    create_b("5", 4, 4, 2, 99, True)

    create_a("6", 9, 9, 4, 99, True)
    create_b("6", 9, 9, 4, 99, True)

    create_a("7", 9, 9, 4, MAX_VAL, True)
    create_b("7", 9, 9, 4, MAX_VAL, True)


if __name__ == "__main__":
    main()
