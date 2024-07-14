#pragma once

struct Result {
    int64_t delta_pixels_sum = 0;
    size_t in_range = 0;
    size_t out_range = 0;
    size_t points = 0;
};

struct CalcSettings
{
    const int16_t x_half;
    const int16_t rowPerspectivePx;
    const int16_t y_baseline;
    const int16_t offset;
    const int16_t refline_distance;
    const int16_t half_refline_distance;
    const int16_t range_baseline;

    CalcSettings(int16_t x_half, int16_t y_screen_size, int16_t rowPerspectivePx, int16_t refline_distance, int16_t offset, int16_t row_count) 
    :   x_half(x_half)
    ,   rowPerspectivePx(rowPerspectivePx)
    ,   y_baseline(rowPerspectivePx + y_screen_size)
    ,   offset(offset)
    ,   refline_distance(refline_distance)
    ,   half_refline_distance(refline_distance / 2)
    ,   range_baseline( ( refline_distance * row_count ) + (refline_distance / 2) )
    {}
};

template<typename fnCalcImpl>
Result run_a_calc(size_t frames, int rows, fnCalcImpl calc_impl)
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

    #define N 64

    int16_t v_x[ N ] __attribute__ ((aligned (128)));
    int16_t v_y[ N ] __attribute__ ((aligned (128)));

    int16_t v_idx = 0;

    int64_t delta_pixels_sum = 0;
    size_t in_range = 0;
    size_t out_range = 0;
    size_t points = 0;

    for ( size_t i=0; i < frames; i++) 
    {
        //run_baseline_impl(screen_width, screen_height, &delta_pixels_sum, &in_range, &out_range, &points, calcSettings, baselineImpl);
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

                    for ( int v=0; v<N; v++) {
                        int32_t delta_pixels;
                        if ( ! calc_impl(v_x[v], v_y[v], calcSettings, &delta_pixels) )
                        {
                            out_range += 1;
                        }
                        else
                        {
                            in_range += 1;
                            delta_pixels_sum += delta_pixels;
                        }
                        points += 1;
                    }
                }
            }
        }
    }

    Result result;
    result.delta_pixels_sum = delta_pixels_sum;
    result.in_range = in_range;
    result.out_range = out_range;
    result.points = points; 

    return result;
}

bool calc_v1_simple   (const int16_t x_screen, const int16_t y_screen, const CalcSettings& settings, int *delta_pixels);
bool calc_v2_float_mul(const int16_t x_screen, const int16_t y_screen, const CalcSettings& settings, int *delta_pixels);
bool calc_v3_short_int(const int16_t x_screen, const int16_t y_screen, const CalcSettings& settings, int *delta_pixels);
