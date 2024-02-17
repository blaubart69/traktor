#include <cmath>

#include "calculate.h"

struct CoordPoint {
    int x;
    int y;
};

//
// 0,0 ... fluchtpunkt
//
CoordPoint project_screen_point_to_coord(const int x_point_screen, const int y_point_screen, const int x_half, const int y_flucht) 
{
    CoordPoint p;
    p.x = x_point_screen - x_half;
    p.y = y_flucht + y_point_screen;
    return p;
}
//
//  Fluchtpunkt: y=0
//  baseline:    line on the lower end of the screen
//  y_baseline:  vertical distance from Fluchtpunkt to lower end of the screen.
//               screen height + y_fluchtpunkt
//
bool project_x_onto_baseline(const CoordPoint point, const int y_baseline, int *x_base_line) 
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

int apply_offset(int x, int offset)
{
    int x_offset =  x - offset;
    return x_offset;
}

bool is_within_range(int x, const unsigned int range_baseline)
{
    return std::abs(x) < range_baseline;
}

int project_x_inbetween_first_row(const int x_baseline, const unsigned int refline_distance) 
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

int distance_to_nearest_refline_on_baseline(const int x_first_row, const unsigned int refline_distance, const unsigned int half_refline_distance)
{
    if ( x_first_row >=  half_refline_distance ) return x_first_row - refline_distance;
    if ( x_first_row <= -half_refline_distance ) return x_first_row + refline_distance;
    return x_first_row;
}

int calc_delta_from_nearest_refline(const int x_screen, const int y_screen, int *delta_pixels)
{
    const int row_count = 1;
    const int x_half = 640 / 2;
    const int y_screen_size = 480;
    const int y_flucht = 0;
    const int y_baseline = y_flucht + y_screen_size;
    const int offset = 5;
    const int refline_distance = 160;
    const int half_refline_distance = refline_distance / 2;
    const unsigned int range_baseline = ( refline_distance * row_count ) + half_refline_distance;

    CoordPoint p;
    p = project_screen_point_to_coord(340,100, x_half, y_flucht);

    int x_baseline;
    if ( ! project_x_onto_baseline(p, y_baseline, &x_baseline ) ) 
    {
        return false;
    }

    x_baseline = apply_offset(x_baseline, offset);

    if ( ! is_within_range(x_baseline, range_baseline) )
    {
        return false;
    }

    x_baseline = project_x_inbetween_first_row(x_baseline, refline_distance);

    *delta_pixels = distance_to_nearest_refline_on_baseline(x_baseline, refline_distance, half_refline_distance);

    return true;
}
