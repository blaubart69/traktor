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
    const hn::RepartitionToWide<decltype(di16)> diw;

	printf("lanes float  : %lu\n", hn::Lanes(df));
    printf("lanes int32_t: %lu\n", hn::Lanes(di32));
    printf("lanes int16_t: %lu\n", hn::Lanes(di16));

	int16_t x[16] __attribute__ ((aligned (64)));
	int16_t y[16] __attribute__ ((aligned (64)));

	for (int i=0; i < 16; i++)
	{
		x[i] = i * i;
		y[i] = i * i;
	}

    x[0] = y[0] = 0x7FFF;
    x[2] = y[2] = 0xFFFF;
    x[4] = 1; y[4] = -1;
    x[1] = -1; y[1] = 0;
    y[3] = 0;

    const auto vx = hn::Load(di16, &x[0] );
    const auto vy = hn::Load(di16, &y[0] );

    auto xDy = hn::Div(vx,vy);


    hn::Print(di16, "x          ", vx, 0, hn::Lanes(di16));
    hn::Print(di16, "y          ", vy, 0, hn::Lanes(di16));
   	hn::Print(di16, "Div x y    ", xDy,0, hn::Lanes(di16));

}

} // namespace play

HWY_AFTER_NAMESPACE();