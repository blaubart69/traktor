#include <cstdio>
#include <cstddef>
#include <chrono>

#include <benchmark/benchmark.h>

#include "calc.h"

/*
void print_result(const char* impl_name, std::chrono::_V2::system_clock::rep duration_ns,
    const Result& result)
{
    printf("%25s\t%6.0f ms\tpoints: %12lu\tdelta_pixels_sum: %10ld\tin: %lu\tout: %lu\n", 
        impl_name
        , ( (float)duration_ns / 1000000 )
        , result.points, result.delta_pixels_sum, result.in_range, result.out_range );
}*/

static void bm_v1_simple(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    run_a_calc(30,3,calc_v1_simple);
  }
}
static void bm_v2_float_mul(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    run_a_calc(30,3,calc_v2_float_mul);
  }
}
static void bm_v3_short_int(benchmark::State& state) {
  // Perform setup here
  for (auto _ : state) {
    // This code gets timed
    run_a_calc(30,3,calc_v3_short_int);
  }
}
// Register the function as a benchmark
BENCHMARK(bm_v1_simple);
BENCHMARK(bm_v2_float_mul);
BENCHMARK(bm_v3_short_int);

/*
int main()
{
    size_t frames = 30;
    int rows = 3;

    auto start = std::chrono::high_resolution_clock::now();

        Result result =
        calc_all_points(frames, rows);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( end - start ).count();

    print_result("calc simple", duration, result);
}*/