#!/usr/bin/env python3
#
# $1: executable
# $2: test dir
# $3: iterations (int)
# $4: max impl version (int)
# $5: benchmark result dir


import sys
import subprocess
import re
from pathlib import Path
import datetime
import pandas as pd


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


executable = sys.argv[1]
test_dir = sys.argv[2]
iterations = sys.argv[3]
max_impl_version = int(sys.argv[4])
benchmark_dir = sys.argv[5]


def eprint(*args, **kvargs):
    ''' print to stderr '''
    print(*args, **kvargs, file=sys.stderr)


def exec_bench(a: str, b: str, impl_version: int) -> float:
    ''' execute benchmark and get time '''
    eprint(f"run: {executable} -a {a} -b {b} -V{impl_version} -B{iterations}")

    # f"{executable} -a {a} -b {b} -B{iterations} | awk '{{print $6}}'"
    result = subprocess.run([executable, "-a", a, "-b", b, f"-V{impl_version}", f"-B{iterations}"],
                            capture_output=True, text=True, check=False)

    try:
        result.check_returncode()
    except subprocess.CalledProcessError:
        eprint("FAILED")
        eprint(f"stdout: >>>>\n{result.stdout}<<<<\n\nstderr: >>>>\n{result.stderr}<<<<")
        sys.exit(1)

    eprint("...finished")

    return float(result.stdout.split(" ")[5])


def natural_keys(obj):
    return [ int(c) if c.isdigit() else c for c in re.split(r'(\d+)', str(obj)) ]


def main():
    generated = Path(test_dir, "generated")

    ls = [p for p in sorted(generated.iterdir(), key=natural_keys) if p.is_dir()]
    vs = list(range(max_impl_version + 1))

    res = {f"v{v}":
         {f"t{p.name}": exec_bench(p.joinpath("a"), p.joinpath("b"), v)
          for p in ls} for v in vs}

    #sorted_l = dict(sorted(((k, dict(sorted(v.items()))) for k, v in l.items())))

    df = pd.DataFrame(res)
    df.index.name = "tests"

    print(df)

    timestr = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    dest = Path(benchmark_dir, f"bench_{timestr}.csv")
    print("result in:", dest)
    df.to_csv(dest)
    #df.to_csv(Path(benchmark_dir, "bench.csv"))

if __name__ == "__main__":
    main()
