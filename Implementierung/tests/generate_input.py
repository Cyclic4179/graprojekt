#!/usr/bin/env python3

import os
from random import randint, uniform
from argparse import ArgumentParser, HelpFormatter

MIN = 0
MAX = pow(2, 64) - 1

DIR = os.path.join(os.path.dirname(__file__), "generated")


class InputGenerator:
    def __init__(self):
        pass

    def empty(self, sparsity: int) -> bool:
        """Randomly decides whether a value should remain empty

        Args:
            sparsity (int): Every sparsity. value remains empty

        Returns:
            bool: should the value remain empty
        """

        return not randint(0, sparsity)

    def value(self, maximum: int, floats: bool) -> str:
        """Randomly generates a value

        Args:
            maximum (int): highest possible value
            floats (bool): should floats be returned

        Returns:
            str: generated value
        """

        if floats:
            return "{0:.10f}".format(uniform(MIN, maximum))
        else:
            return f"{randint(MIN, maximum)}"

    def index(self, maximum: int, minimum: int) -> int:
        """Randomly generates a index

        Args:
            maximum (int): highest possible index
            minimum (int): smallest possible index

        Returns:
            int: generated index
        """

        return randint(minimum, maximum)

    def create(
        self,
        folder: str,
        a: bool,
        noRows: int,
        noCols: int,
        noNonZero: int,
        max: int,
        sparsity: int,
        floats: bool,
    ):
        """Creates a random sparse matrix

        Args:
            folder (str): Folder to generate into
            a (bool): True to generate a, False to generate b
            noRows (int): number of rows
            noCols (int): number of columns
            noNonZero (int): maximal number of non zero values per row
            max (int): highest possible value
            sparsity (int): Every sparsity. value remains empty
            floats (bool): should floats be used as values
        """

        itemsCount: int = noRows * noNonZero

        directory = os.path.join(DIR, folder)
        if not os.path.exists(directory):
            os.makedirs(directory)

        with open(os.path.join(directory, "a" if a else "b"), "w") as f:

            # ---------------------------- LINE 1 ---------------------------- #

            f.write(f"{noRows},{noCols},{noNonZero}\n")

            # ---------------------------- LINE 2 ---------------------------- #

            emptyLine2 = []

            for i in range(itemsCount - 1):
                if (i % noNonZero == noNonZero - 1) and (self.empty(sparsity)):
                    f.write("*,")
                    emptyLine2.append(True)
                else:
                    f.write(f"{self.value(max, floats)},")
                    emptyLine2.append(False)

            f.write(f"{self.value(max, floats)}\n")
            emptyLine2.append(False)

            # ---------------------------- LINE 3 ---------------------------- #

            previous: int = -1
            for i in range(itemsCount - 1):
                if emptyLine2[i]:
                    f.write("*,")
                    previous = -1
                else:
                    noRowsAfter = noNonZero - (i % noNonZero) - 1
                    val = self.index(noCols - noRowsAfter - 1, previous + 1)
                    f.write(f"{val},")
                    previous = -1 if noRowsAfter == 0 else val

            f.write(f"{self.index(noCols - 1, previous + 1)}\n")


def default():
    ig = InputGenerator()

    ig.create("1", True, 6, 4, 4, 99, 3, False)
    ig.create("1", False, 4, 5, 3, 99, 3, False)

    ig.create("2", True, 100, 100, 85, 99, 3, False)
    ig.create("2", False, 100, 100, 80, 99, 3, False)

    ig.create("3", True, 300, 200, 150, MAX, 3, False)
    ig.create("3", False, 200, 250, 150, MAX, 3, False)

    ig.create("4", True, 20, 2000, 1800, 99, 3, True)
    ig.create("4", False, 2000, 20, 15, 99, 3, True)

    ig.create("5", True, 500, 500, 300, 99, 3, True)
    ig.create("5", False, 500, 500, 400, 99, 3, True)

    ig.create("6", True, 9, 9, 4, MAX, 3, True)
    ig.create("6", False, 9, 9, 4, MAX, 3, True)


def main():
    parser = ArgumentParser(
        prog="generate_input",
        formatter_class=lambda prog: HelpFormatter(prog, max_help_position=80),
    )
    parser.add_argument("--folder", help="Folder to generate into", type=str)

    parser.add_argument("--aNoRows", help="A: number of rows", type=int)
    parser.add_argument("--aNoCols", help="A: number of columns", type=int)
    parser.add_argument(
        "--aNoNonZero", help="A: maximal number of non zero values per row", type=int
    )
    parser.add_argument("--aMax", help="A: highest possible value", type=int)
    parser.add_argument(
        "--aSparsity", help="A: Every sparsity. value remains empty", type=int
    )
    parser.add_argument(
        "--aFloats", help="A: should floats be used as values", type=bool
    )

    parser.add_argument("--bNoRows", help="B: number of rows", type=int)
    parser.add_argument("--bNoCols", help="B: number of columns", type=int)
    parser.add_argument(
        "--bNoNonZero", help="B: maximal number of non zero values per row", type=int
    )
    parser.add_argument("--bMax", help="B: highest possible value", type=int)
    parser.add_argument(
        "--bSparsity", help="B: Every sparsity. value remains empty", type=int
    )
    parser.add_argument(
        "--bFloats", help="B: should floats be used as values", type=bool
    )

    args = parser.parse_args()

    if args.folder is None:
        default()
    else:
        ig = InputGenerator()
        ig.create(
            args.folder,
            True,
            args.aNoRows,
            args.aNoCols,
            args.aNoNonZero,
            args.aMax,
            args.aSparsity,
            args.aFloats,
        )
        ig.create(
            args.folder,
            False,
            args.bNoRows,
            args.bNoCols,
            args.bNoNonZero,
            args.bMax,
            args.bSparsity,
            args.bFloats,
        )


if __name__ == "__main__":
    main()
