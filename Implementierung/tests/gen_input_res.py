#!/usr/bin/env python3
#
# $1: path for dir to gen into
# $2: amount of random dirs to create, if omitted, default is executed (int)
# $3: max dimention, default 1000 (int)
# $4: max value, default pow(2, 64) - 1 (int)


import sys
from pathlib import Path
from random import randint, uniform, sample


if len(sys.argv) == 1:
    print("usage: see top of skript")
    print("$1: path for dir to gen into")
    print("$2: amount of random dirs to create, if omitted, default is executed (int)")
    print("$3: max dimention, default 1000 (int)")
    print("$4: max value, default pow(2, 64) - 1 (int)")
    sys.exit(0)


MIN_DIM = 1
MAX_DIM = 1000 if len(sys.argv) <= 3 else int(sys.argv[3])

MIN_VAL = 0
MAX_VAL = pow(2, 64) - 1 if len(sys.argv) <= 4 else int(sys.argv[4])

gen_dir = Path(sys.argv[1])


def gen_random_value(max_val: int, ret_floats: bool) -> str:
    """
    Randomly generates a value
    """

    if ret_floats:
        return "{0:.10f}".format(uniform(MIN_VAL, max_val))
    return str(randint(MIN_VAL, max_val))


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

    l_float = []
    l_index = []

    # index of row that will have max nonzero entries
    row_with_max = randint(0, no_rows - 1)

    # strategy: start at random index, go forth with random steps
    # and if too large or already filled no_non_zero entries go to
    # next line, repeat
    # also, ensure there exists at least one line with no_non_zero nonzero entries
    for i in range(no_rows):

        if i == row_with_max:
            # row will be max
            indices = range(no_cols)
            for k in sorted(sample(indices, k=no_non_zero)):
                l_float.append(str(gen_random_value(max_val, use_floats)))
                l_index.append(str(k))
            continue

        cur_row = randint(0, no_cols)
        j = 0

        # this is one row
        while True:
            if cur_row > no_cols - 1 or j == no_non_zero:
                rem_cells = no_non_zero - j

                if rem_cells > 0:
                    s = ["*"] * rem_cells # fill with remaining '*,'
                    l_float.extend(s)
                    l_index.extend(s)

                break

            l_float.append(str(gen_random_value(max_val, use_floats)))
            l_index.append(str(cur_row))
            j += 1

            if cur_row == no_cols - 1:
                rem_cells = no_non_zero - j

                if rem_cells > 0:
                    s = ["*"] * rem_cells # fill with remaining '*,'
                    l_float.extend(s)
                    l_index.extend(s)

                break

            cur_row += randint(1, no_cols - cur_row) # next index

    s_float = ",".join(l_float)
    s_index = ",".join(l_index)

    with open(str(dest), "w", encoding="ascii") as f:
        f.write(f"{no_rows},{no_cols},{no_non_zero}\n"
                f"{s_float}\n"
                f"{s_index}\n")


def default():
    create("1/a", 1, 100, 80, 99, False)
    create("1/b", 100, 4, 2, 99, False)

    create("2/a", 1000, 1000, 400, 99, False)
    create("2/b", 1000, 1000, 200, 99, False)

    create("3/a", 100, 100, 80, 99, False)
    create("3/b", 100, 100, 80, 99, False)

    create("4/a", 9, 9, 4, MAX_VAL, False)
    create("4/b", 9, 9, 4, MAX_VAL, False)

    create("5/a", 4, 4, 2, 99, True)
    create("5/b", 4, 4, 2, 99, True)

    create("6/a", 9, 9, 4, 99, True)
    create("6/b", 9, 9, 4, 99, True)

    create("7/a", 9, 9, 4, MAX_VAL, True)
    create("7/b", 9, 9, 4, MAX_VAL, True)


def main():
    if len(sys.argv) == 2:
        default()
        return

    amount = int(sys.argv[2])

    for i in range(amount):
        n = randint(MIN_DIM, MAX_DIM)
        k = randint(MIN_DIM, MAX_DIM)
        m = randint(MIN_DIM, MAX_DIM)

        nonzero_a = randint(0, k)
        nonzero_b = randint(0, m)

        create(f"{i}/a", n, k, nonzero_a, MAX_VAL, True)
        create(f"{i}/b", k, m, nonzero_b, MAX_VAL, True)


if __name__ == "__main__":
    main()
