#!/usr/bin/env bash

set -e

MYDIR="$(cd "$(dirname "$0")"; pwd -P)"
progs_file=${MYDIR}/programs_to_test.txt
progs=$(cat "${progs_file}")

set +e

failed_progs=()
for prog in ${progs[@]};do
    echo "Running: $prog -h"
    $prog -h > /dev/null
    if [ $? -ne 0 ];then
        failed_progs+=($prog)
    fi
done

echo
echo "Total failed programs: ${#failed_progs[@]}"
for prog in ${failed_progs[@]};do
    echo ${prog}
done

if [ ${#failed_progs[@]} -ne 0 ];then
    exit 1
fi
