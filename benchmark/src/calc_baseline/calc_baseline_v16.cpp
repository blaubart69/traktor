#include <cmath>

#include "calc_baseline.h"

struct CoordPoint {
    int16_t x;
    int16_t y;
};

#define PRIVATE static

/*
struct CalcSettings
{
    const short int x_half;
    const short int rowPerspectivePx;
    const short int y_baseline;
    const short int offset;
    const unsigned short int refline_distance;
    const unsigned short int half_refline_distance;
    const unsigned short int range_baseline;

    CalcSettings(short int x_half, short int y_screen_size, short int rowPerspectivePx, short int refline_distance, short int offset, short int row_count) 
    :   x_half(x_half)
    ,   rowPerspectivePx(rowPerspectivePx)
    ,   y_baseline(rowPerspectivePx + y_screen_size)
    ,   offset(offset)
    ,   refline_distance(refline_distance)
    ,   half_refline_distance(refline_distance / 2)
    ,   range_baseline( ( refline_distance * row_count ) + (refline_distance / 2) )
    {}
};
*/
//
// 0,0 ... fluchtpunkt
//
PRIVATE void project_screen_point_to_coord_16(const short int * __restrict x_point_screen, const short int * __restrict y_point_screen, CoordPoint * __restrict point, const short int x_half, const short int rowPerspectivePx) 
{
    for ( int i = 0; i < 16; i++ ) {
        point[ i ].x = x_point_screen[ i ] - x_half;
        point[ i ].y = rowPerspectivePx + y_point_screen[ i ];
    }
}
//
//  Fluchtpunkt: y=0
//  baseline:    line on the lower end of the screen
//  y_baseline:  vertical distance from Fluchtpunkt to lower end of the screen.
//               screen height + y_fluchtpunkt
//
PRIVATE void project_x_onto_baseline_16(const CoordPoint * __restrict point, const short int y_baseline, int32_t * __restrict x_base_line, bool * __restrict ok) 
{
    for ( int i = 0; i < 16; i++ ) {
        if ( point[ i ].y == 0 )
        {
            ok[ i ] = false;
        }
        else
        {
            x_base_line[ i ] = ( y_baseline * point[ i ].x ) / point[ i ].y;
        }
    }
}

PRIVATE void apply_offset_16(int32_t * __restrict x, short int offset, int32_t * __restrict x_offset)
{
    for ( int i = 0; i < 16; i++ ) {
        x_offset[ i ] =  x[ i ] - offset;
    }
}

PRIVATE void is_within_range_16(int32_t * __restrict x, const int16_t range_baseline, bool * __restrict ok)
{
    for ( int i = 0; i < 16; i++ ) {
        //ok[ i ] = ((unsigned short int)std::abs(x[ i ]) < range_baseline ) ? false : ok[ i ];
        if ( std::abs(x[ i ]) > range_baseline )
        {
            ok[ i ] = false;
        }
    }
}

PRIVATE void project_x_inbetween_first_row_16(const int32_t * __restrict x_baseline, const int16_t refline_distance, int16_t * __restrict x_baseline_first_row) 
{
    if ( refline_distance == 0)
    {
        // 2024-02-10 Spindler
        // don't know if this makes sense.
        // refline_distance == 0 means there are no reflines?
        for ( int i = 0; i < 16; i++ ) {
            x_baseline_first_row[ i ] = x_baseline[ i ];
        }
    }
    else
    {
        for ( int i = 0; i < 16; i++ ) {
            x_baseline_first_row[ i ] = x_baseline[ i ] % refline_distance;
        }
    }
}

PRIVATE void distance_to_nearest_refline_on_baseline(const short int * __restrict x_first_row, const short int refline_distance, const short int half_refline_distance, short int * __restrict delta_pixels)
{
    for ( int i = 0; i < 16; i++ ) {
        if ( x_first_row[ i ] >=  half_refline_distance ) {
           delta_pixels[ i ] = x_first_row[ i ] - refline_distance;
        } else if ( x_first_row[ i ] <= -half_refline_distance ) {
           delta_pixels[ i ] = x_first_row[ i ] + refline_distance;
        } else {
           delta_pixels[ i ] = x_first_row[ i ];
        }
    }
}

void calc_delta_from_nearest_refline_16(const short int * __restrict x_screen, const short int * __restrict y_screen, const CalcSettingsShort& settings, int32_t *my_pixels, bool * __restrict ok)
{
    CoordPoint point[ 16 ];
    //bool ok[ 16 ] = { true, };

    project_screen_point_to_coord_16(x_screen,y_screen, point, settings.x_half, settings.rowPerspectivePx);

    int32_t x_baseline[ 16 ];
    project_x_onto_baseline_16(point, settings.y_baseline, x_baseline, ok );

    int32_t x_baseline_offset[ 16 ];
    apply_offset_16(x_baseline, settings.offset, x_baseline_offset);

    is_within_range_16(x_baseline_offset, settings.range_baseline, ok );

    short int x_baseline_first_row[ 16 ];
    project_x_inbetween_first_row_16(x_baseline_offset, settings.refline_distance,x_baseline_first_row);

    short int delta_pixels[ 16 ];
    distance_to_nearest_refline_on_baseline(x_baseline_first_row, settings.refline_distance, settings.half_refline_distance, delta_pixels);

    *my_pixels = 0;
    for ( int i = 0; i < 16; i++ ) {
        if ( ok[i] )
        {
            *my_pixels += delta_pixels[ i ];
        }
    }

    //*my_ok+=16;
}