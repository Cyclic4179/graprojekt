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
    sparsity: int,
    use_floats: bool,
):
    """Creates a random sparse matrix

    Args:
        dest (str): File to generate into
        no_rows (int): number of rows
        no_cols (int): number of columns
        no_non_zero (int): maximal number of non zero values per row
        max_val (int): highest possible value
        sparsity (int): Every sparsity. value remains empty
        use_floats (bool): should floats be used as values
    """
    assert no_non_zero < no_cols

    dest = gen_dir.joinpath(dest)
    dest.parent.mkdir(parents=True, exist_ok=True)

    s_float = ""
    s_index = ""

    wrote_max_row = False

    # strategy: start at random index, go forth with random steps
    # and if too large or already filled no_non_zero entries go to
    # next line, repeat
    for _ in range(no_rows):
        cur_row = randint(0, no_cols)
        j = 0
        #print("\tbatch start")

        # this is one row
        while True:
            if cur_row > no_cols - 1 or j == no_non_zero:
                rem_cells = no_non_zero - j
                #print("cur_row:", cur_row)
                #print("j:", j)
                #print("rem_cells:", rem_cells)

                if rem_cells <= 0:
                    #print("aaaa")
                    wrote_max_row = True

                else:
                    s = "*," * rem_cells # fill with remaining '*,'
                    #print("s:\t", s)
                    s_float += s
                    s_index += s

                break

            s_float += f"{gen_random_value(max_val, use_floats)},"
            #print("fchange:", s_float)
            s_index += f"{cur_row},"
            j += 1

            if cur_row == no_cols - 1:
                rem_cells = no_non_zero - j
                #print("cur_row:", cur_row)
                #print("j:", j)
                #print("rem_cells:", rem_cells)

                if rem_cells <= 0:
                    #print("aaaa")
                    wrote_max_row = True

                else:
                    s = "*," * rem_cells # fill with remaining '*,'
                    #print("s:\t", s)
                    s_float += s
                    s_index += s
                break

            cur_row += randint(1, no_cols - cur_row) # next index

    if not wrote_max_row:
        chr_to_rem = 2 * no_non_zero
        #print(s_float)
        #print(s_index)
        s_float = s_float[:-chr_to_rem]
        s_index = s_index[:-chr_to_rem]
        #print(s_float)
        #print(s_index)

        for i in range(no_non_zero):
            s_float += f"{gen_random_value(max_val, use_floats)},"
            s_index += f"{i},"

    s_float = s_float[:-1]
    s_index = s_index[:-1]

    #s_float += self.value(max, floats)
    #s_index += self.index(noCols,previous + 1)

    #for i in range(itemsCount - 1):
    #    if emptyLine2[i]:
    #        #f.write("*,")
    #        index = 0
    #    else:
    #        noRowsAfter = noNonZero - (i % noNonZero) - 1
    #        val = self.index(noCols - noRowsAfter, index + 1)
    #        f.write(f"{val},")
    #        index = 0 if noRowsAfter == 0 else val


    with open(str(dest), "w", encoding="ascii") as f:
        f.write(f"{no_rows},{no_cols},{no_non_zero}\n"
                f"{s_float}\n"
                f"{s_index}\n")


def main():
    create_a("1", 4, 4, 2, 99, 3, False)
    create_b("1", 4, 4, 2, 99, 3, False)

    create_a("2", 10000, 10000, 400, 99, 3, False)
    create_b("2", 10000, 10000, 200, 99, 3, False)

    create_a("3", 100, 100, 80, 99, 3, False)
    create_b("3", 100, 100, 80, 99, 3, False)

    #create_a("3", 9, 9, 4, MAX_VAL, 3, False)
    #create_b("3", 9, 9, 4, MAX_VAL, 3, False)

    #create_a("4", 4, 4, 2, 99, 3, True)
    #create_b("4", 4, 4, 2, 99, 3, True)

    #create_a("5", 9, 9, 4, 99, 3, True)
    #create_b("5", 9, 9, 4, 99, 3, True)

    #create_a("6", 9, 9, 4, MAX_VAL, 3, True)
    #create_b("6", 9, 9, 4, MAX_VAL, 3, True)


if __name__ == "__main__":
    main()
