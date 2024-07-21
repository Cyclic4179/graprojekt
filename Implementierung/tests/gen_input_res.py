#!/usr/bin/env python3

"""Usage:
    gen_input_res.py [rd] [options]

Options:
    -o PATH     destination dir, if not exists, will be created [default: generated]
    -a N        amount of random dirs to create (int), if omitted, default is executed

    -d N        min dimension (int >= 0), [default: 1]
    -D N        max dimension (int >= -d), [default: 1000]

    -n N        min no_non_zero (int >= 0), [default: 0]
    -N N        max no_non_zero (int >= -n), [default: inf]

    -v N        min value (float), [default: 0]
    -V N        max value (float >= -v), [default: 1_000_000]

    -f          use floats instead of int
    -h          display this msg
"""

from pathlib import Path
from random import randint, uniform, sample
import numpy as np
from scipy.sparse import lil_matrix, csr_matrix
from docopt import docopt
from dataclasses import dataclass, field


@dataclass
class Opt:
    """options container"""

    gen_dir: Path = field(init=False)
    amount: int = field(init=False)

    min_dim: int = field(init=False)
    max_dim: int = field(init=False)

    min_no_non_zero: int = field(init=False)
    max_no_non_zero: int = field(init=False)

    # max/min_val be: pow(2, 64) - 1 ?
    min_val: int = field(init=False)
    max_val: int = field(init=False)

    floats: bool = field(init=False)


opt = Opt()


def gen_random_value(min_val: int, max_val: int, floats: bool) -> float:
    """Randomly generates a value"""
    if floats:
        return uniform(min_val, max_val)
    return float(randint(min_val, max_val))


def elpk_str_of_csr_matrix(a: csr_matrix, no_non_zero: int) -> str:
    """Get ellpack string of csr_matrix"""
    if not a.has_sorted_indices:
        a.sort_indices()

    s_val = ""
    s_index = ""

    data_index = 0
    for i in range(len(a.indptr) - 1):
        # i = current row
        amount_non_zero = 0

        indptr = a.indptr[i]
        next_indptr = a.indptr[i + 1]

        for j in a.indices[indptr:next_indptr]:
            # j = current column
            data = a.data[data_index]
            data_str = "{0:.6f}".format(data).rstrip("0").rstrip(".")

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
    dest = opt.gen_dir.joinpath(dest)
    dest.parent.mkdir(parents=True, exist_ok=True)

    with open(dest, "w", encoding="ascii") as f:
        f.write(elpk_str_of_csr_matrix(a, no_non_zero))


def create(
    n: int,
    m: int,
    no_non_zero: int,
    min_val: int | None = None,
    max_val: int | None = None,
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
    if min_val is None:
        min_val = opt.min_val
    if max_val is None:
        max_val = opt.max_val

    # TODO: maybe some argument to control sparsity
    assert no_non_zero <= m

    a = lil_matrix((n, m), dtype=np.float32)

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
                a[i, k] = gen_random_value(min_val, max_val, floats)

            continue

        cur_row = randint(0, m)
        j = 0

        # this is one row
        while True:
            if cur_row >= m or j == no_non_zero:
                break

            a[i, cur_row] = gen_random_value(min_val, max_val, floats)
            j += 1

            cur_row += randint(1, m - cur_row)  # next index

    return a.tocsr()


def xxx(
    dest: str | Path,
    *,
    n: int,
    k: int,
    m: int,
    no_non_zero_a: int,
    no_non_zero_b: int,
    min_val_a: int | None = None,
    max_val_a: int | None = None,
    min_val_b: int | None = None,
    max_val_b: int | None = None,
    floats=False,
):
    """
    create, multiply, print matrices
    """
    print(f"generating {dest}")

    a = create(n, k, no_non_zero_a, min_val_a, max_val_a, floats)
    b = create(k, m, no_non_zero_b, min_val_b, max_val_b, floats)
    res = a @ b

    no_non_zero_res = max(
        res.indptr[i + 1] - res.indptr[i] for i in range(len(res.indptr) - 1)
    )

    write_elpk(Path(dest, "a"), a, no_non_zero_a)
    write_elpk(Path(dest, "b"), b, no_non_zero_b)
    write_elpk(Path(dest, "res"), res, no_non_zero_res)

    print("finished")


def default():
    xxx("d1", n=40, k=40, m=40,
        no_non_zero_a=40, no_non_zero_b=40,
        max_val_a=100_000, max_val_b=100_000)
    xxx("d2", n=110, k=110, m=110,
        no_non_zero_a=110, no_non_zero_b=110,
        max_val_a=100_000, max_val_b=100_000)
    xxx("d3", n=200, k=200, m=200,
        no_non_zero_a=200, no_non_zero_b=200,
        max_val_a=100_000, max_val_b=100_000)

    xxx("ds1", n=150, k=150, m=150,
        no_non_zero_a=80, no_non_zero_b=80,
        max_val_a=100_000, max_val_b=100_000)
    xxx("ds2", n=300, k=300, m=300,
        no_non_zero_a=150, no_non_zero_b=150,
        max_val_a=100_000, max_val_b=100_000)
    xxx("ds3", n=700, k=700, m=700,
        no_non_zero_a=400, no_non_zero_b=400,
        max_val_a=100_000, max_val_b=100_000)

    xxx("s1", n=400, k=3000, m=400,
        no_non_zero_a=10, no_non_zero_b=10,
        max_val_a=100_000, max_val_b=100_000)
    xxx("s2", n=1000, k=30000, m=1000,
        no_non_zero_a=8, no_non_zero_b=8,
        max_val_a=100_000, max_val_b=100_000)
    xxx("s3", n=10_000, k=100_000, m=10_000,
        no_non_zero_a=1, no_non_zero_b=1,
        max_val_a=100, max_val_b=100)

def rd_gen():
    """random generate a, b, res according to opts"""
    for i in range(opt.amount):
        n = randint(opt.min_dim, opt.max_dim)
        k = randint(opt.min_dim, opt.max_dim)
        m = randint(opt.min_dim, opt.max_dim)

        nonzero_a = randint(0, max(min(k, opt.max_no_non_zero), opt.min_no_non_zero))
        nonzero_b = randint(0, max(min(m, opt.max_no_non_zero), opt.min_no_non_zero))

        xxx(str(i + 1), n=n, k=k, m=m,
            no_non_zero_a=nonzero_a, no_non_zero_b=nonzero_b,
            max_val_a=opt.max_val, max_val_b=opt.max_val,
            floats=opt.floats)


def main():
    args = docopt(__doc__)

    opt.gen_dir = Path(args["-o"])
    opt.amount = int(args["-a"] or -1)

    opt.min_dim = int(args["-d"])
    opt.max_dim = int(args["-D"])

    opt.min_no_non_zero = float(args["-n"])
    opt.max_no_non_zero = float(args["-N"])

    opt.min_val = int(args["-v"])
    opt.max_val = int(args["-V"])

    opt.floats = args["-f"]

    #print(args)

    if args["rd"]:
        rd_gen()
    else:
        default()


if __name__ == "__main__":
    main()



    #xxx("4", n=100, k=100, m=100,
    #    no_non_zero_a=80, no_non_zero_b=80,
    #    max_val_a=99, max_val_b=99)

    #xxx("5", n=9, k=9, m=9,
    #    no_non_zero_a=4, no_non_zero_b=4,
    #    max_val_a=opt.max_val, max_val_b=opt.max_val)

    #xxx("6", n=4, k=4, m=4,
    #    no_non_zero_a=1, no_non_zero_b=1,
    #    max_val_a=99, max_val_b=99,
    #    floats=True)

    #xxx("7", n=9, k=9, m=9,
    #    no_non_zero_a=4, no_non_zero_b=4,
    #    max_val_a=99, max_val_b=99,
    #    floats=True)

    #xxx("8", n=9, k=9, m=9,
    #    no_non_zero_a=4, no_non_zero_b=4,
    #    max_val_a=opt.max_val, max_val_b=opt.max_val,
    #    floats=True)

    #xxx("hu-sp", n=100_000, k=100_000, m=100_000,
    #    no_non_zero_a=5, no_non_zero_b=5,
    #    max_val_a=100, max_val_b=100)

    #xxx("v", n=100, k=10000, m=80,
    #    no_non_zero_a=80, no_non_zero_b=2,
    #    max_val_a=1_000_000_000, max_val_b=1_000_000_000)


    #def d(i):
    #    xxx(f"d{i}", n=i, k=i, m=i,
    #        no_non_zero_a=i, no_non_zero_b=i,
    #        max_val_a=100_000, max_val_b=100_000)
    #xxx("1", n=2, k=2, m=3,
    #    no_non_zero_a=1, no_non_zero_b=1,
    #    max_val_a=100, max_val_b=100)

    #xxx("2", n=1, k=100, m=80,
    #    no_non_zero_a=80, no_non_zero_b=2,
    #    max_val_a=1, max_val_b=1)


    ### v0 - v2
    #d(40)
    #d(80)
    #d(120)
    #d(160)
    #d(200)

    #xxx("ds300", n=300, k=300, m=300,
    #    no_non_zero_a=150, no_non_zero_b=150,
    #    max_val_a=100_000, max_val_b=100_000)

    #xxx("s30", n=30, k=3000, m=30,
    #    no_non_zero_a=30, no_non_zero_b=2,
    #    max_val_a=100_000, max_val_b=100_000)

    #xxx("s-hu", n=1000, k=1000, m=1000,
    #    no_non_zero_a=3, no_non_zero_b=3,
    #    max_val_a=100, max_val_b=100)


    ### v0 - v3
    #d(40)
    #d(110)
    #d(200)
