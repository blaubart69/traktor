#pragma once

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

struct CalcSettingsShort
{
    const int16_t x_half;
    const int16_t rowPerspectivePx;
    const int16_t y_baseline;
    const int16_t offset;
    const int16_t refline_distance;
    const int16_t half_refline_distance;
    const int16_t range_baseline;

    CalcSettingsShort(int16_t x_half, int16_t y_screen_size, int16_t rowPerspectivePx, int16_t refline_distance, int16_t offset, int16_t row_count) 
    :   x_half(x_half)
    ,   rowPerspectivePx(rowPerspectivePx)
    ,   y_baseline(rowPerspectivePx + y_screen_size)
    ,   offset(offset)
    ,   refline_distance(refline_distance)
    ,   half_refline_distance(refline_distance / 2)
    ,   range_baseline( ( refline_distance * row_count ) + (refline_distance / 2) )
    {}
};


bool calc_baseline_delta_from_nearest_refline(const int x_screen, const int y_screen, const CalcSettings& settings, int *delta_pixels);
bool calc_baseline_delta_from_nearest_refline_int(const int x_screen, const int y_screen, const CalcSettings& settings, int *delta_pixels);
bool calc_baseline_delta_from_nearest_refline_float_mul(const int x_screen, const int y_screen, const CalcSettings& settings, int *delta_pixels);
bool calc_baseline_delta_from_nearest_refline_short_int(const int16_t x_screen, const int16_t y_screen, const CalcSettingsShort& settings, int *delta_pixels);

void calc_delta_from_nearest_refline_16(const short int * __restrict x_screen, const short int * __restrict y_screen, const CalcSettingsShort& settings, int32_t *my_pixels, bool * __restrict ok);

namespace deltapx {

	int32_t run_hwy_calc_delta_pixels(
	  const size_t	size
  	, const int32_t* __restrict x_screen
  	, const int32_t* __restrict y_screen
  	, const CalcSettings& settings
  	, int32_t *delta_pixels );
}

void test_hwy_calc_delta_pixels();