#include <cstdio>
#include <chrono>

#include "calc_classic/calc_classic.h"
#include "calc_baseline/calc_baseline.h"

void print_result(const char* impl_name, std::chrono::_V2::system_clock::rep duration_ns,
    size_t points, int32_t delta_pixels_sum, size_t in_range, size_t out_range)
{
    printf("%25s: %11.3f ms, points: %12lu, delta_pixels_sum: %10d, in: %lu, out: %lu\n", 
        impl_name
        , ( (float)duration_ns / 1000000 )
        , points, delta_pixels_sum, in_range, out_range );
}

void run_classic(int rows, float *delta_pixels_sum, size_t *in_range, size_t *out_range, size_t *points)
{
    const int screen_width = 960;
    const int screen_height = 720;

    ReflinesSettings ref;
    ref.offset = 0;
    ref.rowPerspectivePx = 0;
    ref.rowSpacingPx = 160;
    ref.rowRangePx = 0;
    ref.rowThresholdPx = 0;
    ref.x_half = screen_width / 2;
    ref.y_fluchtpunkt = screen_height + ref.rowPerspectivePx;
    ref.rowMax = 0;
    ref.half_row_count_auto = rows;

    Point p;
    for ( int x=0; x<screen_width;x++) 
    {
        for ( int y=0; y<screen_height;y++) 
        {
            p.x = x - ref.x_half;
            p.y = y;
            float nearest_refLine_x;
            float deltaPx;
            float refLines_distance_px;
            if ( ! classic_find_point_on_nearest_refline(p, ref, &nearest_refLine_x, &deltaPx, &refLines_distance_px ) )
            {
                *out_range += 1;
            }
            else
            {
                *in_range += 1;
                *delta_pixels_sum += deltaPx;
            }
            *points += 1;
        }
    }
}
void bench_classic(const char* impl_name,size_t frames, int rows)
{
    auto start = std::chrono::high_resolution_clock::now();

    float delta_pixels_sum = 0;
    size_t in_range = 0;
    size_t out_range = 0;
    size_t points = 0;

    for (size_t i=0; i < frames; i++) 
    {
        run_classic(rows, &delta_pixels_sum, &in_range, &out_range, &points);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( end - start ).count();

    print_result(impl_name, duration, points, delta_pixels_sum, in_range, out_range);
}

//typedef bool (*pfBaselineImpl)(int,int, const CalcSettings&, int*);

template<typename S, typename pfImpl>
void run_baseline_impl(
    int screen_width, int screen_height,
    int *delta_pixels_sum, size_t *in_range, size_t *out_range, size_t *points, 
    const S& calcSettings, pfImpl baselineImpl)
{
    for ( int x=0; x<screen_width;x++) 
    {
        for ( int y=0; y<screen_height;y++) 
        {
            int delta_pixels;
            if ( ! baselineImpl(x, y, calcSettings, &delta_pixels) )
            {
                *out_range += 1;
            }
            else
            {
                *in_range += 1;
                *delta_pixels_sum += delta_pixels;
            }
            *points += 1;
        }
    }
}
template<typename tySettings, typename pfImpl>
void bench_a_baseline(const char* impl_name, size_t frames, int rows, pfImpl baselineImpl)
{
    const int screen_width = 960;
    const int screen_height = 720;

    tySettings calcSettings(
          screen_width / 2
        , screen_height
        , 1     // refSettings.rowPerspectivePx
        , 160   // refSettings.rowSpacingPx
        , 0     // refSettings.offset
        , rows     // row_count
    );

    auto start = std::chrono::high_resolution_clock::now();

        int delta_pixels_sum = 0;
        size_t in_range = 0;
        size_t out_range = 0;
        size_t points = 0;

        for ( size_t i=0; i < frames; i++) 
        {
            run_baseline_impl(screen_width, screen_height, &delta_pixels_sum, &in_range, &out_range, &points, calcSettings, baselineImpl);
        }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( end - start ).count();

    print_result(impl_name, duration, points, delta_pixels_sum, in_range, out_range);
}

template<typename tySettings, typename pfImpl>
void bench_a_baseline_loop(const char* impl_name, size_t frames, int rows, pfImpl baselineImpl)
{
    const int screen_width = 960;
    const int screen_height = 720;

    tySettings calcSettings(
          screen_width / 2
        , screen_height
        , 1     // refSettings.rowPerspectivePx
        , 160   // refSettings.rowSpacingPx
        , 0     // refSettings.offset
        , rows     // row_count
    );

    auto start = std::chrono::high_resolution_clock::now();

        int delta_pixels_sum = 0;
        size_t in_range = 0;
        size_t out_range = 0;
        size_t points = 0;

        baselineImpl(frames, screen_width, screen_height, &delta_pixels_sum, &in_range, &out_range, &points, calcSettings);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( end - start ).count();

    print_result(impl_name, duration, points, delta_pixels_sum, in_range, out_range);
}


void bench_v16(const char* impl_name, size_t frames, int rows)
{
    const int screen_width = 960;
    const int screen_height = 720;

    CalcSettingsShort calcSettings(
          screen_width / 2
        , screen_height
        , 1     // refSettings.rowPerspectivePx
        , 160   // refSettings.rowSpacingPx
        , 0     // refSettings.offset
        , rows     // row_count
    );

    auto start = std::chrono::high_resolution_clock::now();

        int delta_pixels_sum = 0;
        size_t in_range = 0;
        size_t out_range = 0;
        size_t points = 0;

        
        int16_t v_x[ 16 ];
        int16_t v_y[ 16 ];

        uint8_t v_idx = 0;

        for ( size_t i=0; i < frames; i++) 
        {
            for ( int16_t x=0; x<screen_width;x++) 
            {
                for ( int16_t y=0; y<screen_height;y++) 
                {
                    if ( v_idx < 16 )
                    {
                        v_x[v_idx] = x;
                        v_y[v_idx] = y;
                        v_idx++;
                    }
                    if ( v_idx == 16 )
                    {
                        v_idx = 0;

                        int delta_pixels;
                        bool ok[ 16 ] = {true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true};
                        calc_delta_from_nearest_refline_16(v_x, v_y, calcSettings, &delta_pixels, ok );
                        delta_pixels_sum += delta_pixels;

                        for ( int z=0; z < 16; z++)
                        {
                            if ( ok[ z ] )  { in_range++; }
                            else            { out_range++; }
                        }
                        points += 16;
                    }
                }
            }
        }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( end - start ).count();

    print_result(impl_name, duration, points, delta_pixels_sum, in_range, out_range);
}

template<typename T, typename pfImpl>
void bench_highway(const char* impl_name, size_t frames, int rows, pfImpl highwayImpl)
{
    const int screen_width = 960;
    const int screen_height = 720;

    CalcSettings calcSettings(
          screen_width / 2
        , screen_height
        , 1     // refSettings.rowPerspectivePx
        , 160   // refSettings.rowSpacingPx
        , 0     // refSettings.offset
        , rows     // row_count
    );

    auto start = std::chrono::high_resolution_clock::now();

        #define N 64

        int delta_pixels_sum = 0;
        size_t in_range = 0;
        size_t out_range = 0;
        size_t points = 0;

        T v_x[ N ] __attribute__ ((aligned (128)));
        T v_y[ N ] __attribute__ ((aligned (128)));

        uint8_t v_idx = 0;

        for ( size_t i=0; i < frames; i++) 
        {
            for ( int16_t x=0; x<screen_width;x++) 
            {
                for ( int16_t y=0; y<screen_height;y++) 
                {
                    if ( v_idx < N )
                    {
                        v_x[v_idx] = x;
                        v_y[v_idx] = y;
                        v_idx++;
                    }
                    if ( v_idx == N )
                    {
                        v_idx = 0;

                        int delta_pixels;
                        //int32_t valid_px = deltapx::run_hwy_calc_delta_pixels(N, v_x, v_y, calcSettings, &delta_pixels);
                        int32_t valid_px = highwayImpl(N, v_x, v_y, calcSettings, &delta_pixels);
                        in_range  +=      valid_px;
                        out_range += (N - valid_px);
                        delta_pixels_sum += delta_pixels;

                        points += N;
                    }
                }
            }
        }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( end - start ).count();

    print_result(impl_name, duration, points, delta_pixels_sum, in_range, out_range);
}


void test_hwy_calc_delta_pixels();

//int main(int argc, char* argv[])
int main()
{

    //test_hwy_calc_delta_pixels();
    //play::run();
    //return 0;

    size_t frames = 30;
    int rows = 3;

    for ( int i=1; i <= 1; i++)
    {
        printf("--- %d. rows: %d, frames: %lu---\n", i, rows, frames);
        bench_classic                      ("Classic",               frames, rows);
        bench_a_baseline<CalcSettings>     ("baseline float (simple)", frames, rows, calc_baseline_delta_from_nearest_refline_simple);
        bench_a_baseline<CalcSettings>     ("baseline int (mul)",    frames, rows, calc_baseline_delta_from_nearest_refline_int);
        bench_a_baseline<CalcSettings>     ("baseline float mul",    frames, rows, calc_baseline_delta_from_nearest_refline_float_mul);
        bench_a_baseline<CalcSettingsShort>("baseline short int",    frames, rows, calc_baseline_delta_from_nearest_refline_short_int);
        bench_a_baseline<CalcSettingsFloat>("baseline only float",   frames, rows, calc_baseline_delta_from_nearest_refline_only_float);
        bench_v16                          ("baseline v16",          frames, rows);
        bench_highway<int32_t>             ("highway int32",         frames, rows, deltapx::run_hwy_calc_delta_pixels_int32);
        bench_highway<int32_t>             ("highway int32 only",    frames, rows, deltapx::run_hwy_calc_delta_pixels_int32_only);
        bench_highway<int16_t>             ("highway int16",         frames, rows, deltapx::run_hwy_calc_delta_pixels_int16);
        
        bench_a_baseline_loop<CalcSettings>("baseline simple loop",  frames, rows, calc_baseline_full_loop);
    }
    printf("=====\n");
    deltapx::print_target();

    return 0;
}