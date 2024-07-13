#!/usr/bin/env python3
#
# $1: executable
# $2: test dir
# $3: iterations
# $4: max impl version
# $5: benchmark result dir

#$executable -a $2 -b $b -B$4 | awk '{print $6}'


import sys
#import json
import subprocess
from pathlib import Path
import datetime
import pandas as pd


executable = sys.argv[1]
#filea = sys.argv[2]
#fileb = sys.argv[3]
test_dir = sys.argv[2]
#DEFAULT_ITERATIONS = 5
#iterations = sys.argv[3] if len(sys.argv) > 2 else DEFAULT_ITERATIONS
iterations = sys.argv[3]

max_impl_version = int(sys.argv[4])
benchmark_dir = sys.argv[5]


def eprint(*args, **kvargs):
    ''' print to stderr '''
    print(*args, **kvargs, file=sys.stderr)


def exec_bench(a: str, b: str, impl_version: int) -> float:
    '''
    execute benchmark and get time
    '''
    eprint(f"run: {executable} -a {a} -b {b} -V{impl_version} -B{iterations}")

    # f"{executable} -a {a} -b {b} -B{iterations} | awk '{{print $6}}'"
    result = subprocess.run([executable, "-a", a, "-b", b, f"-V{impl_version}", f"-B{iterations}"],
                            capture_output=True, text=True, check=False)

    eprint("...finished")

    try:
        result.check_returncode()
    except subprocess.CalledProcessError as e:
        print(f"stdout:\n{result.stdout}\nstderr:\n{result.stderr}", file=sys.stderr)
        raise e

    return float(result.stdout.split(" ")[5])


def main():
    generated = Path(test_dir, "generated")

    ls = [p for p in generated.iterdir() if p.is_dir()]
    vs = list(range(max_impl_version + 1))

    l = {f"v{v}":
         {f"t{p.name}": exec_bench(p.joinpath("a"), p.joinpath("b"), v)
          for p in ls} for v in vs}

    sorted_l = dict(sorted(((k, dict(sorted(v.items()))) for k, v in l.items())))
    #print(json.dumps(sorted_l))

    df = pd.DataFrame(sorted_l)
    df.index.name = "tests"

    print(df)

    timestr = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    df.to_csv(Path(benchmark_dir, f"{timestr}.csv"))

if __name__ == "__main__":
    main()
