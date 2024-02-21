cmake_minimum_required(VERSION 3.20)

set(PROJECT bench)

project(${PROJECT} LANGUAGES CXX)

set(SOURCES 
    src/bench.cpp
    src/calc_baseline/calc_baseline.h
    src/calc_baseline/calc_baseline.cpp
    src/calc_baseline/calc_baseline_without_float.cpp
    src/calc_baseline/calc_baseline_float_mul.cpp
    src/calc_classic/calc_classic.h
    src/calc_classic/calc_classic.cpp
    # ../traktor/src/calculate.cpp
)

add_executable(${PROJECT} ${SOURCES})

target_compile_features(${PROJECT} PRIVATE cxx_std_14)