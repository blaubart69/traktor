#include <cmath>

#include "detect.h"
#include "stats.h"

#define PRIVATE static

struct CalcSettingsShort
{
    const int16_t x_half;
    const int16_t rowPerspectivePx;
    const int16_t y_baseline;
    const int16_t offset;
    const int16_t refline_distance;
    const int16_t half_refline_distance;
    const int16_t range_baseline;

    CalcSettingsShort(int16_t x_half, int16_t y_screen_size, int16_t rowPerspectivePx, int16_t refline_distance, int16_t offset, int16_t half_row_count) 
    :   x_half(x_half)
    ,   rowPerspectivePx(rowPerspectivePx)
    ,   y_baseline(rowPerspectivePx + y_screen_size)
    ,   offset(offset)
    ,   refline_distance(refline_distance)
    ,   half_refline_distance(refline_distance / 2)
    ,   range_baseline( (int16_t)( refline_distance * half_row_count ) + (refline_distance / 2) )
    {}
};

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

PRIVATE int16_t project_x_inbetween_first_row(const int16_t x_baseline, const int16_t refline_distance) 
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

PRIVATE bool calc_baseline_delta_from_nearest_refline_short_int(const int16_t x_screen, const int16_t y_screen, const CalcSettingsShort& settings, int *delta_pixels)
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
    const int16_t x_baseline_within_range = (int16_t)x_baseline_offset;
    const int16_t x_baseline_first_row = project_x_inbetween_first_row(x_baseline_within_range, settings.refline_distance);

    *delta_pixels = distance_to_nearest_refline_on_baseline(x_baseline_first_row, settings.refline_distance, settings.half_refline_distance);

    return true;
}

bool calc_average_delta_short_int(const ReflinesSettings& refSettings, const int frame_rows, Contoures* contoures, int* avg_deltaPx, DetectCounter* stats)
{
    CalcSettingsShort calcSettings(
          (int16_t)refSettings.x_half
        , (int16_t)frame_rows
        , (int16_t)refSettings.rowPerspectivePx
        , (int16_t)refSettings.rowSpacingPx
        , (int16_t)refSettings.offset
        , (int16_t)refSettings.get_half_row_count()
    );

    int sum_deltaPx = 0;
    int plants_processed = 0;

    for ( std::size_t i=0; i < contoures->centers.size(); ++i )
    {
        Center& plant = contoures->centers[i];
        // hier bitte mit Magie befüllen!

        int deltaPx;
        if ( ! calc_baseline_delta_from_nearest_refline_short_int(plant.point.x, plant.point.y, calcSettings, &deltaPx) )
        {
            plant.within_threshold = false;
            plant.within_row_range = false;
            stats->plants_out_range.fetch_add(1, std::memory_order_relaxed);
        }
        else 
        {
            plant.within_threshold = std::abs(deltaPx) <=   refSettings.rowThresholdPx;
            if ( refSettings.rowRangePx == 0)   
            {
                // rowRange is disabled
                plant.within_row_range = true;
            }
            else
            {
                plant.within_row_range = std::abs(deltaPx) <= ( refSettings.rowThresholdPx + refSettings.rowRangePx );
            }

            if ( ! plant.within_row_range )
            {
                // plant is out of range AND also out of threshold:
                // plant is skipped. not used for average.
                stats->plants_out_range.fetch_add(1, std::memory_order_relaxed);
            }
            else
            {
                plants_processed += 1;
                sum_deltaPx += deltaPx;
                
                if ( std::abs(deltaPx) <= refSettings.rowThresholdPx )
                {
                    stats->plants_in_tolerance.fetch_add(1, std::memory_order_relaxed);
                }
                else
                {
                    stats->plants_out_tolerance.fetch_add(1, std::memory_order_relaxed);
                }
            }
        }
    }
    if ( plants_processed > 0 ) {
        *avg_deltaPx = sum_deltaPx / plants_processed;
        //printf("calc_average_delta: plants_processed: %d, sum_deltaPx: %d avg_deltaPx: %d\n", plants_processed, sum_deltaPx, *avg_deltaPx);
    }

    stats->plants_in_picture.fetch_add((uint32_t)contoures->centers.size());

    return plants_processed > 0;
}

