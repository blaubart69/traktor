#pragma once

#include "shared.h"
#include "stats.h"
#include "harrow.h"

enum DETECT_STATE
{
    HARROW_LIFTED,
    NO_VALID_FRAME,
    NOTHING_FOUND,
    NO_PLANTS_WITHIN_LINES,
    SUCCESS
};

struct Center
{
    int contours_idx;
    cv::Point2i point;
    bool within_threshold;

    Center(int idx, int x, int y)
    {
        contours_idx = idx;
        point.x = x;
        point.y = y;
        within_threshold = false;
    }
};

struct Contoures
{
    std::vector< std::vector<cv::Point> >   all_contours;
    //std::vector<int>                        centers_contours_idx;
    //std::vector<cv::Point2i>                centers;
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
    float        avg_threshold;

    void reset() 
    {
        contoures.clearCenters();
    }
};

struct DetectContext
{
    Shared* shared;
    Stats* stats;
    Harrow* harrow;
    bool showDebugWindows = false;
    std::unique_ptr<cv::Mat> status_bar = nullptr;

    DetectContext(Stats* stats, Shared* shared, Harrow* harrow)
    : stats(stats)
    , shared(shared)
    , harrow(harrow)
    {}
};
