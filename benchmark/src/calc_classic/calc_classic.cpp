#include <cmath>

#include "calc_classic.h"

/*
 * y = x * ( 10 / 2 )   
 * x = y / ( 10 / 2 )
 * Wolfram Alpha: plot [y = (-x+2) * (10/2), {x,-2,2}]
 */
bool classic_find_point_on_nearest_refline(
      const Point&        plant
    , const ReflinesSettings& settings
    , float  *nearest_refLine_x
    , float  *deltaPx
    , float  *refLines_distance_px)
{
    const float plant_x_abs_offset_from_0  = std::abs( plant.x );

    float px_delta_to_row;
    float x_ref1 = 0;    // start with the middle
    float x_ref2;

    int refline_x = settings.rowSpacingPx;

    const int rows = 
        settings.get_half_row_count() 
        + 1;    // to check "outside" the last row

    const int last_row_idx = rows - 1;

    for ( int r=0; r < rows; ++r, refline_x += settings.rowSpacingPx)
    {
        const float refline_steigung = settings.y_fluchtpunkt / (float)refline_x;
        x_ref2 = -plant.y / refline_steigung ;
        x_ref2 +=  refline_x;

        if ( x_ref1 <= plant_x_abs_offset_from_0 && plant_x_abs_offset_from_0 < x_ref2 )
        {
            // plant is between rows. which row is closer?
            const float delta_to_row1_px = plant_x_abs_offset_from_0 - x_ref1;
            const float delta_to_row2_px = x_ref2 - plant_x_abs_offset_from_0;

            if ( delta_to_row1_px < delta_to_row2_px )
            {
                *nearest_refLine_x = x_ref1;
                *deltaPx = delta_to_row1_px;        // row 1 is left from the plant. delta is positive.
            }
            else
            {
                if ( r == last_row_idx )
                {
                    // 2023-06-06 Spindler
                    //  the plant is nearer to the "last outer row".
                    //  skip it
                    return false;
                }
                else
                {
                    *nearest_refLine_x = x_ref2;
                    *deltaPx = -delta_to_row2_px;       // row 2 is right from the plant. delta is negative.
                }
            }

            *refLines_distance_px = x_ref2 - x_ref1;

            if ( plant.x < 0 )
            {
                // plant is on the left side. Mirror the value
                *nearest_refLine_x = -*nearest_refLine_x;
                *deltaPx = -*deltaPx;
            }
            // shift it to the right pixel-image-value
            *nearest_refLine_x += settings.x_half;

            return true;
        }
        else
        {
            // try next rows
            x_ref1 = x_ref2;
        }
    }
    
    return false;
}

/*
bool calc_overall_threshold_draw_plants(const ReflinesSettings& refSettings, const int frame_rows, Contoures* contoures, float* avg_threshold)
{
    const float threshold_percent =   (float)refSettings.rowThresholdPx                                   / (float)refSettings.rowSpacingPx;
    const float rowRange_percent  = ( (float)refSettings.rowThresholdPx + (float)refSettings.rowRangePx ) / (float)refSettings.rowSpacingPx;

    float   sum_threshold = 0;
    cv::Point plant_coord;

    uint centers_processed = 0;
    for ( int i=0; i < contoures->centers.size(); ++i )
    {
        Center& plant = contoures->centers[i];
        // hier bitte mit Magie befüllen!

        plant_coord.x = plant.point.x - refSettings.x_half;
        plant_coord.y = frame_rows - plant.point.y;

        float nearest_refLine_x;
        float deltaPx;
        float refLines_distance_px;
        if ( find_point_on_nearest_refline(plant_coord, refSettings, &nearest_refLine_x, &deltaPx, &refLines_distance_px) )
        {
            const float threshold = deltaPx / refLines_distance_px;

            if ( refSettings.rowRangePx == 0)
            {
                sum_threshold += threshold;
                plant.within_threshold = std::abs(threshold) < threshold_percent;
                plant.within_row_range = true;
                centers_processed += 1;
            }
            else
            {
                plant.within_row_range = std::abs(threshold) < rowRange_percent;

                if ( plant.within_row_range )
                {
                    sum_threshold += threshold;
                    plant.within_threshold = std::abs(threshold) < threshold_percent;
                    centers_processed += 1;
                }
                else
                {
                    plant.within_threshold = false;
                }
            }
        }
    }
    if ( centers_processed > 0 ) {
        *avg_threshold = sum_threshold / (float)centers_processed;
    }

    return centers_processed > 0;
}
*/
