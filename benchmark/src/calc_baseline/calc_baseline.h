#pragma once

struct CalcSettings
{
    const int x_half;
    const int rowPerspectivePx;
    const int y_baseline;
    const int offset;
    const unsigned int refline_distance;
    const unsigned int half_refline_distance;
    const unsigned int range_baseline;

    CalcSettings(int x_half, int y_screen_size, int rowPerspectivePx, int refline_distance, int offset, int row_count) 
    :   x_half(x_half)
    ,   rowPerspectivePx(rowPerspectivePx)
    ,   y_baseline(rowPerspectivePx + y_screen_size)
    ,   offset(offset)
    ,   refline_distance(refline_distance)
    ,   half_refline_distance(refline_distance / 2)
    ,   range_baseline( ( refline_distance * row_count ) + (refline_distance / 2) )
    {}
};

bool calc_baseline_delta_from_nearest_refline(const int x_screen, const int y_screen, const CalcSettings& settings, int *delta_pixels);
bool calc_baseline_delta_from_nearest_refline_int(const int x_screen, const int y_screen, const CalcSettings& settings, int *delta_pixels);
bool calc_baseline_delta_from_nearest_refline_float_mul(const int x_screen, const int y_screen, const CalcSettings& settings, int *delta_pixels);
