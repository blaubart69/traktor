#include <cmath>

#include "calc_baseline.h"

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
    /*if ( point.x == 0 ) 
    {
        // point is on the middle line
        // projected to the baseline: x = 0
        *x_base_line = 0;
    }
    else */if ( point.y == 0) 
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
        *x_base_line = (int )( (float)y_baseline * (float)point.x ) / (float)point.y;
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

bool calc_baseline_delta_from_nearest_refline_float_mul(const int x_screen, const int y_screen, const CalcSettings& settings, int *delta_pixels)
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
