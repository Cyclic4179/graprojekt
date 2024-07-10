#!/usr/bin/env bash
#
# $1: executable

executable=$1

tests=`find * -maxdepth 0 -type d -not -name 'generated'`

for i in $tests; do
    got=$($executable -a $i/a -b $i/b 2>/dev/null)
    expected=$(cat $i/res)
    if [[ "$got" = "$expected" ]]; then
        echo PASSED: testcase `basename $i`
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
        failed=1
    fi
done

if test -z "$failed"; then
    echo SUCCESS
fi
