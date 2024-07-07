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
    
    const hn::ScalableTag<int16_t>          di16;
	const hn::ScalableTag<int32_t>          di32;
    const hn::ScalableTag<hwy::bfloat16_t>  bf16;
    const hn::ScalableTag<hwy::float16_t>   df16;
    const hn::ScalableTag<float>            df32;
    const hn::RepartitionToWide<decltype(di16)> diw;

	printf("lanes float      %lu\n", hn::Lanes(df32));
    printf("lanes int32_t    %lu\n", hn::Lanes(di32));
    printf("lanes int16_t    %lu\n", hn::Lanes(di16));
    printf("lanes bfloat16_t %lu\n", hn::Lanes(bf16));
    printf("lanes float16_t  %lu\n", hn::Lanes(df16));

	int16_t x16[16] __attribute__ ((aligned (64)));
	int16_t y16[16] __attribute__ ((aligned (64)));
    //int32_t x32[16] __attribute__ ((aligned (64)));
	//int32_t y32[16] __attribute__ ((aligned (64)));

	for (int i=0; i < hn::Lanes(di16); i++)
	{
		x16[i] = 1 << i;
		y16[i] = 3;
        
	}
    /*
    x16[0] = y16[0] = 0x7FFF;
    x16[2] = y16[2] = 0xFFFF;
    x16[4] = 1; y16[4] = -1;
    x16[1] = -1; y16[1] = 0;
    y16[3] = 0;
    */
    const auto vx_i16 = hn::Load(di16, &x16[0] );
    const auto vy_i16 = hn::Load(di16, &y16[0] );

    auto iResult = hn::Mod( vx_i16, vy_i16 );

    auto fResult = hn::Div( 
          hn::ConvertTo(df16, vx_i16)
        , hn::ConvertTo(df16, vy_i16) );

    hn::Print(di16, "xi16          ", vx_i16, 0, hn::Lanes(di16));
    hn::Print(di16, "yi16          ", vy_i16, 0, hn::Lanes(di16));
    puts("");
    hn::Print(di16, "iResult       ", iResult,   0, hn::Lanes(di16));
    hn::Print(df16, "fResult       ", fResult,   0, hn::Lanes(df16));

}

} // namespace play

HWY_AFTER_NAMESPACE();

/*

    const hn::Half< hn::RepartitionToNarrow<hn::ScalableTag<float>> > half;

    const auto vx_f16 = 
        hn::Combine(df16
            ,hn::DemoteTo(half, hn::ConvertTo(df32, hn::Load(di32, &x32[0] )))
            ,hn::DemoteTo(half, hn::ConvertTo(df32, hn::Load(di32, &y32[0] )))
        );

    const auto vy_f16 = 
        hn::Combine(df16
            ,hn::DemoteTo(half, hn::ConvertTo(df32, hn::Load(di32, &y32[0] )))
            ,hn::DemoteTo(half, hn::ConvertTo(df32, hn::Load(di32, &x32[0] )))
        );

    //const auto vy_f16 = hn::DemoteTo(half, hn::ConvertTo(df, hn::Load(di32, &y32[0] )));

*/