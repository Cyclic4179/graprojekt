#!/usr/bin/env python3
#
# $1: path for dir to gen into
# $2: amount of random dirs to create, if omitted, default is executed (int)
# $3: max dimention, default 1000 (int)
# $4: min value, default 0
# $5: max value, default pow(2, 64) - 1 (int)
# $6: if set, use floats instead of int (anything)


import sys
from pathlib import Path
from random import randint, uniform, sample
import numpy as np
from scipy.sparse import lil_matrix, csr_matrix


if len(sys.argv) == 1:
    print("usage: see top of skript")
    print("$1: path for dir to gen into")
    print("$2: amount of random dirs to create, if omitted, default is executed (int)")
    print("$3: max dimention, default 1000 (int)")
    print("$4: min value, default 0")
    print("$5: max value, default pow(2, 64) - 1 (int)")
    print("$6: if set, use floats instead of int (anything)")
    sys.exit(0)


MIN_DIM = 1
MAX_DIM = 1000 if len(sys.argv) <= 3 else int(sys.argv[3])

MIN_VAL = 0 if len(sys.argv) <= 4 else int(sys.argv[4])
# MIN_VAL be: pow(2, 64) - 1 ?
MAX_VAL = pow(10, 12) if len(sys.argv) <= 5 else int(sys.argv[5])

FLOATS = len(sys.argv) > 6

gen_dir = Path(sys.argv[1])


def gen_random_value(min_val: int, max_val: int, floats: bool) -> float:
    """ Randomly generates a value """
    if floats:
        return uniform(min_val, max_val)
    return float(randint(min_val, max_val))


def elpk_str_of_csr_matrix(a: csr_matrix, no_non_zero: int, floats: bool = False) -> str:
    """ Get ellpack string of csr_matrix """
    s_val = ""
    s_index = ""

    data_index = 0
    for i in range(len(a.indptr) - 1):
        # i = current row
        amount_non_zero = 0

        indptr = a.indptr[i]
        next_indptr = a.indptr[i+1]

        for j in a.indices[indptr:next_indptr]:
            # j = current column
            data = a.data[data_index]
            if floats:
                data_str = ('%.10f' % data).rstrip('0').rstrip('.')
            else:
                data_str = str(data)

            s_val += f"{data_str},"
            s_index += f"{j},"

            data_index += 1
            amount_non_zero += 1

        assert amount_non_zero <= no_non_zero

        rem_cells = no_non_zero - amount_non_zero
        s = "*," * rem_cells
        s_val += s
        s_index += s

    n, m = a.shape

    return f"{n},{m},{no_non_zero}\n{s_val[:-1]}\n{s_index[:-1]}\n"


def write_elpk(dest: str | Path, a: csr_matrix, no_non_zero: int):
    dest = gen_dir.joinpath(dest)
    dest.parent.mkdir(parents=True, exist_ok=True)

    with open(dest, "w", encoding="ascii") as f:
        f.write(elpk_str_of_csr_matrix(a, no_non_zero))


def create(
    n: int,
    m: int,
    no_non_zero: int,
    min_val: int = MIN_VAL,
    max_val: int = MAX_VAL,
    floats: bool = False,
) -> csr_matrix:
    """Creates a random sparse matrix in Q^(n x m)

    Args:
        n (int): number of rows
        m (int): number of columns
        no_non_zero (int): maximal number of non zero values per row
        max_val (int): highest possible value
        min_val (int): lowest possible value
        floats (bool): use floats instead of ints
    """

    # TODO: maybe some argument to control sparsity
    assert no_non_zero <= m

    a = lil_matrix((n, m), dtype=np.int64)
    #l_float = []
    #l_index = []

    # index of row that will have max nonzero entries
    row_with_max = randint(0, n - 1)

    # strategy: start at random index, go forth with random steps
    # and if too large or already filled no_non_zero entries go to
    # next line, repeat
    # also, ensure there exists at least one line with no_non_zero nonzero entries
    for i in range(n):

        if i == row_with_max:
            # row will be max
            indices = range(m)
            for k in sorted(sample(indices, k=no_non_zero)):
                a[i, k] = gen_random_value(-max_val, max_val, floats)
                #l_float.append(str(gen_random_value(max_val, use_floats)))
                #l_index.append(str(k))
            continue

        cur_row = randint(0, m)
        j = 0

        # this is one row
        while True:
            if cur_row >= m or j == no_non_zero:
                #rem_cells = no_non_zero - j

                #if rem_cells > 0:
                #    s = ["*"] * rem_cells # fill with remaining '*,'
                #    l_float.extend(s)
                #    l_index.extend(s)

                break

            a[i, cur_row] = gen_random_value(min_val, max_val, floats)
            #l_float.append(str(gen_random_value(max_val, use_floats)))
            #l_index.append(str(cur_row))
            j += 1

            #if cur_row == m - 1:
            #    rem_cells = no_non_zero - j

            #    #if rem_cells > 0:
            #    #    s = ["*"] * rem_cells # fill with remaining '*,'
            #    #    l_float.extend(s)
            #    #    l_index.extend(s)

            #    break

            cur_row += randint(1, m - cur_row) # next index

    return a.tocsr()

    #s_float = ",".join(l_float)
    #s_index = ",".join(l_index)

    #with open(str(dest), "w", encoding="ascii") as f:
    #    f.write(f"{n},{m},{no_non_zero}\n"
    #            f"{s_float}\n"
    #            f"{s_index}\n")


def xxx(dest: str | Path, *, n: int, k: int, m: int,
        no_non_zero_a: int,
        no_non_zero_b: int,
        min_val_a: int = MIN_VAL,
        max_val_a: int = MAX_VAL,
        min_val_b: int = MIN_VAL,
        max_val_b: int = MAX_VAL,
        floats = False):
    """
    create, multiply, print matricies
    """
    a = create(n, k, no_non_zero_a, min_val_a, max_val_a, floats)
    b = create(k, m, no_non_zero_b, min_val_b, max_val_b, floats)
    res = a @ b

    no_non_zero_res = max(res.indptr[i+1] - res.indptr[i] for i in range(len(res.indptr) - 1))

    write_elpk(Path(dest, "a"), a, no_non_zero_a)
    write_elpk(Path(dest, "b"), b, no_non_zero_b)
    write_elpk(Path(dest, "res"), res, no_non_zero_res)


#def create(
#    dest: str | Path,
#    no_rows: int,
#    no_cols: int,
#    no_non_zero: int,
#    max_val: int,
#    use_floats: bool,
#) -> lil_matrix:
#    """Creates a random sparse matrix
#
#    Args:
#        dest (str): File to generate into
#        no_rows (int): number of rows
#        no_cols (int): number of columns
#        no_non_zero (int): maximal number of non zero values per row
#        max_val (int): highest possible value
#        use_floats (bool): should floats be used as values
#    """
#    # TODO: maybe some argument to control sparsity
#    assert no_non_zero <= no_cols
#
#    dest = gen_dir.joinpath(dest)
#    dest.parent.mkdir(parents=True, exist_ok=True)
#
#    l_float = []
#    l_index = []
#
#    # index of row that will have max nonzero entries
#    row_with_max = randint(0, no_rows - 1)
#
#    # strategy: start at random index, go forth with random steps
#    # and if too large or already filled no_non_zero entries go to
#    # next line, repeat
#    # also, ensure there exists at least one line with no_non_zero nonzero entries
#    for i in range(no_rows):
#
#        if i == row_with_max:
#            # row will be max
#            indices = range(no_cols)
#            for k in sorted(sample(indices, k=no_non_zero)):
#                l_float.append(str(gen_random_value(max_val, use_floats)))
#                l_index.append(str(k))
#            continue
#
#        cur_row = randint(0, no_cols)
#        j = 0
#
#        # this is one row
#        while True:
#            if cur_row > no_cols - 1 or j == no_non_zero:
#                rem_cells = no_non_zero - j
#
#                if rem_cells > 0:
#                    s = ["*"] * rem_cells # fill with remaining '*,'
#                    l_float.extend(s)
#                    l_index.extend(s)
#
#                break
#
#            l_float.append(str(gen_random_value(max_val, use_floats)))
#            l_index.append(str(cur_row))
#            j += 1
#
#            if cur_row == no_cols - 1:
#                rem_cells = no_non_zero - j
#
#                if rem_cells > 0:
#                    s = ["*"] * rem_cells # fill with remaining '*,'
#                    l_float.extend(s)
#                    l_index.extend(s)
#
#                break
#
#            cur_row += randint(1, no_cols - cur_row) # next index
#
#    s_float = ",".join(l_float)
#    s_index = ",".join(l_index)
#
#    with open(str(dest), "w", encoding="ascii") as f:
#        f.write(f"{no_rows},{no_cols},{no_non_zero}\n"
#                f"{s_float}\n"
#                f"{s_index}\n")
#

def default():
    xxx("1", n=2, k=2, m=3,
        no_non_zero_a=1, no_non_zero_b=1,
        max_val_a=100, max_val_b=100)
    #a = create(1, 2, 1, max_val=99)
    #print(a.todense())
    #print(elpk_str_of_csr_matrix(a, 1))
    #create(2, 4, 2, max_val=99).todense()

    #create("1/a", 1, 100, 80, 99, False)
    #create("1/b", 100, 4, 2, 99, False)
    xxx("1", n=1, k=100, m=80,
        no_non_zero_a=80, no_non_zero_b=2,
        max_val_a=99, max_val_b=99)

    #create("2/a", 1000, 1000, 400, 99, False)
    #create("2/b", 1000, 1000, 200, 99, False)
    xxx("2", n=1000, k=1000, m=1000,
        no_non_zero_a=400, no_non_zero_b=200,
        max_val_a=99, max_val_b=99)

    #create("3/a", 100, 100, 80, 99, False)
    #create("3/b", 100, 100, 80, 99, False)
    xxx("3", n=100, k=100, m=100,
        no_non_zero_a=80, no_non_zero_b=80,
        max_val_a=99, max_val_b=99)

    #create("4/a", 9, 9, 4, MAX_VAL, False)
    #create("4/b", 9, 9, 4, MAX_VAL, False)
    xxx("4", n=9, k=9, m=9,
        no_non_zero_a=4, no_non_zero_b=4,
        max_val_a=MAX_VAL, max_val_b=MAX_VAL)

    #create("5/a", 4, 4, 2, 99, True)
    #create("5/b", 4, 4, 2, 99, True)
    xxx("5", n=4, k=4, m=4,
        no_non_zero_a=1, no_non_zero_b=1,
        max_val_a=99, max_val_b=99,
        floats=True)

    #create("6/a", 9, 9, 4, 99, True)
    #create("6/b", 9, 9, 4, 99, True)
    xxx("6", n=9, k=9, m=9,
        no_non_zero_a=4, no_non_zero_b=4,
        max_val_a=99, max_val_b=99,
        floats=True)

    #create("7/a", 9, 9, 4, MAX_VAL, True)
    #create("7/b", 9, 9, 4, MAX_VAL, True)
    xxx("7", n=9, k=9, m=9,
        no_non_zero_a=4, no_non_zero_b=4,
        max_val_a=MAX_VAL, max_val_b=MAX_VAL,
        floats=True)

    # ./main could not handle
    #xxx("8", n=100_000, k=100_000, m=100_000,
    #    no_non_zero_a=10, no_non_zero_b=10,
    #    max_val_a=100, max_val_b=100)


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

        #create(f"{i}/a", n, k, nonzero_a, MAX_VAL)
        #create(f"{i}/b", k, m, nonzero_b, MAX_VAL)
        xxx(str(i), n=n, k=k, m=m,
            no_non_zero_a=nonzero_a, no_non_zero_b=nonzero_b,
            max_val_a=MAX_VAL, max_val_b=MAX_VAL,
            floats=True)


if __name__ == "__main__":
    main()
