#!/bin/sh

SOURCES="src/main.cpp src/calc.cpp"

clang++  -O3 $SOURCES    -o calc_clang
g++-13   -O3 $SOURCES    -o calc_gcc

clang++  -O3 -S src/calc.cpp -o calc_clang.s
g++-13   -O3 -S src/calc.cpp -o calc_gcc.s

clang++  -O3 -S src/calc_one.cpp -o calc_one_clang.s
g++-13   -O3 -S src/calc_one.cpp -o calc_one_gcc.s

MCA_OPTS="-mcpu=znver2 -timeline --timeline-max-cycles=0 -iterations=1"
llvm-mca $MCA_OPTS calc_gcc.s   >mca_calc_gcc.txt
llvm-mca $MCA_OPTS calc_clang.s >mca_calc_clang.txt

llvm-mca $MCA_OPTS calc_one_gcc.s   >mca_calc_one_gcc.txt
llvm-mca $MCA_OPTS calc_one_clang.s >mca_calc_one_clang.txt


