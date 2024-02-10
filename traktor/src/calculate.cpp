#include <cmath>

#include "calculate.h"
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
float project_x_onto_baseline(const CoordPoint point, const int y_baseline) 
{
    if ( point.x == 0 ) 
    {
        // point is on the middle line
        // projected to the baseline: x = 0
        return 0;
    }
    else if ( point.y == 0) 
    {
        // point is on the same y as the Fluchtpunkt
        // projecting to the baseline is not possible.
        // Hoff ma's...
        // ToDo: wos is mit point.y < 0???
        //   ein Punkt hinter dem Fluchtpunkt.
        //   Kaun NIE sei...
        return std::numeric_limits<float>::infinity();
    }
    else
    {
        const float steigung = (float)point.y / (float)point.x;
        const float x_base_line = (float)y_baseline / steigung;
        return x_base_line;
    }
}

int apply_offset(int x, int offset)
{
    int x_offset =  x - offset;
    return x_offset;
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

int delta_from_nearest_refline(const int x, const unsigned int refline_distance)
{
    int half_refline = refline_distance / 2;

    int delta_pixel;
    if ( std::abs(x) < half_refline )
    {
        delta_pixel = x;
    }
    else 
    {
        int v = ( x > 0 ) ? -refline_distance : refline_distance;
        delta_pixel = x + v;
    }

    return delta_pixel;
}

