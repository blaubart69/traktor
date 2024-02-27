#include "hwy/highway.h"
#include "hwy/print-inl.h"
#include "hwy/detect_targets.h"

#include "calc_baseline.h"

HWY_BEFORE_NAMESPACE();

namespace deltapx {
// This namespace name is unique per target, which allows code for multiple
// targets to co-exist in the same translation unit. Required when using dynamic
// dispatch, otherwise optional.

namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

template <class DF, class DI>
static int32_t ONE_delta_pixels(
	const DF df
  , const DI di		
  , const CalcSettings& settings
  ,	const int32_t * HWY_RESTRICT x_point_screen
  , const int32_t * HWY_RESTRICT y_point_screen
  , int32_t *delta_pixels
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
	//		!!! make sure "y_coord" IS NEVER 0 			!!!
	//		!!! so we don't have to deal with INFINITY 	!!!
	//		hint for myself: y_fluchtpunkt > 0
	//
	const auto v_y_baseline = hn::Set(df, (float)settings.y_baseline);
	
	auto v_x_coord_float = hn::ConvertTo(df, v_x_coord);
	auto v_y_coord_float = hn::ConvertTo(df, v_y_coord);

	// x_base_line[ i ] = ( y_baseline * point[ i ].x ) / point[ i ].y;
	auto v_x_baseline_float = hn::Div( hn::Mul( v_x_coord_float, v_y_baseline), v_y_coord_float );
	auto v_x_baseline = hn::ConvertTo(di, v_x_baseline_float);

	//
	// 3. apply offset
	//
	const auto v_offset = hn::Set(di, settings.offset);
	v_x_baseline = hn::Add( v_x_baseline, v_offset );

	//
	// 4. is_within_range
	//
	const auto v_range_baseline = hn::Set(di, settings.range_baseline);
	auto mask_within_range = hn::Lt( hn::Abs(v_x_baseline), v_range_baseline);

	if ( hn::AllFalse(di, mask_within_range) )
	{
		return 0;
	}
	
	//
	// 5. project_x_inbetween_first_row 
	//	==> x_baseline % (int)refline_distance
	//
	// V: {u,i} \ V MaskedModOr(V no, M m, V a, V b): returns a[i] % b[i] or no[i] if m[i] is false.
	const auto v_refline_distance = hn::Set(di, settings.refline_distance);
	auto v_x_one_row = hn::MaskedModOr( hn::Zero(di) , mask_within_range, v_x_baseline, v_refline_distance );

	//
	// 6. distance_to_nearest_refline_on_baseline
	//
	//		if ( x_first_row >=  half_refline_distance ) return x_first_row - refline_distance;
    //		if ( x_first_row <= -half_refline_distance ) return x_first_row + refline_distance;
    //		return x_first_row;
	//
	const auto       v_half_refline_distance = hn::Set(di, settings.half_refline_distance);
	const auto v_minus_half_refline_distance = hn::Neg(           v_half_refline_distance);

	const auto mask_gt = hn::Gt(v_x_one_row, v_half_refline_distance);
	const auto mask_lt = hn::Lt(v_x_one_row, v_minus_half_refline_distance);

	auto x_result_gt = hn::MaskedSubOr( hn::Zero(di), mask_gt, v_x_one_row, v_refline_distance);
	auto x_result_lt = hn::MaskedAddOr( hn::Zero(di), mask_lt, v_x_one_row, v_refline_distance);

	auto mask_rest       = hn::And( hn::Not(mask_gt), hn::Not(mask_lt) );
	auto mask_valid_rest = hn::And( mask_rest, mask_within_range );

	auto x_result_valid_rest = hn::IfThenElse( mask_valid_rest, v_x_one_row, hn::Zero(di) );
	 
	*delta_pixels = hn::ReduceSum(di, x_result_gt)
				  + hn::ReduceSum(di, x_result_lt)
				  + hn::ReduceSum(di, x_result_valid_rest);

	#ifdef DEBUG
	hn::Print(di, "v_x_baseline       ", v_x_baseline, 	        0, hn::Lanes(di));
	hn::Print(di, "v_x_one_row        ", v_x_one_row, 	        0, hn::Lanes(di));
	hn::Print(di, "x_result_gt        ", x_result_gt, 	        0, hn::Lanes(di));
	hn::Print(di, "x_result_lt        ", x_result_lt, 	        0, hn::Lanes(di));
	hn::Print(di, "x_result_valid_rest", x_result_valid_rest, 	0, hn::Lanes(di));
	printf("----\n");
	#endif

	const auto ones = hn::Set(di, 1);
	auto valid_points = hn::IfThenElse( mask_within_range, ones, hn::Zero(di) );
	int32_t count_valid_points = hn::ReduceSum(di, valid_points);

	return count_valid_points;
}

static int32_t hwy_calc_delta_pixels(
	  const size_t	size
  	, const int32_t* __restrict x_screen
  	, const int32_t* __restrict y_screen
  	, const CalcSettings& settings
  	, int32_t *delta_pixels )
{
	const hn::ScalableTag<float>   df;
	const hn::ScalableTag<int32_t> di;
  	const size_t N = hn::Lanes(di);

	#ifdef DEBUG
	printf("lanes int32_t: %lu\n", N);
	printf("lanes float  : %lu\n", hn::Lanes(df));
	#endif

	//size_t sum_delta_pixels = 0;
	int32_t valid_points = 0;
	for (size_t i = 0; i < size; i += N) 
	{
		int32_t ONE_delta_px = 0;
		valid_points += ONE_delta_pixels(df, di, settings, x_screen + i, y_screen + i, &ONE_delta_px);
		*delta_pixels += ONE_delta_px;
	}

	#ifdef DEBUG
	printf("SUM delta: %zu, valid points: %d\n",sum_delta_pixels, valid_points);
	#endif

	return valid_points;
}


}
}
HWY_AFTER_NAMESPACE();

namespace deltapx {

	int32_t run_hwy_calc_delta_pixels(
	  const size_t	size
  	, const int32_t* __restrict x_screen
  	, const int32_t* __restrict y_screen
  	, const CalcSettings& settings
  	, int32_t *delta_pixels )
	{
		return HWY_STATIC_DISPATCH(hwy_calc_delta_pixels)(size, x_screen, y_screen, settings, delta_pixels );
	}
}
