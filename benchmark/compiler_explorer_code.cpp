#include "hwy/highway.h"

HWY_BEFORE_NAMESPACE();

//namespace deltapx {
// This namespace name is unique per target, which allows code for multiple
// targets to co-exist in the same translation unit. Required when using dynamic
// dispatch, otherwise optional.

namespace HWY_NAMESPACE {

namespace hn = hwy::HWY_NAMESPACE;

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
    VI offset; 			         
    VI range_baseline; 	         
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
		offset 			            = hn::Set(di, settings.offset);
		range_baseline 	            = hn::Set(di, settings.range_baseline);
		refline_distance            = hn::Set(di, settings.refline_distance);
              half_refline_distance = hn::Set(di, settings.half_refline_distance);
		minus_half_refline_distance = hn::Neg(             half_refline_distance);
	}
};


template <class DI, class DW, class DF>
static int32_t ONE_delta_pixels_int16_fdiv(
    const DI di
  , const DW dw	
  , const DF df
  , const ViCalcSettings<DI>& settings
  ,	const int16_t * HWY_RESTRICT x_point_screen
  , const int16_t * HWY_RESTRICT y_point_screen
  , int32_t *delta_pixels
)
{
    const hn::Half< hn::RepartitionToNarrow<DF> > dfh;
	//
	// 1. project to coord
	//
	auto x_coord = hn::Sub( hn::Load(di, x_point_screen), settings.x_half );		
	auto y_coord = hn::Add( hn::Load(di, y_point_screen), settings.rowPerspectivePx );		

	//
	// 2. project to onto baseline
	//		!!! make sure "y_coord" IS NEVER 0 			!!!
	//		!!! so we don't have to deal with INFINITY 	!!!
	//		hint for myself: y_fluchtpunkt > 0
	//
	// --> *x_base_line = ( y_baseline * point.x ) / point.y;

    auto mul = hn::Mul( 
        hn::ConvertTo(dfh, settings.y_baseline)
      , hn::ConvertTo(dfh, x_coord ) );

    auto x_baseline = 
        hn::ConvertTo(di,
            hn::Div( 
                mul, 
                hn::ConvertTo(dfh,y_coord) ) );
	
	//
	// 3. apply offset
	//
    x_baseline = hn::Add( x_baseline, settings.offset );

	//
	// 4. is_within_range
	//
    auto mask_within_range = hn::Lt( hn::Abs(x_baseline ), settings.range_baseline);

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
	auto v_x_one_row = hn::MaskedModOr( hn::Zero(di) , mask_within_range, x_baseline, settings.refline_distance );

	//
	// 6. distance_to_nearest_refline_on_baseline
	//
	//		if ( x_first_row >=  half_refline_distance ) return x_first_row - refline_distance;
    //		if ( x_first_row <= -half_refline_distance ) return x_first_row + refline_distance;
    //		return x_first_row;
	//
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


int32_t hwy_calc_delta_pixels_int16_fdiv(
	  const size_t	size
  	, const int16_t* __restrict x_screen
  	, const int16_t* __restrict y_screen
  	, const CalcSettings& settings
  	, int32_t *delta_pixels )
{
	const hn::ScalableTag<int16_t> di;
	const hn::ScalableTag<int32_t> di32;
	const hn::ScalableTag<float> df;
  	const size_t N = hn::Lanes(di);

	//#ifdef DEBUG
	//printf("lanes int16_t: %lu\n", hn::Lanes(di));
	//#endif

	ViCalcSettings vsettings(di,settings);

	int32_t valid_points = 0;
	*delta_pixels = 0;
	for (size_t i = 0; i < size; i += N) 
	{
		int32_t ONE_delta_px = 0;
		valid_points += ONE_delta_pixels_int16_fdiv(di, di32, df, vsettings, x_screen + i, y_screen + i, &ONE_delta_px);
		*delta_pixels += ONE_delta_px;
	}

	//#ifdef DEBUG
	//printf("SUM delta: %zu, valid points: %d\n",delta_pixels, valid_points);
	//#endif

	return valid_points;
}

template <class DI>
int32_t sum_mod(
    const DI di
  ,	const int16_t * HWY_RESTRICT a
  , const int16_t * HWY_RESTRICT b) 
{
	
	auto result = hn::Mod(
		  hn::Load(di, a)
		, hn::Load(di, b) );

	auto sum = hn::ReduceSum(di, result);
	return sum;
}

int32_t call_sum_mod(
      const int16_t* __restrict x_screen
  	, const int16_t* __restrict y_screen) {
    const hn::ScalableTag<int16_t> di;
    return sum_mod(di,x_screen,y_screen);
}



} // namespace HWY_NAMESPACE {
//} // namespace deltspx

HWY_AFTER_NAMESPACE();