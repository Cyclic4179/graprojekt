#!/usr/bin/env python3
'''Usage:
    bench.py test <executable> <impl_ver>... [options] [-p N -e FLOAT] -t PATH...
    bench.py bench <executable> <impl_ver>... [options] [-i N] -t PATH...

Options:
    -t PATH     dir with tests (eg: tests/generated)
    -b PATH     result dest dir, if not exists, will be created [default: .]
    -h          display this msg
Test:
    -p N        print thresh (dont print content of files more than N chars) [default: 200]
    -e FLOAT    max error passed when testing [default: 0.001]
Bench:
    -i N        iterations [default: 3]
'''


import sys
import subprocess
import re
import atexit
import tempfile
from pathlib import Path
import datetime
import pandas as pd
from docopt import docopt
from dataclasses import dataclass, field


@dataclass
class Opt:
    ''' options container '''
    executable: str = field(init=False)
    impl_versions: list[str] = field(init=False)

    test_dirs: list[Path] = field(init=False)

    # bench
    benchmark_dir: Path = field(init=False)
    iterations: int = field(init=False)

    # test
    max_error: float = field(init=False)
    print_thresh: int = field(init=False)
    tmp_file: Path = field(init=False)


opt = Opt()


def eprint(*args, **kvargs):
    ''' print to stderr '''
    print(*args, **kvargs, file=sys.stderr)


def eprint_file_if_not_too_large(path: Path):
    ''' print file content if char count <= opt.print_thresh '''
    with open(path, "r", encoding="ascii") as f:
        s = f.read()
        if len(s) > opt.print_thresh:
            eprint("...file too large")
        else:
            eprint(s)


def exec_bench(a: Path, b: Path, impl_version: int) -> float:
    ''' execute benchmark and get time '''
    eprint(f"run: {opt.executable} -a {a} -b {b} -V{impl_version} -B{opt.iterations}")

    # f"{executable} -a {a} -b {b} -B{iterations} | awk '{{print $6}}'"
    result = subprocess.run(
            [opt.executable, "-a", a, "-b", b, f"-V{impl_version}", f"-B{opt.iterations}"],
            capture_output=True, text=True, check=False)

    try:
        result.check_returncode()
    except subprocess.CalledProcessError:
        eprint("\n---------------------\nFAILED")
        eprint(f"stdout: >>>>\n{result.stdout}<<<<\n\nstderr: >>>>\n{result.stderr}<<<<")
        sys.exit(1)

    eprint("...finished")

    return float(result.stdout.split(" ")[5])


def exec_test(a: Path, b: Path, res: Path, impl_version: int):
    ''' execute test '''
    eprint(f"run: {opt.executable} -a {a} -b {b} -o {opt.tmp_file} -V{impl_version} -B{opt.iterations}")

    result = subprocess.run(
            [opt.executable, "-a", a, "-b", b, "-o", opt.tmp_file, f"-V{impl_version}"],
            capture_output=True, text=True, check=False)
    try:
        result.check_returncode()
    except subprocess.CalledProcessError:
        eprint("\n---------------------\nFAILED")
        eprint(f"stdout: >>>>\n{result.stdout}<<<<\n\nstderr: >>>>\n{result.stderr}<<<<")
        sys.exit(1)

    eprint(f"check result: {opt.executable} -a {res} -b {opt.tmp_file} -e{opt.max_error}")
    result_eq = subprocess.run([opt.executable, "-a", res, "-b", opt.tmp_file, f"-e{opt.max_error}"],
            capture_output=True, text=True, check=False)

    try:
        result_eq.check_returncode()
    except subprocess.CalledProcessError:
        eprint("\n---------------------\nFAILED")
        eprint("\nfactor a:")
        eprint_file_if_not_too_large(a)
        eprint("\nfactor b:")
        eprint_file_if_not_too_large(b)
        eprint("\nexpected:")
        eprint_file_if_not_too_large(res)
        eprint("\nbut got:")
        eprint_file_if_not_too_large(opt.tmp_file)
        eprint(f"\nstdout: >>>>\n{result_eq.stdout}<<<<\n\nstderr: >>>>\n{result_eq.stderr}<<<<")
        sys.exit(1)

    eprint(" -> PASSED")


def natural_keys(obj):
    return [int(c) if c.isdigit() else c for c in re.split(r'(\d+)', str(obj))]


def bench():
    tests = []
    for test_dir in opt.test_dirs:
        tests.extend(p for p in test_dir.iterdir() if p.is_dir())
    tests = sorted(tests, key=natural_keys)

    res = {f"v{v}":
         {f"t{p.name}": exec_bench(p.joinpath("a"), p.joinpath("b"), v)
          for p in tests} for v in opt.impl_versions}

    df = pd.DataFrame(res)
    df.index.name = "tests"

    print(df)

    timestr = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    dest = opt.benchmark_dir.joinpath(f"bench_v{'-'.join(opt.impl_versions)}_{timestr}.csv")
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


def main():
    args = docopt(__doc__)
    #print(args)

    opt.executable = args["<executable>"]
    opt.impl_versions = args["<impl_ver>"]

    opt.test_dirs = list(map(Path, args["-t"]))

    opt.iterations = int(args["-i"])
    opt.benchmark_dir = Path(args["-b"])
    opt.print_thresh = int(args["-p"])
    opt.max_error = float(args["-e"])

    with tempfile.NamedTemporaryFile(delete=False) as tmpfile:
        opt.tmp_file = Path(tmpfile.name)
    atexit.register(opt.tmp_file.unlink)

    if args["test"]:
        test()
    elif args["bench"]:
        bench()


if __name__ == "__main__":
    main()
