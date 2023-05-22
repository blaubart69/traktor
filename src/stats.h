#pragma once

struct CameraCounter
{
    std::atomic<int>        camera_frames{0};
    std::atomic<uint64_t>   camera_read_ns{0};
};

struct DetectCounter
{
    std::atomic<int>        fps{0};
    std::atomic<uint64_t>   detect_overall_ns{0};
    std::atomic<uint64_t>   prepare_ns{0};
    std::atomic<uint64_t>   findContours_ns{0};

    std::atomic<uint64_t>   prepare_cvtColor_ns{0};
    std::atomic<uint64_t>   prepare_GaussianBlur_ns{0};
    std::atomic<uint64_t>   prepare_inRange_ns{0};
    std::atomic<uint64_t>   prepare_erode_ns{0};
    std::atomic<uint64_t>   prepare_dilate_ns{0};
    std::atomic<uint64_t>   frame_bytes_processed{0};
};

struct EncodeCounter
{
    std::atomic<uint64_t>   jpeg_images_sent{0};
    std::atomic<uint64_t>   jpeg_bytes_sent{0};
    std::atomic<int>        jpeg_threads_running{0};
};

struct Stats
{
    CameraCounter camera;
    DetectCounter detect;
    EncodeCounter encode;

    void calculate(int secondsPassed);

};