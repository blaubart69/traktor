#include <opencv2/imgproc.hpp>

#include "detect.h"
#include "pipeline/ImagePipeline.hpp"
#include "calculate.h"

void init_status_bar(int frame_columns, int frame_type, std::unique_ptr<cv::Mat>& status_bar)
{
    if ( status_bar == nullptr )
    {
        status_bar = std::make_unique<cv::Mat>( cv::Mat::zeros(20, frame_columns, frame_type ) );
    }
    else if ( status_bar->cols != frame_columns ) {
        status_bar.reset();
        status_bar = std::make_unique<cv::Mat>( cv::Mat::zeros(20, frame_columns, frame_type ) );
    }
}

static const cv::Size GaussKernel(5,5);

const int erosion_size = 2;
const cv::Mat erodeKernel = cv::getStructuringElement( cv::MorphShapes::MORPH_RECT,
                    cv::Size ( 2*erosion_size + 1, 2*erosion_size + 1 ),
                    cv::Point(   erosion_size    ,   erosion_size     ) );

const int dilate_size = 2;
const cv::Mat dilateKernel = cv::getStructuringElement( cv::MorphShapes::MORPH_RECT,
                    cv::Size( 2*dilate_size + 1, 2*dilate_size + 1 ),
                    cv::Point( dilate_size, dilate_size ) );

const cv::Scalar RED    = cv::Scalar(0,0,255);
const cv::Scalar BLUE   = cv::Scalar(255,0,0);
const cv::Scalar GREEN  = cv::Scalar(0,255,0);
const cv::Point  POINT_MINUS1 = cv::Point(-1,-1);

size_t mat_byte_size(const cv::Mat& mat)
{
    return mat.total() * mat.elemSize();
}

void find_contours(const ImageSettings& settings, DetectCounter* stats, const cv::Mat& cameraFrame, Contoures* structures, const bool showWindows)
{
    static cv::Mat img_inRange;
    static cv::Mat img_GaussianBlur;
    static cv::Mat img_eroded_dilated;

    static cv::Mat in;
    static cv::Mat out;

    auto start = std::chrono::high_resolution_clock::now();

    cv::cvtColor    (cameraFrame, out, cv::COLOR_BGR2HSV );                                      stats->cvtColor     += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(cameraFrame);   std::swap(in,out);                                                      
    cv::GaussianBlur(in, out, GaussKernel, 0);                                                   stats->GaussianBlur += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(in);            std::swap(in,out); if (showWindows) { in.copyTo(img_GaussianBlur); }    
    cv::inRange     (in, settings.colorFrom, settings.colorTo, out );                            stats->inRange      += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(in);            std::swap(in,out); if (showWindows) { in.copyTo(img_inRange); }         
    cv::erode       (in, out, erodeKernel, POINT_MINUS1, settings.erode_iterations  );           stats->erode        += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(in);            std::swap(in,out);                                                      
    cv::dilate      (in, out, dilateKernel,POINT_MINUS1, settings.dilate_iterations );           stats->dilate       += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(in);                               if (showWindows) { out.copyTo(img_eroded_dilated); } 
    cv::findContours(out, structures->all_contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE); stats->findContours += trk::get_duration(&start);  stats->frame_bytes += mat_byte_size(out);                                                                                    

    /* DEBUG purpose
    if (showWindows)
    {
        cv::imshow("inRange",       img_inRange );
        cv::imshow("GaussianBlur",  img_GaussianBlur );
        cv::imshow("eroded_dilated",img_eroded_dilated );
        cv::waitKey(1);
    }*/
}

void calc_center_of_contour(const std::vector<cv::Point2i>& points, cv::Point* center)
{
    cv::Moments M = cv::moments(points);
    center->x = int(M.m10 / M.m00);
	center->y = int(M.m01 / M.m00);
}

void calc_centers_of_contours(Contoures* found, const int minimalContourArea)
{
    for ( std::size_t i=0; i < found->all_contours.size(); ++i )
    {
        const auto& contour = found->all_contours[i];
        if ( cv::contourArea(contour) > minimalContourArea )
        {
            //
            // calc center and add it to centers
            //
            cv::Point2i centerPoint;
            calc_center_of_contour(contour, &centerPoint);
            //found->centers.emplace_back( centerPoint.x, centerPoint.y );
            found->centers.emplace_back((int)i, centerPoint.x, centerPoint.y);
            //
            // remember index of found center to access the corresponding contour afterwards
            //
            //found->centers_contours_idx.push_back(i);
        }
    }
}

/*
 * y = x * ( 10 / 2 )   
 * x = y / ( 10 / 2 )
 * Wolfram Alpha: plot [y = (-x+2) * (10/2), {x,-2,2}]
 */

HARROW_DIRECTION get_harrow_direction(const bool is_within_threshold, const int avg_threshold)
{
    HARROW_DIRECTION direction;

    if ( is_within_threshold ) {
        direction = HARROW_DIRECTION::STOP;
    }
    else if ( avg_threshold > 0 ) {
        direction = HARROW_DIRECTION::RIGHT;
    }
    else {
        direction = HARROW_DIRECTION::LEFT;
    }

    return direction;
}

void detect_main(Workitem* work, DetectContext* ctx)
{
    work->detect_result.reset();

    if ( ! work->isPictureFromCamera )
    {
        work->detect_result.state = DETECT_STATE::NO_VALID_FRAME;
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();

    find_contours(
            ctx->shared->detectSettings.getImageSettings()
        ,   ctx->stats
        ,  work->frame               
        ,&(work->detect_result.contoures)
        ,   ctx->showDebugWindows );                                                           

    auto refline_settings = ctx->shared->detectSettings.getReflineSettings();

    calc_centers_of_contours(
        &(work->detect_result.contoures)
        , ctx->shared->detectSettings.getImageSettings().minimalContourArea);

    if ( work->detect_result.contoures.centers.size() == 0 )
    {
        work->detect_result.state = DETECT_STATE::NOTHING_FOUND;
    }
    else if ( ! calc_average_delta(
        refline_settings
        , work->frame.rows
        , &(work->detect_result.contoures)
        , &(work->detect_result.avg_delta_px)
        , ctx->stats) )
    {
        work->detect_result.state = DETECT_STATE::NO_PLANTS_WITHIN_LINES;
    }
    else
    {
        work->detect_result.state           = DETECT_STATE::SUCCESS;
        work->detect_result.is_in_threshold = std::abs(work->detect_result.avg_delta_px) <= refline_settings.rowThresholdPx;
        
        if (    (ctx->harrow != nullptr)                         
            &&  (ctx->shared->harrowLifted.load() == false) )
        {
            HARROW_DIRECTION direction = get_harrow_direction(work->detect_result.is_in_threshold, work->detect_result.avg_delta_px);
            ctx->harrow->move(direction, "detect");
        }
    }

    ctx->stats->overall += trk::get_duration(&start);
    ctx->stats->frames++;
}