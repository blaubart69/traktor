#include <cstdio>
#include <cstddef>
#include <chrono>

#include "calc.h"

void print_result(const char* impl_name, std::chrono::_V2::system_clock::rep duration_ns,
    const Result& result)
{
    printf("%25s\t%6.0f ms\tpoints: %12lu\tdelta_pixels_sum: %10ld\tin: %lu\tout: %lu\n", 
        impl_name
        , ( (float)duration_ns / 1000000 )
        , result.points, result.delta_pixels_sum, result.in_range, result.out_range );
}

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
}