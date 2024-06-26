#include <cmath>

#include "calc_baseline.h"

#define PRIVATE static

struct CoordPoint {
    int16_t x;
    int16_t y;
};

//
// 0,0 ... fluchtpunkt
//
PRIVATE CoordPoint project_screen_point_to_coord(const int16_t x_point_screen, const int16_t y_point_screen, const int16_t x_half, const int16_t rowPerspectivePx) 
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
PRIVATE bool project_x_onto_baseline(const CoordPoint point, const int16_t y_baseline, int32_t *x_base_line) 
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
        //const float steigung = (float)point.y / (float)point.x;
        //*x_base_line = (int) ( (float)y_baseline / steigung );
        *x_base_line = ( y_baseline * point.x ) / point.y;
    }

    return true;
}

PRIVATE int32_t apply_offset(int32_t x, int16_t offset)
{
    int x_offset =  x - offset;
    return x_offset;
}

PRIVATE bool is_within_range(int32_t x, const int16_t range_baseline)
{
    return std::abs(x) < (int32_t)range_baseline;
}

PRIVATE int16_t project_x_inbetween_first_row(const int32_t x_baseline, const int32_t refline_distance) 
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
        return x_baseline % refline_distance;
    }
}

PRIVATE int distance_to_nearest_refline_on_baseline(const int16_t x_first_row, const int16_t refline_distance, const int16_t half_refline_distance)
{
    if ( x_first_row >=  half_refline_distance ) return x_first_row - refline_distance;
    if ( x_first_row <= -half_refline_distance ) return x_first_row + refline_distance;
    return x_first_row;
}

bool calc_baseline_delta_from_nearest_refline_short_int(const int16_t x_screen, const int16_t y_screen, const CalcSettingsShort& settings, int *delta_pixels)
{
    CoordPoint p;
    p = project_screen_point_to_coord(x_screen,y_screen, settings.x_half, settings.rowPerspectivePx);

    int32_t x_baseline;
    if ( ! project_x_onto_baseline(p, settings.y_baseline, &x_baseline ) ) 
    {
        return false;
    }

    const int32_t x_baseline_offset = apply_offset(x_baseline, settings.offset);

    if ( ! is_within_range(x_baseline_offset, settings.range_baseline) )
    {
        return false;
    }

    const int16_t x_baseline_first_row = project_x_inbetween_first_row(x_baseline_offset, settings.refline_distance);

    *delta_pixels = distance_to_nearest_refline_on_baseline(x_baseline_first_row, settings.refline_distance, settings.half_refline_distance);

    return true;
}
