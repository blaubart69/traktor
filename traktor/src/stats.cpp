#include <chrono>

#include "stats.h"

const std::chrono::seconds Stats::pause{2};

template<class T>
void assign_relaxed( std::atomic<T>& lhs, const std::atomic<T>& rhs )
{
    lhs.store( rhs.load(std::memory_order_relaxed), std::memory_order_relaxed );
}

void Stats::tick()
{
    diff.camera.frames              = camera.frames         - last.camera.frames;

    diff.detect.frames              = detect.frames         - last.detect.frames;
    diff.detect.frame_bytes         = detect.frame_bytes    - last.detect.frame_bytes;

    diff.detect.plants_in_picture    = detect.plants_in_picture    - last.detect.plants_in_picture;
    diff.detect.plants_out_range     = detect.plants_out_range     - last.detect.plants_out_range;
    diff.detect.plants_out_tolerance = detect.plants_out_tolerance - last.detect.plants_out_tolerance;
    diff.detect.plants_in_tolerance  = detect.plants_in_tolerance  - last.detect.plants_in_tolerance;

    if (diff.detect.frames > 0)
    {
        diff.detect.cvtColor     = (detect.cvtColor     - last.detect.cvtColor     ); // / diff.detect.frames;
        diff.detect.GaussianBlur = (detect.GaussianBlur - last.detect.GaussianBlur ); // / diff.detect.frames;
        diff.detect.inRange      = (detect.inRange      - last.detect.inRange      ); // / diff.detect.frames;
        diff.detect.erode        = (detect.erode        - last.detect.erode        ); // / diff.detect.frames;
        diff.detect.dilate       = (detect.dilate       - last.detect.dilate       ); // / diff.detect.frames;
        diff.detect.findContours = (detect.findContours - last.detect.findContours ); // / diff.detect.frames;
        diff.detect.overall      = (detect.overall      - last.detect.overall      ); // / diff.detect.frames;
    }

    diff.encode.bytes_sent  = encode.bytes_sent  - last.encode.bytes_sent;
    diff.encode.images_sent = encode.images_sent - last.encode.images_sent;

    if (diff.encode.images_sent > 0)
    {
        diff.encode.draw    = (encode.draw.load()    - last.encode.draw.load());    // / diff.encode.images_sent;
        diff.encode.overall = (encode.overall.load() - last.encode.overall.load()); /// diff.encode.images_sent;
    }

    this->last.camera = this->camera;
    this->last.detect = this->detect;
    this->last.encode = this->encode;
}

CameraCounter& CameraCounter::operator= (const CameraCounter& rhs)
{
    this->frames = rhs.frames;

    return *this;
}

DetectCounter& DetectCounter::operator= (const DetectCounter& rhs)
{
    this->cvtColor = rhs.cvtColor;
    this->dilate = rhs.dilate;
    this->erode = rhs.erode;
    this->findContours = rhs.findContours;
    this->frame_bytes = rhs.frame_bytes;
    this->frames = rhs.frames;
    this->GaussianBlur = rhs.GaussianBlur;
    this->inRange = rhs.inRange;
    this->overall = rhs.overall;

    assign_relaxed(this->plants_in_picture,     rhs.plants_in_picture);
    assign_relaxed(this->plants_out_range,      rhs.plants_out_range);
    assign_relaxed(this->plants_out_tolerance,  rhs.plants_out_tolerance);
    assign_relaxed(this->plants_in_tolerance,   rhs.plants_in_tolerance);

    return *this;
}

EncodeCounter& EncodeCounter::operator= (const EncodeCounter& rhs)
{
    assign_relaxed(this->bytes_sent,  rhs.bytes_sent);
    assign_relaxed(this->draw,        rhs.draw);
    assign_relaxed(this->images_sent, rhs.images_sent);
    assign_relaxed(this->overall,     rhs.overall);

    return *this;
}