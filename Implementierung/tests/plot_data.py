#!/usr/bin/env python3

"""Usage:
    plot_data.py <csv file to open>
"""

import sys
import matplotlib.pyplot as plt
import pandas as pd
from matplotlib.colors import ListedColormap


if len(sys.argv) == 1:
    print("usage:")
    print("    plot_data.py <csv file to open>")
    sys.exit(0)

file = sys.argv[1]


def main():
    df = pd.read_csv(file)
    print(df)

    cmap = ListedColormap(["#0343df", "#e50000", "#ffff14", "#929591"])
    ax = df.plot.bar(x="tests", colormap=cmap)

    ax.set_xlabel(None)
    ax.set_ylabel('time in s')
    ax.set_title('benchmark')

    plt.show()


if __name__ == "__main__":
    main()
