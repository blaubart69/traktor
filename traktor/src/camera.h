#pragma once

#include <opencv2/videoio.hpp>

#include "shared.h"

class CameraContext
{
    public:
    
    CameraCounter*      stats;
    const Options*      options;
    Shared*             shared;
    std::unique_ptr<cv::VideoCapture> capture;

    int                 errorCount;
    int delay_for_realtime_video_millis;

    CameraContext(CameraCounter* stats, const Options* options, Shared* shared)
    : stats(stats)
    , options(options)
    , shared(shared)
    , capture(nullptr)    
    , errorCount(0)
    , delay_for_realtime_video_millis(0)
    {}
};

