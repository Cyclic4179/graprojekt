#!/usr/bin/env bash
#
# $1: executable
# $2: tolerated error

executable=$(realpath $1)
SOURCE_DIR=`dirname $(realpath ${BASH_SOURCE[0]})`

cd $SOURCE_DIR

#./gen_input_res.py $SOURCE_DIR/generated 100


function print_if_not_too_large() {
    if test `wc -c <<<"$1"` -gt 100; then
        echo ...too large
    else
        echo $1
    fi
}


versions=`seq 0 $($executable -x)`
#versions="0 1 2"
tests=`find * -maxdepth 0 -type d -not -name 'generated' ; { echo generated/* | tr " " "\n" | sort -V ; }`
maxerr=$2

echo testing versions: $versions

for v in $versions; do
    echo start test for version $v

    for i in $tests; do
        echo running: testcase `basename $i`

        # hack to not trim trailing newline
        got=$($executable -a $i/a -b $i/b -V $v; echo suffix)
        got=${got%suffix}

        expected=$(cat $i/res ; echo suffix)
        expected=${expected%suffix}

        #echo $got
        #echo $expected

        if error=$($executable -e"$maxerr" -a <(echo "$got") -b <(echo "$expected")); then
            echo " -> "PASSED
        else
            echo
            echo ---------------------
            echo FAILED: testcase `basename $i`
            echo

            echo error: $error
            echo

            echo factor a:
            print_if_not_too_large "$(cat $i/a)"
            echo

            echo factor b:
            print_if_not_too_large "$(cat $i/b)"
            echo

            echo expected result:
            print_if_not_too_large "$expected"
            echo

            echo but got:
            print_if_not_too_large "$got"

            exit
        fi
    done
done

echo SUCCESS

cd - >/dev/null
