#pragma once

struct Result {
    int64_t delta_pixels_sum = 0;
    size_t in_range = 0;
    size_t out_range = 0;
    size_t points = 0;
};

Result calc_all_points(size_t frames, int rows);
