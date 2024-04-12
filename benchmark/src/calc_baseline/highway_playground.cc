#include "hwy/highway.h"
#include "hwy/print-inl.h"
#include "hwy/detect_targets.h"

HWY_BEFORE_NAMESPACE();

namespace play {
// This namespace name is unique per target, which allows code for multiple
// targets to co-exist in the same translation unit. Required when using dynamic
// dispatch, otherwise optional.

namespace hn = hwy::HWY_NAMESPACE;

void run()
{
    const hn::ScalableTag<float>   df;
    const hn::ScalableTag<int16_t> di16;
	const hn::ScalableTag<int32_t> di32;
    const hn::ScalableTag<hwy::bfloat16_t> bf16;
    const hn::ScalableTag<hwy::float16_t>  df16;
    const hn::RepartitionToWide<decltype(di16)> diw;

	printf("lanes float      %lu\n", hn::Lanes(df));
    printf("lanes int32_t    %lu\n", hn::Lanes(di32));
    printf("lanes int16_t    %lu\n", hn::Lanes(di16));
    printf("lanes bfloat16_t %lu\n", hn::Lanes(bf16));
    printf("lanes float16_t  %lu\n", hn::Lanes(df16));

	int16_t x[16] __attribute__ ((aligned (64)));
	int16_t y[16] __attribute__ ((aligned (64)));
    int32_t x32[16] __attribute__ ((aligned (64)));
	int32_t y32[16] __attribute__ ((aligned (64)));

	for (int i=0; i < 16; i++)
	{
		x32[i] = x[i] = i * i;
		y32[i] = y[i] = i * i;
        
	}

    x[0] = y[0] = 0x7FFF;
    x[2] = y[2] = 0xFFFF;
    x[4] = 1; y[4] = -1;
    x[1] = -1; y[1] = 0;
    y[3] = 0;

    const auto vx_i16 = hn::Load(di16, &x[0] );
    const auto vy_i16 = hn::Load(di16, &y[0] );

    const hn::Half< hn::RepartitionToNarrow<hn::ScalableTag<float>> > half;

    const auto vx_f16 = 
        hn::Combine(df16
            ,hn::DemoteTo(half, hn::ConvertTo(df, hn::Load(di32, &x32[0] )))
            ,hn::DemoteTo(half, hn::ConvertTo(df, hn::Load(di32, &y32[0] )))
        );

    const auto vy_f16 = 
        hn::Combine(df16
            ,hn::DemoteTo(half, hn::ConvertTo(df, hn::Load(di32, &y32[0] )))
            ,hn::DemoteTo(half, hn::ConvertTo(df, hn::Load(di32, &x32[0] )))
        );

    //const auto vy_f16 = hn::DemoteTo(half, hn::ConvertTo(df, hn::Load(di32, &y32[0] )));

    
    
    #if HWY_HAVE_FLOAT16 == 1
auto xDy = hn::Add(vx_f16,vy_f16);
    #endif

    hn::Print(df16, "x          ", vx_f16, 0, hn::Lanes(df16));
    hn::Print(df16, "y          ", vy_f16, 0, hn::Lanes(df16));
   	//hn::Print(df16, "Div x y    ", xDy,    0, hn::Lanes(df16));


    /*
    const auto vx = hn::Load(di16, &x[0] );
    const auto vy = hn::Load(di16, &y[0] );

    auto xDy = hn::Div(vx,vy);

    hn::Print(di16, "x          ", vx, 0, hn::Lanes(di16));
    hn::Print(di16, "y          ", vy, 0, hn::Lanes(di16));
   	hn::Print(di16, "Div x y    ", xDy,0, hn::Lanes(di16));
    */

}

} // namespace play

HWY_AFTER_NAMESPACE();