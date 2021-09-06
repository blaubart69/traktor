//#include <atomic>
//#include <optional>
//
////#include <opencv2/opencv.hpp>
//#include <opencv2/core.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2/highgui.hpp>
//
#include "shared.h"
#include "stats.h"
#include "util.h"
//
static const cv::Size GaussKernel(5,5);

const int erosion_size = 2;
const cv::Mat erodeKernel = cv::getStructuringElement( cv::MorphShapes::MORPH_RECT,
                    cv::Size ( 2*erosion_size + 1, 2*erosion_size + 1 ),
                    cv::Point(   erosion_size    ,   erosion_size     ) );

const int dilate_size = 2;
const cv::Mat dilateKernel = cv::getStructuringElement( cv::MorphShapes::MORPH_RECT,
                    cv::Size( 2*dilate_size + 1, 2*dilate_size + 1 ),
                    cv::Point( dilate_size, dilate_size ) );

const cv::Scalar RED   = cv::Scalar(0,0,255);
const cv::Scalar BLUE  = cv::Scalar(255,0,0);
const cv::Scalar GREEN = cv::Scalar(0,255,0);

cv::Mat tmp;
cv::Mat img_inRange;
cv::Mat img_GaussianBlur;
cv::Mat img_threshold;
cv::Mat img_eroded_dilated;

struct Structures
{
    std::vector< std::vector<cv::Point> >   all_contours;
    std::vector<cv::Point2i>                centers;
    std::vector<int>                        centers_contours_idx;

    void clearCenters()
    {
        centers.clear();
        centers_contours_idx.clear();
    }
};

/* 2021-08-23 Spindler
    nice try but doesn't work as expected
void calc_center(const std::vector<cv::Point2i>& points, cv::Point* center)
{
    size_t sum_x = 0;
    size_t sum_y = 0;

    for ( const auto& p : points )
    {
        sum_x += p.x;
        sum_y += p.y;
    }

    center->x = sum_x / points.size();
    center->y = sum_y / points.size();
}*/

size_t mat_byte_size(const cv::Mat& mat)
{
    return mat.total() * mat.elemSize();
}

void calc_center2(const std::vector<cv::Point2i>& points, cv::Point* center)
{
    cv::Moments M = cv::moments(points);
    center->x = int(M.m10 / M.m00);
	center->y = int(M.m01 / M.m00);
}

void drawContoursAndCenters(cv::InputOutputArray frame, const DetectSettings &settings, const Structures &found)
{
    for ( auto const& c : found.centers )
    {
        cv::drawMarker(frame, c, BLUE, cv::MarkerTypes::MARKER_TILTED_CROSS, 20, 2 );
    }

    for ( int contour_idx_to_draw : found.centers_contours_idx)
    {
        cv::drawContours( frame, found.all_contours, contour_idx_to_draw, BLUE, 2 );
    }
}

/***
 * 2021-08-30 Spindler
 *   the lines left and right:
 *      start point: botton of the frame. +/- x_spacing
 *      end   point: Fluchtpunkt(!) des Bildes. X ist mittig. Y ist ein Wert "oberhalb" des Bildes. Minus Y!
 ***/
void drawRowLines(cv::InputOutputArray frame, const DetectSettings &settings)
{
    const cv::Scalar rowLineColor         (150,255,255);
    const cv::Scalar rowToleranceLineColor( 60,255,128);

    const int x_half = settings.frame_cols / 2;
    const int y_max  = settings.frame_rows;

    const int       deltapx = settings.rowThresholdPx;
    const cv::Point Fluchtpunkt(x_half, -settings.rowPerspectivePx);
    
    cv::line(frame, cv::Point(x_half,0), cv::Point(x_half,y_max), rowLineColor, 3 );

    cv::line(frame, cv::Point(x_half-deltapx,y_max), Fluchtpunkt, rowToleranceLineColor, 2 );
    cv::line(frame, cv::Point(x_half+deltapx,y_max), Fluchtpunkt, rowToleranceLineColor, 2 );

    //
    // Reihen rechts und links der Mittellinie zeichnen
    //
    if ( settings.rowCount > 1 )
    {
        for ( int r=settings.rowCount-1, x_spacing = settings.rowSpacingPx; r > 0; r-=2, x_spacing += settings.rowSpacingPx )
        {
            // rows
            cv::line(frame, cv::Point(x_half - x_spacing,y_max),            Fluchtpunkt, rowLineColor, 3 );
            cv::line(frame, cv::Point(x_half + x_spacing,y_max),            Fluchtpunkt, rowLineColor, 3 );
            // row tolerance
            cv::line(frame, cv::Point(x_half - x_spacing - deltapx ,y_max), Fluchtpunkt, rowToleranceLineColor, 2 );
            cv::line(frame, cv::Point(x_half - x_spacing + deltapx ,y_max), Fluchtpunkt, rowToleranceLineColor, 2 );
            cv::line(frame, cv::Point(x_half + x_spacing - deltapx ,y_max), Fluchtpunkt, rowToleranceLineColor, 2 );
            cv::line(frame, cv::Point(x_half + x_spacing + deltapx ,y_max), Fluchtpunkt, rowToleranceLineColor, 2 );
        }
    }
}

void find_contours(cv::Mat& cameraFrame, const DetectSettings& settings, cv::Mat& outputFrame, Structures* structures, Stats* stats, const bool showWindows)
{
    cv::Mat* in  = &tmp;
    cv::Mat* out = &outputFrame;

    auto start = std::chrono::high_resolution_clock::now();

    cv::cvtColor    (cameraFrame, *out, cv::COLOR_BGR2HSV );                                 std::swap(in,out);                                                         stats->frame_bytes_processed += mat_byte_size(cameraFrame);
    cv::GaussianBlur(*in, *out, GaussKernel, 0);                                             std::swap(in,out); if (showWindows) { in->copyTo(img_GaussianBlur); }      stats->frame_bytes_processed += mat_byte_size(*in);
    cv::inRange     (*in, settings.colorFrom, settings.colorTo, *out );                      std::swap(in,out); if (showWindows) { in->copyTo(img_inRange); }           stats->frame_bytes_processed += mat_byte_size(*in);
    //cv::threshold   (*in, *out, 0, 255, cv::THRESH_BINARY );                                 std::swap(in,out); if (showWindows) { in->copyTo(img_threshold);}        stats->frame_bytes_processed += mat_byte_size(*in);
    cv::erode       (*in, *out, erodeKernel, cv::Point(-1,-1), settings.erode_iterations  ); std::swap(in,out);                                                         stats->frame_bytes_processed += mat_byte_size(*in);
    cv::dilate      (*in, *out, dilateKernel,cv::Point(-1,-1), settings.dilate_iterations );                    if (showWindows) { out->copyTo(img_eroded_dilated); }   stats->frame_bytes_processed += mat_byte_size(*in);

    stats->prepare_ns += trk::getDuration_ns(&start);
    
    cv::findContours(*out, structures->all_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);       stats->findContours_ns  += trk::getDuration_ns(&start);         stats->frame_bytes_processed += mat_byte_size(*out);
}
void calc_centers(Structures* found, const int minimalContourArea)
{
    found->clearCenters();

    cv::Point2i centerPoint;

    for ( int i=0; i < found->all_contours.size(); ++i )
    {
        const auto& contour = found->all_contours[i];
        if ( cv::contourArea(contour) > minimalContourArea )
        {
            //
            // calc center and add it to centers
            //
            calc_center2(contour, &centerPoint);
            found->centers.emplace_back( centerPoint.x, centerPoint.y );
            //
            // remeber index of found center to access the corresponding contour afterwards
            //
            found->centers_contours_idx.push_back(i);
        }
    }
}
/*
 * y = x * ( 10 / 2 )   
 * x = y / ( 10 / 2 )
 * Wolfram Alpha: plot [y = (-x+2) * (10/2), {x,-2,2}]
 */
bool find_point_on_nearest_refline(const cv::Point& plant, const DetectSettings& settings, cv::Point* nearest_refLine_point, float* deltaPx, int* refLines_distance_px)
{
    const int frame_x_half   = (settings.frame_cols  / 2);
    const int half_row_count = (settings.rowCount-1) / 2;

    const int   plant_x_offset_from_middle = plant.x - frame_x_half;
    const float plant_x_abs_offset_from_0  = abs( plant_x_offset_from_middle );
    const float plant_y                    = settings.frame_rows - plant.y;

          int   refline_x_width  = settings.rowSpacingPx;
    const float refline_y_height = settings.frame_rows + settings.rowPerspectivePx; // Höhe Fluchtpunkt

    int found_row_idx = -1;
    float px_delta_to_row;

    float x_ref1 = 0;    // start with the middle
    float x_ref2;

    int refline_x_found;

    for ( int r=0; r < half_row_count; ++r, refline_x_width += settings.rowSpacingPx)
    {
        const float refline_steigung = refline_y_height / (float)refline_x_width;
        x_ref2 = -plant_y / refline_steigung ;
        x_ref2 +=  refline_x_width;

        if ( x_ref1 < plant_x_abs_offset_from_0 && plant_x_abs_offset_from_0 < x_ref2 )
        {
            // plant is between rows
            // which row is closer?
            float px_delta_to_row1 = abs(x_ref1 - plant_x_abs_offset_from_0);
            float px_delta_to_row2 = abs(x_ref2 - plant_x_abs_offset_from_0);

            if ( px_delta_to_row1 < px_delta_to_row2 )
            {
                nearest_refLine_point->x = (int)x_ref1;
                *deltaPx = px_delta_to_row1;
            }
            else
            {
                nearest_refLine_point->x = (int)x_ref2;
                *deltaPx = px_delta_to_row2;
            }

            nearest_refLine_point->y = plant.y;
            *refLines_distance_px    = (int)x_ref2 - (int)x_ref1;

            if ( plant_x_offset_from_middle < 0 )
            {
                // plant is on the left side. Mirror the x value to minus
                nearest_refLine_point->x = -nearest_refLine_point->x;
            }
            // shift it to the right pixel-image-value
            nearest_refLine_point->x += frame_x_half;

            return true;
        }
        else
        {
            // try next rows
            x_ref1 = x_ref2;
        }
    }
    
    return false;
}

void calc_deltas_to_ref_lines(Structures* structures, const DetectSettings& settings, cv::Mat& frame)
{
    const int x_half   = (settings.frame_cols  / 2);
    const cv::Point Fluchtpunkt(x_half, -settings.rowPerspectivePx);
    const float threshold_percent = (float)settings.rowThresholdPx / (float)settings.rowSpacingPx;

    cv::Point   nearest_refLine_point;
    float       sum_threshold = 0;
    for ( int i=0; i < structures->centers.size(); ++i )
    {
        const cv::Point& plant = structures->centers[i];
        // hier bitte mit Magie befüllen!
        int refLines_distance_px;
        float deltaPx;
        if ( find_point_on_nearest_refline(plant, settings, &nearest_refLine_point, &deltaPx, &refLines_distance_px) )
        {
            const float threshold = (float)deltaPx / (float)refLines_distance_px;
            sum_threshold += nearest_refLine_point.x > x_half ? threshold : -threshold;
            
            const cv::Scalar& plant_color = threshold < threshold_percent ? BLUE : RED;
            //cv::line(frame, plant, nearest_refLine_point, color_delta_line, 2);
            cv::drawMarker  ( frame, plant , plant_color, cv::MarkerTypes::MARKER_CROSS, 20, 2 );
            cv::drawContours( frame, structures->all_contours, structures->centers_contours_idx[i], plant_color, 1 );
        }
    }
    const float avg_threshold = sum_threshold / (float)structures->centers.size();
    int x_overall_threshold_px = (float)avg_threshold * (float)settings.rowSpacingPx;

    const cv::Scalar& color_overall_delta = abs(x_overall_threshold_px) < settings.rowThresholdPx ? GREEN : RED;
    //cv::line(frame, cv::Point(x_overall_threshold_px + x_half, settings.frame_rows), Fluchtpunkt, color_overall_delta_line, 3 );

    cv::Mat arrow = cv::Mat::zeros(20, frame.cols, frame.type() );
    const int delta_status_px = (float)avg_threshold * (float)x_half;
    cv::rectangle(arrow, cv::Point(x_half,0), cv::Point(x_half + delta_status_px,arrow.rows), color_overall_delta, 8);
    frame.push_back(arrow);
}

void thread_detect(Shared* shared, Stats* stats, bool showDebugWindows)
{
    printf("I: thread detect running\n");

    Structures structures;
    int idx_doubleBuffer = 0;

    for (;;)
    {
        //
        // get new frame from camera buffer
        //
        int frameReadyIdx;
        while ( (frameReadyIdx=std::atomic_exchange( &(shared->frame_buf_slot), -1)) == -1 )
        {
            // wait for a frame ready in the frameBuffer
            std::unique_lock<std::mutex> lk(shared->camera_frame_ready_mutex);
            shared->camera_frame_ready.wait(lk, [&shared]() { return shared->frame_buf_slot.load() != -1; });
        }
        cv::Mat& cameraFrame = shared->frame_buf[frameReadyIdx];
        cv::Mat& outFrame    = shared->analyzed_frame_buf[idx_doubleBuffer];
        //
        // lock an output buffer
        //
        {
            std::lock_guard<std::mutex> lk(shared->analyzed_frame_buf_mutex[idx_doubleBuffer]);
            //
            // 1. detect
            //
            auto start = std::chrono::high_resolution_clock::now();
            find_contours(
                  cameraFrame               
                , shared->detectSettings    
                , outFrame                  
                , &structures
                , stats
                , showDebugWindows );                                                           stats->detect_overall_ns += trk::getDuration_ns(&start);

            calc_centers            (&structures, shared->detectSettings.minimalContourArea);   stats->calcCenters_ns += trk::getDuration_ns(&start);

            cameraFrame.copyTo(outFrame);
            calc_deltas_to_ref_lines(&structures, shared->detectSettings, outFrame);
            //
            // draw 
            //
            //drawContoursAndCenters(outFrame, shared->detectSettings, structures );
            drawRowLines          (outFrame, shared->detectSettings );

            stats->draw_ns += trk::getDuration_ns(&start);
        }
        if (showDebugWindows)
        {
            cv::imshow("inRange",       img_inRange );
            cv::imshow("GaussianBlur",  img_GaussianBlur );
            //cv::imshow("threshold",     img_threshold );
            cv::imshow("eroded_dilated",img_eroded_dilated );
            cv::imshow("drawContours",  shared->analyzed_frame_buf[idx_doubleBuffer] );
            cv::waitKey(1);
        }
        stats->fps++;
        //
        // set index for other thread
        //
        shared->analyzed_frame_ready_idx.store(idx_doubleBuffer);
        //
        // notify other thread(s) about ready buffer
        // 2021-08-08 Spindler (Moz'ens Geburtstag)
        //   Still don't know if a lock is required to do .notify_xxx()
        //
        {
            std::unique_lock<std::mutex> ul(shared->analyzed_frame_ready_mutex);
            shared->analyzed_frame_encoded_to_JPEG.store(false);
            shared->analyzed_frame_ready.notify_all();
        }
        
        idx_doubleBuffer = 1 - idx_doubleBuffer;

        if (shared->shutdown_requested.load())
        {
            printf("I: thread detect quitting...\n");
            break;
        }
    }
}