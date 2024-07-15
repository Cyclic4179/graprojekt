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
        echo run: testcase `basename $i`

        # hack to not trim trailing newline, capture stderr, stdout
        #{
        #    IFS=$'\n' read -r -d '' captured_stderr;
        #    IFS=$'\n' read -r -d '' got;
        #} < <((printf '\0%s\0' "$($cmd; echo suffix; (echo suffix 1>&2))" 1>&2) 2>&1)
        got=$($executable -a $i/a -b $i/b -V $v && echo suffix)
        if test "$?" -ne 0; then
            exit
        fi
        got=${got%suffix}
        captured_stderr=${captured_stderr%suffix}

        expected=$(cat $i/res ; echo suffix)
        expected=${expected%suffix}

        if error=$($executable -e"$maxerr" -a <(echo "$got") -b <(echo "$expected")); then
            echo " -> "PASSED
        else
            echo
            echo ---------------------
            echo FAILED
            echo

            #echo ran: $prettycmd
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
            echo

            echo stderr: ">>>>"
            echo "$captured_stderr<<<<"

            exit
        fi
    done
done

echo SUCCESS

cd - >/dev/null
