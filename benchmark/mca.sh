#!/bin/bash

run() {
    local CPU=$1
    local FILE=$2

    filename=$(basename -- "$FILE")
    extension="${filename##*.}"
    filename="${filename%.*}"

    local OUT="cmp/mca_${filename}_${CPU}.txt"
    llvm-mca -mcpu=$CPU -iterations=1 -timeline --timeline-max-cycles=0 --all-views $FILE > $OUT
    echo "out: ${OUT}"
}

CPU=znver2
run $CPU cmp/baseline_simple_clang.s 
run $CPU cmp/baseline_simple_gcc.s
