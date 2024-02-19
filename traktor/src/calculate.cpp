#include <cmath>

#include "detect.h"
#include "calculate.h"

#define PRIVATE static

/*
struct RefLineSettings 
{
    int row_count;
    int x_half;
    int y_screen_size;
    int y_flucht;
    int y_baseline = y_flucht + y_screen_size;
    int offset;
    int refline_distance;
    int half_refline_distance;
    //unsigned int range_baseline = ( refline_distance * row_count ) + half_refline_distance;
    unsigned int range_baseline;
};
*/

struct CalcSettings
{
    const int x_half;
    const int y_fluchtpunkt;
    const int y_baseline;
    const int offset;
    const unsigned int refline_distance;
    const unsigned int half_refline_distance;
    const unsigned int range_baseline;

    CalcSettings(int x_half, int y_screen_size, int y_fluchtpunkt, int refline_distance, int offset, int row_count) 
    :   x_half(x_half)
    ,   y_fluchtpunkt(y_fluchtpunkt)
    ,   y_baseline(y_fluchtpunkt + y_screen_size)
    ,   offset(offset)
    ,   refline_distance(refline_distance)
    ,   half_refline_distance(refline_distance / 2)
    ,   range_baseline( ( refline_distance * row_count ) + (refline_distance / 2) )
    {}
};

//
// 0,0 ... fluchtpunkt
//
PRIVATE CoordPoint project_screen_point_to_coord(const int x_point_screen, const int y_point_screen, const int x_half, const int y_flucht) 
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

PRIVATE int project_x_inbetween_first_row(const int x_baseline, const unsigned int refline_distance) 
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

PRIVATE int distance_to_nearest_refline_on_baseline(const int x_first_row, const int refline_distance, const int half_refline_distance)
{
    if ( x_first_row >=  half_refline_distance ) return x_first_row - refline_distance;
    if ( x_first_row <= -half_refline_distance ) return x_first_row + refline_distance;
    return x_first_row;
}

PRIVATE bool calc_delta_from_nearest_refline(const int x_screen, const int y_screen, const CalcSettings& settings, int *delta_pixels)
{
    CoordPoint p;
    p = project_screen_point_to_coord(x_screen,y_screen, settings.x_half, settings.y_fluchtpunkt);

    int x_baseline;
    if ( ! project_x_onto_baseline(p, settings.y_baseline, &x_baseline ) ) 
    {
        return false;
    }

    x_baseline = apply_offset(x_baseline, settings.offset);

    if ( ! is_within_range(x_baseline, settings.range_baseline) )
    {
        return false;
    }

    x_baseline = project_x_inbetween_first_row(x_baseline, settings.refline_distance);

    *delta_pixels = distance_to_nearest_refline_on_baseline(x_baseline, settings.refline_distance, settings.half_refline_distance);

    return true;
}

bool calc_average_delta(const ReflinesSettings& refSettings, const int frame_rows, Contoures* contoures, int* avg_deltaPx)
{
    CalcSettings calcSettings(
        refSettings.x_half
        , frame_rows
        , refSettings.y_fluchtpunkt
        , refSettings.rowSpacingPx
        , refSettings.offset
        , refSettings.get_half_row_count()
    );

    int  sum_deltaPx = 0;
    uint plants_processed = 0;

    for ( std::size_t i=0; i < contoures->centers.size(); ++i )
    {
        Center& plant = contoures->centers[i];
        // hier bitte mit Magie befüllen!

        int deltaPx;
        if ( ! calc_delta_from_nearest_refline(plant.point.x, plant.point.y, calcSettings, &deltaPx) )
        {
            // out of range
        }
        else 
        {
            plant.within_threshold = std::abs(deltaPx) <=   refSettings.rowThresholdPx;
            plant.within_row_range = std::abs(deltaPx) <= ( refSettings.rowThresholdPx + refSettings.rowRangePx );

            if ( ! plant.within_row_range )
            {
                // plant is out of range AND also out of threshold:
                // plant is skipped. not used for average.
            }
            else
            {
                plants_processed += 1;
                sum_deltaPx += deltaPx;
            }
        }
    }
    if ( plants_processed > 0 ) {
        *avg_deltaPx = sum_deltaPx / plants_processed;
    }

    return plants_processed > 0;
}
