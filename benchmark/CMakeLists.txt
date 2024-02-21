cmake_minimum_required(VERSION 3.15)

set(PROJECT bench)

project(${PROJECT} LANGUAGES CXX)

set(SOURCES 
    src/bench.cpp
    src/calc_baseline/calc_baseline.h
    src/calc_baseline/calc_baseline.cpp
    src/calc_baseline/calc_baseline_int.cpp
    src/calc_baseline/calc_baseline_short_int.cpp
    src/calc_baseline/calc_baseline_float_mul.cpp
    src/calc_baseline/calc_baseline_v16.cpp
    src/calc_classic/calc_classic.h
    src/calc_classic/calc_classic.cpp
    # ../traktor/src/calculate.cpp
)

add_executable(${PROJECT} ${SOURCES})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

target_compile_features(${PROJECT} PRIVATE cxx_std_17)

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_VERSION}")