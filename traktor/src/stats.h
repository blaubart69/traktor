#pragma once

#include <atomic>

struct CameraCounter
{
    uint32_t                   frames{0};

    CameraCounter& operator= (const CameraCounter& rhs);
};

struct DetectCounter
{
    uint32_t                   frames{};
    uint64_t                   frame_bytes{};

    std::atomic<uint32_t>      plants_in_picture{};
    std::atomic<uint32_t>      plants_out_range{};
    std::atomic<uint32_t>      plants_in_tolerance{};
    std::atomic<uint32_t>      plants_out_tolerance{};
    
    std::chrono::nanoseconds   overall{};
    std::chrono::nanoseconds   cvtColor{};
    std::chrono::nanoseconds   GaussianBlur{};
    std::chrono::nanoseconds   inRange{};
    std::chrono::nanoseconds   erode{};
    std::chrono::nanoseconds   dilate{};
    std::chrono::nanoseconds   findContours{};

    DetectCounter& operator= (const DetectCounter& rhs);
};

struct EncodeCounter
{
    std::atomic<uint64_t>   images_sent{0};
    std::atomic<uint64_t>   bytes_sent{0};
    std::atomic<uint64_t>   draw{0};
    std::atomic<uint64_t>   overall{0};

    EncodeCounter& operator= (const EncodeCounter& rhs);
};

struct Counter 
{
    CameraCounter camera;
    DetectCounter detect;
    EncodeCounter encode;
};

class Stats
{
    public:
        CameraCounter camera;
        DetectCounter detect;
        EncodeCounter encode;

        Counter diff;

        void tick();

        const static std::chrono::seconds pause;

    private:

        Counter     last;
};