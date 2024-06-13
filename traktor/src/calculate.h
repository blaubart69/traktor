#pragma once

struct CoordPoint {
    int x;
    int y;
};

bool calc_average_delta(const ReflinesSettings& refSettings, const int frame_rows, Contoures* contoures, int* avg_deltaPx, DetectCounter* stats);
