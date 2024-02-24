#include "hwy/highway.h"
#include "hwy/print-inl.h"
//#include "hwy/detect_targets.h"

HWY_BEFORE_NAMESPACE();

namespace deltapx {
// This namespace name is unique per target, which allows code for multiple
// targets to co-exist in the same translation unit. Required when using dynamic
// dispatch, otherwise optional.
namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

#include "calc_baseline.h"

template <class DF, class DI>
void ONE_delta_pixels(
	const DF df
  , const DI di		
  , const CalcSettings& settings
  ,	const int32_t * HWY_RESTRICT x_point_screen
  , const int32_t * HWY_RESTRICT y_point_screen
)
{
	//
	// 1. project to coord
	//
	const auto v_x_half           = hn::Set(di, settings.x_half);
  	const auto v_rowPerspectivePx = hn::Set(di, settings.rowPerspectivePx);

	// x_coord[ i ] = x_point_screen[ i ] - x_half;
	auto v_x_coord = hn::Sub( hn::Load(di, x_point_screen) , v_x_half );		
	// y_coord[ i ] = y_point_screen[ i ] + rowPerspectivePx
	auto v_y_coord = hn::Add( hn::Load(di, y_point_screen), v_rowPerspectivePx );		

	//
	// 2. project to onto baseline
	//
	const auto v_y_baseline = hn::Set(df, (float)settings.y_baseline);
	
	auto v_x_coord_float = hn::ConvertTo(df, v_x_coord);
	auto v_y_coord_float = hn::ConvertTo(df, v_y_coord);

	// x_base_line[ i ] = ( y_baseline * point[ i ].x ) / point[ i ].y;
	auto v_x_baseline = hn::Div( hn::Mul( v_x_coord_float, v_y_baseline), v_y_coord_float );

	hn::Print(df, "x_baseline", v_x_baseline, 0, hn::Lanes(df));
	
}

void hwy_calc_delta_pixels(
	  const size_t	size
  	, const int32_t* __restrict x_screen
  	, const int32_t* __restrict y_screen
  	, const CalcSettings& settings
  	, int32_t *delta_pixels )
{
	const hn::ScalableTag<float>   df;
	const hn::ScalableTag<int32_t> di;
  	const size_t N = hn::Lanes(di);

	printf("lanes int32_t: %lu\n", N);
	printf("lanes float  : %lu\n", hn::Lanes(df));
	for (size_t i = 0; i < size; i += N) 
	{
		ONE_delta_pixels(df,di, settings, x_screen + i, y_screen + i);
	}
}

void test_hwy_calc_delta_pixels()
{
	
	auto targets = hwy::SupportedAndGeneratedTargets();
	for (auto t : targets)
	{
		printf("TargetName: %s\n",hwy::TargetName(t));
	}

	CalcSettings calcSettings(
          640 / 2
        , 480
        , 0     // refSettings.rowPerspectivePx
        , 160   // refSettings.rowSpacingPx
        , 0     // refSettings.offset
        , 3     // row_count
    );

	int32_t x[16] __attribute__ ((aligned (64)));
	int32_t y[16] __attribute__ ((aligned (64)));

	for (int i=0; i < 16; i++)
	{
		x[i] = 240 + i * 10;
		y[i] = 479;
	}

	int32_t delta_pixels;
	hwy_calc_delta_pixels(16, x, y, calcSettings, &delta_pixels);
}

}
}
HWY_AFTER_NAMESPACE();

void test_hwy_calc_delta_pixels()
{
	deltapx::HWY_NAMESPACE::test_hwy_calc_delta_pixels();
}