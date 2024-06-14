#pragma once

#include "shared.h"
#include "stats.h"
#include "harrow.h"

enum DETECT_STATE
{
    // HARROW_LIFTED,
    NO_VALID_FRAME,
    NOTHING_FOUND,
    NO_PLANTS_WITHIN_LINES,
    SUCCESS
};

typedef cv::Point_<int16_t> Point2i16;

struct Center
{
    int contours_idx;
    Point2i16 point;
    bool within_threshold;
    bool within_row_range;

    Center(int idx, int16_t x, int16_t y)
    {
        contours_idx = idx;
        point.x = x;
        point.y = y;
        within_threshold = false;
        within_row_range = true;
    }
};

struct Contoures
{
    std::vector< std::vector<cv::Point> >   all_contours;
    std::vector<Center> centers;

    void clearCenters()
    {
        centers.clear();
    }
};

struct DetectResult
{
    DETECT_STATE state;
    Contoures    contoures;
    bool         is_in_threshold;
    int          avg_delta_px;

    void reset() 
    {
        contoures.clearCenters();
    }
};

struct DetectContext
{
    DetectCounter* stats;
    Shared* shared;
    Harrow* harrow;
    bool showDebugWindows = false;
    std::unique_ptr<cv::Mat> status_bar = nullptr;

    DetectContext(DetectCounter* stats, Shared* shared, Harrow* harrow)
    : stats(stats)
    , shared(shared)
    , harrow(harrow)
    {}
};

