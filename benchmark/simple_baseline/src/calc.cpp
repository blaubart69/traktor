#include <cmath>
#include <cstddef>

#include "calc.h"

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


#define PRIVATE static

struct CoordPoint {
    int x;
    int y;
};

//
// 0,0 ... fluchtpunkt
//
PRIVATE CoordPoint project_screen_point_to_coord(const int x_point_screen, const int y_point_screen, const int x_half, const int rowPerspectivePx) 
{
    CoordPoint p;
    p.x = x_point_screen - x_half;
    p.y = rowPerspectivePx + y_point_screen;
    return p;
}
//
//  Fluchtpunkt: y=0
//  baseline:    line on the lower end of the screen
//  y_baseline:  vertical distance from Fluchtpunkt to lower end of the screen.
//               screen height + y_fluchtpunkt
//
PRIVATE bool project_x_onto_baseline(const CoordPoint point, const int y_baseline, int *x_base_line) 
{
    if ( point.x == 0 ) 
    {
        // point is on the middle line
        // projected to the baseline: x = 0
        *x_base_line = 0;
    }
    else if ( point.y == 0) 
    {
        // point is on the same y as the Fluchtpunkt
        // projecting to the baseline is not possible.
        // Hoff ma's...
        // ToDo: wos is mit point.y < 0???
        //   ein Punkt hinter dem Fluchtpunkt.
        //   Kaun NIE sei...
        return false;
    }
    else
    {
        const float steigung = (float)point.y / (float)point.x;
        *x_base_line = (int) ( (float)y_baseline / steigung );
    }

    return true;
}

PRIVATE int apply_offset(int x, int offset)
{
    int x_offset =  x - offset;
    return x_offset;
}

PRIVATE bool is_within_range(int x, const unsigned int range_baseline)
{
    return (unsigned int)std::abs(x) < range_baseline;
}

PRIVATE int project_x_inbetween_first_row(const int x_baseline, const int refline_distance) 
{
    if ( refline_distance == 0)
    {
        // 2024-02-10 Spindler
        // don't know if this makes sense.
        // refline_distance == 0 means there are no reflines?
        return x_baseline;
    }
    else
    {
        return x_baseline % (int)refline_distance;
    }
}

PRIVATE int distance_to_nearest_refline_on_baseline(const int x_first_row, const int refline_distance, const int half_refline_distance)
{
    if ( x_first_row >=  half_refline_distance ) return x_first_row - refline_distance;
    if ( x_first_row <= -half_refline_distance ) return x_first_row + refline_distance;
    return x_first_row;
}

PRIVATE bool calc_simple(const int x_screen, const int y_screen, const CalcSettings& settings, int *delta_pixels) 
{
    CoordPoint p;
    p = project_screen_point_to_coord(x_screen,y_screen, settings.x_half, settings.rowPerspectivePx);

    int x_baseline;
    if ( ! project_x_onto_baseline(p, settings.y_baseline, &x_baseline ) ) 
    {
        return false;
    }

    const int x_baseline_offset = apply_offset(x_baseline, settings.offset);

    if ( ! is_within_range(x_baseline_offset, settings.range_baseline) )
    {
        return false;
    }

    const int x_baseline_first_row = project_x_inbetween_first_row(x_baseline_offset, settings.refline_distance);

    *delta_pixels = distance_to_nearest_refline_on_baseline(x_baseline_first_row, settings.refline_distance, settings.half_refline_distance);

    return true;
}


Result calc_all_points(size_t frames, int rows)
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

    int32_t v_x[ N ] __attribute__ ((aligned (128)));
    int32_t v_y[ N ] __attribute__ ((aligned (128)));

    int16_t v_idx = 0;

    int64_t delta_pixels_sum = 0;
    size_t in_range = 0;
    size_t out_range = 0;
    size_t points = 0;

    for ( size_t i=0; i < frames; i++) 
    {
        //run_baseline_impl(screen_width, screen_height, &delta_pixels_sum, &in_range, &out_range, &points, calcSettings, baselineImpl);
        for ( int x=0; x<screen_width;x++) 
        {
            for ( int y=0; y<screen_height;y++) 
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
                        if ( ! calc_simple(v_x[v], v_y[v], calcSettings, &delta_pixels) )
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
