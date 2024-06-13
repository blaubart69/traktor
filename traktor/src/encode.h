#pragma once

#include "shared.h"
#include "stats.h"

struct EncodeContext
{
    EncodeCounter* stats;
    Shared* shared;
    std::unique_ptr<cv::Mat> status_bar = nullptr;
    std::vector<uchar> jpeg_buffer;
    std::function<bool(std::vector<uchar>&, uint64_t* bytes_sent)> sendJPEGbytes;
    const DetectCounter* detectCounter;

    EncodeContext(
          EncodeCounter* stats
        , const DetectCounter* detectCounterStats
        , Shared* shared
        , std::function<bool(std::vector<uchar>&, uint64_t* bytes_sent)> sendJPEGbytes
        )
    : stats(stats)
    , shared(shared)
    , sendJPEGbytes(sendJPEGbytes)
    , detectCounter(detectCounterStats)
    {}
};

