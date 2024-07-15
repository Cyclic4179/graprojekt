#!/usr/bin/env python3
#
# $1: csv file to open

import sys
import matplotlib.pyplot as plt
import pandas as pd
from matplotlib.colors import ListedColormap


if len(sys.argv) == 1:
    print("usage:")
    with open(__file__, "r", encoding="ascii") as f:
        for l in f:
            l = l.strip()
            if l == "":
                break
            l = l.removeprefix('# ').strip()
            if l != "":
                print(f"  - {l}")
    sys.exit(0)

file = sys.argv[1]


def main():
    df = pd.read_csv(file)
    print(df)

    cmap = ListedColormap(['#0343df', '#e50000', '#ffff14', '#929591'])
    ax = df.plot.bar(x='tests', colormap=cmap)

    ax.set_xlabel(None)
    ax.set_ylabel('time')
    ax.set_title('benchmark')

    plt.show()


if __name__ == "__main__":
    main()
