#!/bin/bash

run_one_build() {
    export CC=$1
    export CXX=$2
    local BUILDDIR=$3

    mkdir -p $BUILDDIR

    echo -e "+++\n+++ CC ($CC), CXX ($CXX), BUILDDIR ($BUILDDIR)\n+++" 
    rm -rf $BUILDDIR/*
    cmake -S . -B $BUILDDIR -DCMAKE_BUILD_TYPE=Release 
    ( cd $BUILDDIR &&  make -j`nproc` \
        && make src/calc_baseline/highway.s \
        && make src/calc_baseline/calc_baseline.s \
        )
    cp $BUILDDIR/CMakeFiles/bench.dir/src/calc_baseline/highway.cc.s         ./cmp/highway_$CC.s
    cp $BUILDDIR/CMakeFiles/bench.dir/src/calc_baseline/calc_baseline.cpp.s  ./cmp/baseline_V0_$CC.s
    cp $BUILDDIR/bench ./cmp/bench_${CC}
}

build() {
    local C=$1
    local CPP=$2

    BUILDDIR="./build/$C"

    run_one_build $C $CPP $BUILDDIR
}

build "gcc"   "g++"
build "clang" "clang++"

#mkdir -p ./build/gcc
#mkdir -p ./build/clang
#
#(   export  CC=/usr/bin/gcc \
#&&  export CXX=/usr/bin/g++ \
#&&        rm -rf ./build/gcc/*  \
#&& cmake -S . -B ./build/gcc -DCMAKE_BUILD_TYPE=Release \
#&& cd ./build/gcc && make -j12 )
#
#
#( export CC=/usr/bin/clang    \
#  export CXX=/usr/bin/clang++ \
#&&  rm -rf        ./build/clang/* \
#&&  cmake -S . -B ./build/clang -DCMAKE_BUILD_TYPE=Release \
#&&  cd ./build/clang && make -j12 )
