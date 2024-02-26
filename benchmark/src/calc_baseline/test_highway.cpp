
#include "hwy/highway.h"
#include "hwy/print-inl.h"

#include "calc_baseline.h"

namespace deltapx {

	void run_hwy_calc_delta_pixels(
	  const size_t	size
  	, const int32_t* __restrict x_screen
  	, const int32_t* __restrict y_screen
  	, const CalcSettings& settings
  	, int32_t *delta_pixels );
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
        , 25   // refSettings.rowSpacingPx
        , 0     // refSettings.offset
        , 1    // row_count
    );

	int32_t x[16] __attribute__ ((aligned (64)));
	int32_t y[16] __attribute__ ((aligned (64)));

	for (int i=0; i < 16; i++)
	{
		x[i] = 240 + i * 10;
		y[i] = 479;
	}

	int32_t delta_pixels;
	x[0] = INT32_MAX;
	x[15] = INT32_MIN;
	deltapx::run_hwy_calc_delta_pixels(16, x, y, calcSettings, &delta_pixels);
}
