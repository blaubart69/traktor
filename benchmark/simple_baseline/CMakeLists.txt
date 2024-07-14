cmake_minimum_required(VERSION 3.25)

set(PROJECT bench_baseline)

project(${PROJECT} LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES 
    src/main.cpp
    src/calc.h
    src/v1_simple.cpp
    src/v2_float_mul.cpp
    src/v3_short_int.cpp
)

add_executable(${PROJECT} ${SOURCES})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_compile_features(${PROJECT} PRIVATE cxx_std_17)
target_compile_definitions(${PROJECT} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

#
# GOOGLE BENCHMARK
#
# if you have Benachmark within your project:
#   add_subdirectory(benchmark)
#   target_link_libraries(${PROJECT}  benchmark::benchmark)
target_include_directories(${PROJECT} PRIVATE "/home/bee/dev/benchmark/include"   )
target_link_directories   (${PROJECT} PRIVATE "/home/bee/dev/benchmark/build/src" ) 
target_link_libraries     (${PROJECT} PRIVATE "benchmark" "benchmark_main"        ) # !!! don't have BENCHMARK_MAIN() within your programm !!!

#target_compile_options(${PROJECT} PRIVATE -Wall -Wextra -Wpedantic -masm=intel -save-temps=obj -march=native)
target_compile_options(${PROJECT} PRIVATE -march=native -mtune=native)
# -march=native
# -march=znver2
# -mavx2

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_VERSION}")