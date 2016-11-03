#!/bin/bash

set -euo pipefail

tmpdir=$(mktemp -d -t dcscat.XXXX)

trap "rm -rf $tmpdir" EXIT

function dotest() {
    size=${1:-1024}
    file=${tmpdir}/data
    out=${tmpdir}/out

    # Get data
    head -c ${size} /dev/urandom >$file

    exptsha=$(sha1sum < ${file} | awk '{print $1}')

    ./dcscat $out <${file}

    gotsha=$(sha1sum <${out} | awk '{print $1}')

    test "$exptsha" == "$gotsha"
    echo "PASS:  $size"
}

size=1024
while [[ $size -le $(( 8 * 1024 * 1024)) ]]
do
    dotest $size
    size=$(($size * 2))
done
rm -rf $tmpdir
