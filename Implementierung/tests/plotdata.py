#!/usr/bin/env python3

import sys
import matplotlib.pyplot as plt
import pandas as pd
from matplotlib.colors import ListedColormap


if len(sys.argv) > 1:
    file = sys.argv[1]
else:
    file = sys.stdin.readline()


def main():
    df = pd.read_csv("data.csv")
    print(df)

    cmap = ListedColormap(['#0343df', '#e50000', '#ffff14', '#929591'])
    ax = df.plot.bar(x='tests', colormap=cmap)

    ax.set_xlabel(None)
    ax.set_ylabel('time')
    ax.set_title('benchmark')

    plt.show()


if __name__ == "__main__":
    main()
