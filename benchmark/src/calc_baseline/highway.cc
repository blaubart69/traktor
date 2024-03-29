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
struct VCalcSettings
{
	const DF df;
	const DI di;

	using VI = hn::Vec<decltype(di)>;
	using VF = hn::Vec<decltype(df)>;

    VI x_half;           	         
    VI rowPerspectivePx; 	         
    VF y_baseline; 		         
    VI offset; 			         
    VI range_baseline; 	         
    VI refline_distance;            
    VI       half_refline_distance ;
    VI minus_half_refline_distance;

	VCalcSettings(
		const DF df,
		const DI di,
		const CalcSettings& settings)
	{
		x_half           	        = hn::Set(di, settings.x_half);
		rowPerspectivePx 	        = hn::Set(di, settings.rowPerspectivePx);
		y_baseline 		            = hn::Set(df, (float)settings.y_baseline);
		offset 			            = hn::Set(di, settings.offset);
		range_baseline 	            = hn::Set(di, settings.range_baseline);
		refline_distance            = hn::Set(di, settings.refline_distance);
              half_refline_distance = hn::Set(di, settings.half_refline_distance);
		minus_half_refline_distance = hn::Neg(             half_refline_distance);
	}
};

template <class DI>
struct ViCalcSettings
{
	const DI di;
	const hn::RepartitionToWide<decltype(di)> dw;

	using VI = hn::Vec<decltype(di)>;
	using VW = hn::Vec<decltype(dw)>;

    VI x_half;           	         
    VI rowPerspectivePx; 	         
    VI y_baseline; 		         
    VW offset; 			         
    VW range_baseline; 	         
    VI refline_distance;            
    VI       half_refline_distance;
    VI minus_half_refline_distance;

	ViCalcSettings(
		const DI di,
		const CalcSettings& settings)
	{
		x_half           	        = hn::Set(di, settings.x_half);
		rowPerspectivePx 	        = hn::Set(di, settings.rowPerspectivePx);
		y_baseline 		            = hn::Set(di, settings.y_baseline);
		offset 			            = hn::Set(dw, settings.offset);
		range_baseline 	            = hn::Set(dw, settings.range_baseline);
		refline_distance            = hn::Set(di, settings.refline_distance);
              half_refline_distance = hn::Set(di, settings.half_refline_distance);
		minus_half_refline_distance = hn::Neg(             half_refline_distance);
	}
};

template <class DI>
struct Vi2CalcSettings
{
	const DI di;

	using VI = hn::Vec<decltype(di)>;

    VI x_half;           	         
    VI rowPerspectivePx; 	         
    VI y_baseline; 		         
    VI offset; 			         
    VI range_baseline; 	         
    VI refline_distance;            
    VI       half_refline_distance;
    VI minus_half_refline_distance;

	Vi2CalcSettings(
		const DI di,
		const CalcSettings& settings)
	{
		x_half           	        = hn::Set(di, settings.x_half);
		rowPerspectivePx 	        = hn::Set(di, settings.rowPerspectivePx);
		y_baseline 		            = hn::Set(di, settings.y_baseline);
		offset 			            = hn::Set(di, settings.offset);
		range_baseline 	            = hn::Set(di, settings.range_baseline);
		refline_distance            = hn::Set(di, settings.refline_distance);
              half_refline_distance = hn::Set(di, settings.half_refline_distance);
		minus_half_refline_distance = hn::Neg(             half_refline_distance);
	}
};


template <class DF, class DI>
static int32_t ONE_delta_pixels_int32(
	const DF df
  , const DI di		
  , const VCalcSettings<DF,DI>& settings
  ,	const int32_t * HWY_RESTRICT x_point_screen
  , const int32_t * HWY_RESTRICT y_point_screen
  , int32_t *delta_pixels
)
{
	//
	// 1. project to coord
	//
	//const auto v_x_half           = hn::Set(di, settings.x_half);
  	//const auto v_rowPerspectivePx = hn::Set(di, settings.rowPerspectivePx);

	// x_coord[ i ] = x_point_screen[ i ] - x_half;
	auto v_x_coord = hn::Sub( hn::Load(di, x_point_screen) , settings.x_half );		
	// y_coord[ i ] = y_point_screen[ i ] + rowPerspectivePx
	auto v_y_coord = hn::Add( hn::Load(di, y_point_screen), settings.rowPerspectivePx );		

	//
	// 2. project to onto baseline
	//		!!! make sure "y_coord" IS NEVER 0 			!!!
	//		!!! so we don't have to deal with INFINITY 	!!!
	//		hint for myself: y_fluchtpunkt > 0
	//
	//const auto v_y_baseline = hn::Set(df, (float)settings.y_baseline);
	
	auto v_x_coord_float = hn::ConvertTo(df, v_x_coord);
	auto v_y_coord_float = hn::ConvertTo(df, v_y_coord);

	// x_base_line[ i ] = ( y_baseline * point[ i ].x ) / point[ i ].y;
	auto v_x_baseline_float = hn::Div( hn::Mul( v_x_coord_float, settings.y_baseline), v_y_coord_float );
	auto v_x_baseline = hn::ConvertTo(di, v_x_baseline_float);

	//
	// 3. apply offset
	//
	//const auto v_offset = hn::Set(di, settings.offset);
	v_x_baseline = hn::Add( v_x_baseline, settings.offset );

	//
	// 4. is_within_range
	//
	//const auto v_range_baseline = hn::Set(di, settings.range_baseline);
	auto mask_within_range = hn::Lt( hn::Abs(v_x_baseline), settings.range_baseline);
	
	auto count_valid_points = hn::CountTrue(di,mask_within_range);
	if ( count_valid_points == 0 )
	{
		return 0;
	}
	
	//
	// 5. project_x_inbetween_first_row 
	//	==> x_baseline % (int)refline_distance
	//
	// V: {u,i} \ V MaskedModOr(V no, M m, V a, V b): returns a[i] % b[i] or no[i] if m[i] is false.
	//const auto v_refline_distance = hn::Set(di, settings.refline_distance);
	auto v_x_one_row = hn::MaskedModOr( hn::Zero(di) , mask_within_range, v_x_baseline, settings.refline_distance );

	//
	// 6. distance_to_nearest_refline_on_baseline
	//
	//		if ( x_first_row >=  half_refline_distance ) return x_first_row - refline_distance;
    //		if ( x_first_row <= -half_refline_distance ) return x_first_row + refline_distance;
    //		return x_first_row;
	//
	//const auto       v_half_refline_distance = hn::Set(di, settings.half_refline_distance);
	//const auto v_minus_half_refline_distance = hn::Neg(           v_half_refline_distance);

	const auto mask_gt = hn::Gt(v_x_one_row, settings.half_refline_distance);
	const auto mask_lt = hn::Lt(v_x_one_row, settings.minus_half_refline_distance);

	auto x_result_gt = hn::MaskedSubOr( hn::Zero(di), mask_gt, v_x_one_row, settings.refline_distance);
	auto x_result_lt = hn::MaskedAddOr( hn::Zero(di), mask_lt, v_x_one_row, settings.refline_distance);

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

	return count_valid_points;
}

static int32_t hwy_calc_delta_pixels_int32(
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

	VCalcSettings vsettings(df,di,settings);

	int32_t valid_points = 0;
	*delta_pixels = 0;
	for (size_t i = 0; i < size; i += N) 
	{
		int32_t ONE_delta_px = 0;
		valid_points += ONE_delta_pixels_int32(df, di, vsettings, x_screen + i, y_screen + i, &ONE_delta_px);
		*delta_pixels += ONE_delta_px;
	}

	#ifdef DEBUG
	printf("SUM delta: %zu, valid points: %d\n",delta_pixels, valid_points);
	#endif

	return valid_points;
}

// --- int32 only

template <class DI>
static int32_t ONE_delta_pixels_int32_only(
    const DI di		
  , const Vi2CalcSettings<DI>& settings
  ,	const int32_t * HWY_RESTRICT x_point_screen
  , const int32_t * HWY_RESTRICT y_point_screen
  , int32_t *delta_pixels
)
{
	//
	// 1. project to coord
	//
	auto v_x_coord = hn::Sub( hn::Load(di, x_point_screen), settings.x_half );		
	auto v_y_coord = hn::Add( hn::Load(di, y_point_screen), settings.rowPerspectivePx );		

	//
	// 2. project to onto baseline
	//		!!! make sure "y_coord" IS NEVER 0 			!!!
	//		!!! so we don't have to deal with INFINITY 	!!!
	//		hint for myself: y_fluchtpunkt > 0
	//
	// x_base_line[ i ] = ( y_baseline * point[ i ].x ) / point[ i ].y;

	auto v_x_baseline = hn::Div( hn::Mul( v_x_coord, settings.y_baseline), v_y_coord );
	//
	// 3. apply offset
	//
	//const auto v_offset = hn::Set(di, settings.offset);
	v_x_baseline = hn::Add( v_x_baseline, settings.offset );

	//
	// 4. is_within_range
	//
	//const auto v_range_baseline = hn::Set(di, settings.range_baseline);
	auto mask_within_range = hn::Lt( hn::Abs(v_x_baseline), settings.range_baseline);
	
	auto count_valid_points = hn::CountTrue(di,mask_within_range);
	if ( count_valid_points == 0 )
	{
		return 0;
	}
	
	//
	// 5. project_x_inbetween_first_row 
	//	==> x_baseline % (int)refline_distance
	//
	// V: {u,i} \ V MaskedModOr(V no, M m, V a, V b): returns a[i] % b[i] or no[i] if m[i] is false.
	//const auto v_refline_distance = hn::Set(di, settings.refline_distance);
	auto v_x_one_row = hn::MaskedModOr( hn::Zero(di) , mask_within_range, v_x_baseline, settings.refline_distance );

	//
	// 6. distance_to_nearest_refline_on_baseline
	//
	//		if ( x_first_row >=  half_refline_distance ) return x_first_row - refline_distance;
    //		if ( x_first_row <= -half_refline_distance ) return x_first_row + refline_distance;
    //		return x_first_row;
	//
	//const auto       v_half_refline_distance = hn::Set(di, settings.half_refline_distance);
	//const auto v_minus_half_refline_distance = hn::Neg(           v_half_refline_distance);

	const auto mask_gt = hn::Gt(v_x_one_row, settings.half_refline_distance);
	const auto mask_lt = hn::Lt(v_x_one_row, settings.minus_half_refline_distance);

	auto x_result_gt = hn::MaskedSubOr( hn::Zero(di), mask_gt, v_x_one_row, settings.refline_distance);
	auto x_result_lt = hn::MaskedAddOr( hn::Zero(di), mask_lt, v_x_one_row, settings.refline_distance);

	auto mask_rest       = hn::And( hn::Not(mask_gt), hn::Not(mask_lt) );
	auto mask_valid_rest = hn::And( mask_rest, mask_within_range );

	auto x_result_valid_rest = hn::IfThenElse( mask_valid_rest, v_x_one_row, hn::Zero(di) );
	 
	*delta_pixels = hn::ReduceSum(di, x_result_gt)
				  + hn::ReduceSum(di, x_result_lt)
				  + hn::ReduceSum(di, x_result_valid_rest);

	return count_valid_points;
}

static int32_t hwy_calc_delta_pixels_int32_only(
	  const size_t	size
  	, const int32_t* __restrict x_screen
  	, const int32_t* __restrict y_screen
  	, const CalcSettings& settings
  	, int32_t *delta_pixels )
{
	const hn::ScalableTag<int32_t> di;
  	const size_t N = hn::Lanes(di);

	Vi2CalcSettings vsettings(di,settings);

	int32_t valid_points = 0;
	*delta_pixels = 0;
	for (size_t i = 0; i < size; i += N) 
	{
		int32_t ONE_delta_px = 0;
		valid_points += ONE_delta_pixels_int32_only(di, vsettings, x_screen + i, y_screen + i, &ONE_delta_px);
		*delta_pixels += ONE_delta_px;
	}

	#ifdef DEBUG
	printf("SUM delta: %zu, valid points: %d\n",delta_pixels, valid_points);
	#endif

	return valid_points;
}

// --- int32 only 

template <class DI, class DW>
static int32_t ONE_delta_pixels_int16(
    const DI di
  , const DW dw	
  , const ViCalcSettings<DI>& settings
  ,	const int16_t * HWY_RESTRICT x_point_screen
  , const int16_t * HWY_RESTRICT y_point_screen
  , int32_t *delta_pixels
)
{
	//
	// 1. project to coord
	//
	auto v_x_coord = hn::Sub( hn::Load(di, x_point_screen), settings.x_half );		
	auto v_y_coord = hn::Add( hn::Load(di, y_point_screen), settings.rowPerspectivePx );		

	//
	// 2. project to onto baseline
	//		!!! make sure "y_coord" IS NEVER 0 			!!!
	//		!!! so we don't have to deal with INFINITY 	!!!
	//		hint for myself: y_fluchtpunkt > 0
	//
	// --> *x_base_line = ( y_baseline * point.x ) / point.y;

	auto mul_even = hn::MulEven( settings.y_baseline , v_x_coord );
	auto mul_odd  = hn::MulOdd ( settings.y_baseline , v_x_coord );

	using VW = hn::Vec<decltype(dw)>;
	using VI = hn::Vec<decltype(di)>;

	VW point_y_even = hn::PromoteEvenTo(dw, v_y_coord );
	VW point_y_odd  = hn::PromoteOddTo (dw, v_y_coord );

	auto x_baseline_even = hn::Div( mul_even, point_y_even );
	auto x_baseline_odd  = hn::Div( mul_odd,  point_y_odd );

	//
	// 3. apply offset
	//
	//const auto v_offset = hn::Set(di, settings.offset);
	//v_x_baseline = hn::Add( v_x_baseline, settings.offset );
	x_baseline_even = hn::Add( x_baseline_even, settings.offset );
	x_baseline_odd  = hn::Add( x_baseline_odd,  settings.offset );

	//
	// 4. is_within_range
	//
	//const auto v_range_baseline = hn::Set(di, settings.range_baseline);
	auto mask_within_range_even = hn::Lt( hn::Abs(x_baseline_even), settings.range_baseline);
	auto mask_within_range_odd  = hn::Lt( hn::Abs(x_baseline_odd ), settings.range_baseline);

	const hn::Half< hn::RepartitionToNarrow<DW> > dwh;

	VI v_x_baseline = 
		hn::Combine(di, 
			hn::DemoteTo(dwh, x_baseline_even)
		  , hn::DemoteTo(dwh, x_baseline_odd) );
		  
	auto mask_within_range = 
		hn::CombineMasks(di, 
			  hn::DemoteMaskTo(dwh, dw, mask_within_range_even)
			, hn::DemoteMaskTo(dwh, dw, mask_within_range_odd));

	
	auto count_valid_points = hn::CountTrue(di,mask_within_range);
	if ( count_valid_points == 0 )
	{
		return 0;
	}
	
	//
	// 5. project_x_inbetween_first_row 
	//	==> x_baseline % (int)refline_distance
	//
	// V: {u,i} \ V MaskedModOr(V no, M m, V a, V b): returns a[i] % b[i] or no[i] if m[i] is false.
	//const auto v_refline_distance = hn::Set(di, settings.refline_distance);
	auto v_x_one_row = hn::MaskedModOr( hn::Zero(di) , mask_within_range, v_x_baseline, settings.refline_distance );

	//
	// 6. distance_to_nearest_refline_on_baseline
	//
	//		if ( x_first_row >=  half_refline_distance ) return x_first_row - refline_distance;
    //		if ( x_first_row <= -half_refline_distance ) return x_first_row + refline_distance;
    //		return x_first_row;
	//
	//const auto       v_half_refline_distance = hn::Set(di, settings.half_refline_distance);
	//const auto v_minus_half_refline_distance = hn::Neg(           v_half_refline_distance);

	const auto mask_gt = hn::Gt(v_x_one_row, settings.half_refline_distance);
	const auto mask_lt = hn::Lt(v_x_one_row, settings.minus_half_refline_distance);

	auto x_result_gt = hn::MaskedSubOr( hn::Zero(di), mask_gt, v_x_one_row, settings.refline_distance);
	auto x_result_lt = hn::MaskedAddOr( hn::Zero(di), mask_lt, v_x_one_row, settings.refline_distance);

	auto mask_rest       = hn::And( hn::Not(mask_gt), hn::Not(mask_lt) );
	auto mask_valid_rest = hn::And( mask_rest, mask_within_range );

	auto x_result_valid_rest = hn::IfThenElse( mask_valid_rest, v_x_one_row, hn::Zero(di) );
	 
	*delta_pixels = hn::ReduceSum(di, x_result_gt)
				  + hn::ReduceSum(di, x_result_lt)
				  + hn::ReduceSum(di, x_result_valid_rest);

	return count_valid_points;
}

static int32_t hwy_calc_delta_pixels_int16(
	  const size_t	size
  	, const int16_t* __restrict x_screen
  	, const int16_t* __restrict y_screen
  	, const CalcSettings& settings
  	, int32_t *delta_pixels )
{
	const hn::ScalableTag<int16_t> di;
	const hn::ScalableTag<int32_t> di32;
  	const size_t N = hn::Lanes(di);

	#ifdef DEBUG
	printf("lanes int16_t: %lu\n", hn::Lanes(di));
	#endif

	ViCalcSettings vsettings(di,settings);

	int32_t valid_points = 0;
	*delta_pixels = 0;
	for (size_t i = 0; i < size; i += N) 
	{
		int32_t ONE_delta_px = 0;
		valid_points += ONE_delta_pixels_int16(di, di32, vsettings, x_screen + i, y_screen + i, &ONE_delta_px);
		*delta_pixels += ONE_delta_px;
	}

	#ifdef DEBUG
	printf("SUM delta: %zu, valid points: %d\n",delta_pixels, valid_points);
	#endif

	return valid_points;
}

}
}
HWY_AFTER_NAMESPACE();

namespace deltapx {

	int32_t run_hwy_calc_delta_pixels_int32(
	  const size_t	size
  	, const int32_t* __restrict x_screen
  	, const int32_t* __restrict y_screen
  	, const CalcSettings& settings
  	, int32_t *delta_pixels )
	{
		return HWY_STATIC_DISPATCH(hwy_calc_delta_pixels_int32)(size, x_screen, y_screen, settings, delta_pixels );
	}

	int32_t run_hwy_calc_delta_pixels_int32_only(
	  const size_t	size
  	, const int32_t* __restrict x_screen
  	, const int32_t* __restrict y_screen
  	, const CalcSettings& settings
  	, int32_t *delta_pixels )
	{
		return HWY_STATIC_DISPATCH(hwy_calc_delta_pixels_int32_only)(size, x_screen, y_screen, settings, delta_pixels );
	}

	int32_t run_hwy_calc_delta_pixels_int16(
	  const size_t	size
  	, const int16_t* __restrict x_screen
  	, const int16_t* __restrict y_screen
  	, const CalcSettings& settings
  	, int32_t *delta_pixels )
	{
		return HWY_STATIC_DISPATCH(hwy_calc_delta_pixels_int16)(size, x_screen, y_screen, settings, delta_pixels );
	}

	void print_target()
	{
		for (auto t : hwy::SupportedAndGeneratedTargets())
		{
			printf("Highway target: %s\n",hwy::TargetName(t));
		}
	}
}
