#!/usr/bin/env bash
#
# $1: executable
# $2: test dir

executable=$1
test_dir=$2

tests=`find $test_dir/* -maxdepth 0 -type d`

for i in $tests; do
    got=$($executable -a $i/a -b $i/b 2>/dev/null)
    expected=$(cat $i/res)
    if [[ "$got" = "$expected" ]]; then
        #echo ---------------------
        echo PASSED: testcase `basename $i`
        #echo
    else
        echo
        echo ---------------------
        echo FAILED: testcase `basename $i`
        echo
        echo factor a:
        echo $(cat $i/a)
        echo
        echo factor b:
        echo $(cat $i/b)
        echo
        echo expected result:
        echo $expected
        echo
        echo but got:
        echo $got
        false
    fi
done && echo SUCCESS || exit 0
