#pragma once

struct ReflinesSettings {

    public:
        int         rowMax;
        int         half_row_count_auto;
        
        int         rowSpacingPx;
        int         rowPerspectivePx;
        int         rowThresholdPx;
        int         rowRangePx;
        
        int         x_half;
        int         y_fluchtpunkt;
        int         offset;

        int get_half_row_count() const {
            if ( rowMax == 0) {
                return half_row_count_auto;
            }
            else {
                return rowMax;
            }
        }

    ReflinesSettings()
    {
        //rowCount           = 3;
        half_row_count_auto     = 1;
        rowMax             = 0;
        rowSpacingPx       = 160;
        rowPerspectivePx   = 0;
        rowThresholdPx     = 1;
        rowRangePx         = 0;

        y_fluchtpunkt      = 0;
        offset             = 0;
    }
};

struct Point {
    int x;
    int y;
};

bool classic_find_point_on_nearest_refline(
      const Point&        plant
    , const ReflinesSettings& settings
    , float  *nearest_refLine_x
    , float  *deltaPx
    , float  *refLines_distance_px);