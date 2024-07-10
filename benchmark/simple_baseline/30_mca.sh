#!/bin/sh

MCA_OPTS="-mcpu=znver2 -timeline --timeline-max-cycles=0 -iterations=100"
llvm-mca $MCA_OPTS gcc_calc.s   >mca_gcc.txt
llvm-mca $MCA_OPTS clang_calc.s >mca_clang.txt
