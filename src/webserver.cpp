//#include <stdio.h>
//#include <vector>
//#include <functional>
//
//#include "httplib.h"

#include "shared.h"
#include "util.h"
#include "pipeline/ImagePipeline.hpp"
#include "encode.h"

using namespace httplib;

void thread_send_jpeg(Shared* shared, std::function<bool(std::vector<uchar>&)> sendJPEGbytes);

WORKER_RC encode_main(Workitem* work, EncodeContext* ctx);

/*
 * 2023-05-21 Spindler
 *  in all the URL callbacks sample: "svr->Get("/video", [=](const Request &req, Response &res)"
 *  the "[=]" is important! Giving [&] crashes the program.
 *  Don't know right now why.
 */

void URL_video(httplib::Server* svr, Shared* shared, ImagePipeline* pipeline, Stats* stats)
{
    svr->Get("/video", [=](const Request &req, Response &res) {

        static const std::string boundary("--Ba4oTvQMY8ew04N8dcnM\r\nContent-Type: image/jpeg\r\n\r\n");
        static const std::string CRLF("\r\n");

        res.set_content_provider(
            "multipart/x-mixed-replace;boundary=Ba4oTvQMY8ew04N8dcnM", // Content type
            [&](size_t offset, DataSink &sink) {
                //
                // begin of JPEG streaming
                //
                EncodeContext ctx(stats, shared,
                    [&sink,&stats](std::vector<unsigned char>& jpegBytes) {
                        // yield(b'--Ba4oTvQMY8ew04N8dcnM\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(encodedImage) + b'\r\n')
                        jpegBytes.insert(jpegBytes.end(), CRLF.begin(), CRLF.end());
                        if ( !sink.write( boundary.data(), boundary.length() ) )
                        {
                            return false;
                        }
                        else if ( !sink.write( (char*)jpegBytes.data(), jpegBytes.size() ) )
                        {
                            return false;
                        }

                        stats->jpeg_bytes_sent += boundary.length() + jpegBytes.size();

                        return true;
                    });

                pipeline->run_encode_3( encode_main, &ctx );

                return true; // return 'false' if you want to cancel the process.
            },
            [](bool success) {}
        );
    });

}

void URL_applyChanges(httplib::Server* svr, DetectSettings* detect_settings)
{
    svr->Post("/applyChanges", [=](const Request &req, Response &res)
    {
        try
        {
            detect_settings->set_fromJson(req.body);

            nlohmann::json data = nlohmann::json::parse(req.body);

            const std::string jsonData = data.dump();
            trk::write_to_file("./detect/lastSettings.json", jsonData);
            res.status = 200;
            printf("I: applyChanges: %s\n", jsonData.c_str());
        }
        catch(const std::exception& e)
        {
            fprintf(stderr, "/applyChanges: %s\n", e.what());
            res.status = 500;
            res.set_content(e.what(), "text/plain");
        }
    });
}

void URL_current(httplib::Server* svr, DetectSettings* settings)
{
    svr->Get("/current", [=](const Request &req, Response &res) {

        nlohmann::json data;
        {
            const auto from = settings->getImageSettings().colorFrom;
            const auto to   = settings->getImageSettings().colorTo;
            data["colorFrom"] = { (int)from[0], (int)from[1], (int)from[2] };
            data["colorTo"]   = { (int)to  [0], (int)to  [1], (int)to  [2] };
        }
        data["erode"]               = settings->getImageSettings().erode_iterations;
        data["dilate"]              = settings->getImageSettings().dilate_iterations;
        data["minimalContourArea"]  = settings->getImageSettings().minimalContourArea;

        data["maxRows"]             = settings->getReflineSettings().rowMax;
        data["rowThresholdPx"]      = settings->getReflineSettings().rowThresholdPx;
        data["rowSpacingPx"]        = settings->getReflineSettings().rowSpacingPx;
        data["rowPerspectivePx"]    = settings->getReflineSettings().rowPerspectivePx;
        
        res.set_content(data.dump(), "application/json");
        res.status = 200;
    });
}

void URL_stats(httplib::Server* svr, Stats* stats)
{
    svr->Get("/stats", [=](const Request &req, Response &res) {

        int fps = stats->fps.exchange(0);

        nlohmann::json data = 
        {
            { 
                "camera" , 
                {
                    { "frames_read" , 10 }
                }
            }
           ,{
                "detect" ,
                {
                    { "overall_ns" , 117 }
                }
            }
        };
        res.set_content(data.dump(), "application/json");
        res.status = 200;
    });
}
int thread_webserver(int port, Shared* shared, ImagePipeline* pipeline, Stats* stats)
{
    
    Server svr;

    shared->webSvr = &svr;

    const char* webroot = "./static";

    if ( !svr.set_mount_point("/", webroot)) {
        printf("E: web root dir does not exist\n");
        return 1;
    }

    URL_video(&svr, shared, pipeline, stats);
    URL_applyChanges(&svr, &shared->detectSettings );
    URL_current(&svr, &shared->detectSettings);
    URL_stats(&svr, stats);
    
    //
    // ------------------------------------------------------------------------
    //
    
    //
    // ------------------------------------------------------------------------
    //
    svr.Get("/list",       [&](const Request &req, Response &res) {
        errno = 0;
        DIR* dp = opendir("./detect");
        if (dp != NULL) {
            auto arr = nlohmann::json::array();
            
            dirent* dir;
            while((dir = readdir(dp)) != NULL)  
            {
                if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
                    continue;
                }
                arr.push_back(dir->d_name);
            }
            closedir(dp);

            nlohmann::json list;
            list["entries"] = arr;
            res.set_content(list.dump(), "application/json");
            res.status = 200;
        }

        if ( errno != 0 ) {
            res.status = 400;
            res.set_content(strerror(errno), "text/plain");
        }
    });
    //
    // ------------------------------------------------------------------------
    //
    svr.Get("/load/(.+)",  [&](const Request &req, Response &res) {
        std::string filename_to_load("./detect/");
        filename_to_load.append(req.matches[1].str());

        std::string content;
        if ( !trk::load_file_to_string(filename_to_load, &content) ) {
            res.status = 400;
            res.set_content(strerror(errno), "text/plain");
        }
        else {
            res.status = 200;
            res.set_content(content, "application/json");
        }
    });
    //
    // ------------------------------------------------------------------------
    //
    svr.Post("/save/(.+)", [&](const Request &req, Response &res) {
        std::string filename_to_save("./detect/");
        filename_to_save.append(req.matches[1].str());

        if ( ! trk::write_to_file(filename_to_save, req.body) ) {
            res.status = 400;
            res.set_content(strerror(errno), "text/plain");
        }
        else {
            res.status = 200;
        }
    });

    const char* host = "0.0.0.0";
    printf("I: webserver start listening on %s:%d\n", host, port);
    //svr.listen("0.0.0.0", port);
    svr.listen(host, port);

    return 0;
}

