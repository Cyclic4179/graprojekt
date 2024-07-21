#!/usr/bin/env python3

"""Usage:
    bench.py test <executable> [<impl-ver>...] [options] [-p N -e FLOAT] -t PATH...
    bench.py bench <executable> [<impl-ver>...] [options] [-i N] -t PATH...
    bench.py show <csv-file> [-s]

Options:
    -t PATH     dir with tests (eg: tests/generated)
    -b PATH     result dest dir, if not exists, will be created [default: benchmark_results]
    -h          display this msg
Test:
    -T N        timeout after N seconds [default: 10]
    -p N        print thresh (don't print content of files more than N chars) [default: 200]
    -e FLOAT    max error passed when testing [default: 1]
Bench:
    -i N        iterations [default: 3]
Show:
    -s          show plot

Notes:
    if no <impl-ver> is specified, all impl_versions will be used,
    `<executable> -x` is executed to find out the max impl version

    when benchmarking, (timeout + 1) * iterations will be used as bench_timeout for
    `<executable> ... -B`, if bench_timeout hit -> value of timeout set in data
"""


import sys
import subprocess
import re
from pathlib import Path
import datetime
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.colors import ListedColormap
from docopt import docopt
from dataclasses import dataclass, field


@dataclass
class Opt:
    """options container"""

    executable: str = field(init=False)
    impl_versions: list[str] = field(init=False)

    test_dirs: list[Path] = field(init=False)

    timeout: int = field(init=False)

    # bench
    benchmark_dir: Path = field(init=False)
    iterations: int = field(init=False)
    bench_timeout: int = field(init=False)

    # test
    max_error: float = field(init=False)
    print_thresh: int = field(init=False)

    # show
    csv_file: str = field(init=False)
    show_plot: bool = field(init=False)

opt = Opt()


def eprint(*args, **kvargs):
    """print to stderr"""
    print(*args, **kvargs, file=sys.stderr)


def eprint_std_out_err(o: subprocess.CompletedProcess[str] |
                       subprocess.TimeoutExpired |
                       subprocess.CalledProcessError):
    """helper for printing stdout and stderr streams"""
    if o.stdout:
        eprint(f"stdout: >>>>\n{str(o.stdout)}<<<<\n")
    if o.stderr:
        eprint(f"stderr: >>>>\n{str(o.stderr)}<<<<\n")


def eprint_if_short(s: str):
    """print s if not longer than print_thresh"""
    if len(s) > opt.print_thresh:
        eprint("...too large")
    else:
        eprint(s)


def eprint_file_if_small(path: Path):
    """print file content if char count <= opt.print_thresh"""
    with open(path, "r", encoding="ascii") as f:
        eprint_if_short(f.read())


def get_max_impl_ver():
    """
    get max impl version from executable (can be retrieved with -x)
    relies solely on opt.executable
    """
    eprint(f"run: {opt.executable} -x")

    try:
        result = subprocess.run(
            [opt.executable, "-x"],
            capture_output=True,
            text=True,
            check=True,
            timeout=1
        )
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired) as e:
        eprint("failed to get max impl version")
        eprint_std_out_err(e)
        sys.exit(1)

    eprint("...finished")

    return int(result.stdout)


def exec_bench(a: Path, b: Path, impl_version: int) -> float:
    """execute benchmark and get time"""
    eprint(f"run: {opt.executable} -a {a} -b {b} -V{impl_version} -B{opt.iterations}")

    # {executable} -a {a} -b {b} -B{iterations} | awk '{{print $6}}'
    try:
        result = subprocess.run(
            [opt.executable, "-a", a, "-b", b, f"-V{impl_version}", f"-B{opt.iterations}"],
            capture_output=True,
            text=True,
            check=True,
            timeout=opt.bench_timeout
        )

    except subprocess.TimeoutExpired as e:
        eprint(f"\n---------------------\nFAILED: TIMEOUT after {opt.timeout} seconds\n")
        eprint_std_out_err(e)
        return opt.timeout

    except subprocess.CalledProcessError as e:
        eprint("\n---------------------\nFAILED")
        eprint_std_out_err(e)
        sys.exit(1)

    eprint("...finished")

    return float(result.stdout.split(" ")[5])


def exec_test(a: Path, b: Path, res: Path, impl_version: int):
    """execute test"""
    eprint(
        f"run: {opt.executable} -a {a} -b {b} -V{impl_version}"
    )

    try:
        result = subprocess.run(
            [opt.executable, "-a", a, "-b", b, f"-V{impl_version}"],
            capture_output=True,
            text=True,
            check=True,
            timeout=opt.timeout
        )
    except subprocess.TimeoutExpired as e:
        eprint(f"\n---------------------\nFAILED: TIMEOUT after {opt.timeout} seconds\n")
        eprint_std_out_err(e)
        return

    except subprocess.CalledProcessError as e:
        eprint("\n---------------------\nFAILED")
        eprint_std_out_err(e)
        sys.exit(1)

    eprint(
        f"check result: {opt.executable} -a {res} -e{opt.max_error} <<<\"$RESULT\""
    )

    try:
        subprocess.run(
            [opt.executable, "-a", res, f"-e{opt.max_error}"],
            input=result.stdout,
            capture_output=True,
            text=True,
            check=True,
            timeout=1,
        )
    except subprocess.CalledProcessError as e:
        eprint("\n---------------------\nFAILED")
        eprint("\nfactor a:")
        eprint_file_if_small(a)
        eprint("\nfactor b:")
        eprint_file_if_small(b)
        eprint("\nexpected:")
        eprint_file_if_small(res)
        eprint("\nbut got:")
        eprint_if_short(result.stdout)
        eprint("\n")
        eprint_std_out_err(e)
        sys.exit(1)

    eprint(" -> PASSED")


def natural_keys(obj):
    return [int(c) if c.isdigit() else c for c in re.split(r"(\d+)", str(obj))]


def bench():
    tests = []
    for test_dir in opt.test_dirs:
        tests.extend(p for p in test_dir.iterdir() if p.is_dir())
    tests = sorted(tests, key=natural_keys)

    res = {
        f"v{v}": {
            f"{p.name}": exec_bench(p.joinpath("a"), p.joinpath("b"), v) for p in tests
        }
        for v in opt.impl_versions
    }

    df = pd.DataFrame(res)
    df.index.name = "tests"

    print(df)

    timestr = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    dest = opt.benchmark_dir.joinpath(
        f"bench_v{'-v'.join(opt.impl_versions)}_{timestr}.csv"
    )
    print("result in:", dest)
    df.to_csv(dest)


def test():
    tests = []
    for test_dir in opt.test_dirs:
        tests.extend(p for p in test_dir.iterdir() if p.is_dir())
    tests = sorted(tests, key=natural_keys)

    for v in opt.impl_versions:
        for p in tests:
            exec_test(p.joinpath("a"), p.joinpath("b"), p.joinpath("res"), v)

    eprint("SUCCESS")


def show():
    df = pd.read_csv(opt.csv_file)
    print(df)

    if opt.show_plot:
        cmap = ListedColormap(["#0343df", "#e50000", "#ffff14", "#929591"])
        ax = df.plot.bar(x="tests", colormap=cmap)

        ax.set_xlabel(None)
        ax.set_ylabel('time in s')
        ax.set_title('benchmark')

        plt.show()


def main():
    args = docopt(__doc__)
    #print(args)

    opt.executable = args["<executable>"]
    opt.impl_versions = args["<impl-ver>"]

    opt.test_dirs = list(map(Path, args["-t"]))
    opt.timeout = int(args["-T"])

    opt.iterations = int(args["-i"])
    opt.benchmark_dir = Path(args["-b"])
    opt.benchmark_dir.parent.mkdir(exist_ok=True, parents=True)
    opt.bench_timeout = (opt.timeout + 1) * opt.iterations

    opt.print_thresh = int(args["-p"])
    opt.max_error = float(args["-e"])

    opt.csv_file = args["<csv-file>"]
    opt.show_plot = args["-s"]

    if len(opt.impl_versions) == 0 and opt.executable:
        opt.impl_versions = list(map(str, range(1 + get_max_impl_ver())))

    if args["test"]:
        test()
    elif args["bench"]:
        bench()
    elif args["show"]:
        show()


if __name__ == "__main__":
    main()
