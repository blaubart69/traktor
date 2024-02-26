cmake_minimum_required(VERSION 3.15)

set(PROJECT bench)

project(${PROJECT} LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
    src/calc_baseline/highway.cc
    src/calc_baseline/test_highway.cpp
    /home/bee/dev/highway/hwy/print.cc
    /home/bee/dev/highway/hwy/targets.cc
    # ../traktor/src/calculate.cpp
)

add_executable(${PROJECT} ${SOURCES})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

target_include_directories(${PROJECT} PRIVATE "/home/bee/dev/highway")

target_compile_features(${PROJECT} PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

#target_compile_options(${PROJECT} PRIVATE -Wall -Wextra -Wpedantic -masm=intel -save-temps=obj -march=native)
target_compile_options(${PROJECT} PRIVATE -mavx2 -march=native)
# -march=native

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_VERSION}")