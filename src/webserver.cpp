//#include <stdio.h>
//#include <vector>
//#include <functional>
//
//#include "httplib.h"

#include "shared.h"
#include "util.h"

void thread_send_jpeg(Shared* shared, std::function<bool(std::vector<uchar>&)> sendJPEGbytes);

int thread_webserver(int port, Shared* shared)
{
    using namespace httplib;
    Server svr;

    shared->webSvr = &svr;

    const char* webroot = "./static";

    if ( !svr.set_mount_point("/", webroot)) {
        printf("E: web root dir does not exist\n");
        return 1;
    }

    svr.Get("/video", [&](const Request &req, Response &res) {

        static const std::string boundary("--Ba4oTvQMY8ew04N8dcnM\r\nContent-Type: image/jpeg\r\n\r\n");
        static const std::string CRLF("\r\n");

        Stats &stats = shared->stats;

        res.set_content_provider(
            "multipart/x-mixed-replace;boundary=Ba4oTvQMY8ew04N8dcnM", // Content type
            [&](size_t offset, DataSink &sink) {
                //
                // begin of JPEG streaming
                //
                thread_send_jpeg(
                    shared,
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

                        stats.jpeg_bytes_sent += boundary.length() + jpegBytes.size();

                        return true;
                    });

                return true; // return 'false' if you want to cancel the process.
            },
            [](bool success) {}
        );
    });
    /*
        /applyChanges
    {
     "duration":1
    ,"left":12
    ,"right":13
    ,"direction":"center"
    ,"url":"http://10.3.141.165:8888/video"
    ,"colorFilter":true
    ,"detecting":true
    ,"colorFrom":"36,80,25"
    ,"colorTo":"80,255,255"
    ,"erode":10
    ,"dilate":10
    ,"contourMode":"POLY"
    ,"threshold":5
    ,"maximumMarkers":10
    ,"rowCount": 1
    ,"rowSpacePx": 160
    ,"rowPerspectivePx": 0
    }
    */
    svr.Post("/applyChanges", [&](const Request &req, Response &res)
    {
        try
        {
            nlohmann::json data = nlohmann::json::parse(req.body);
            //printf("applyChanges data: %s\n", data.dump(2).c_str() );
            
            DetectSettings& settings = shared->detectSettings;
            
            //settings.set_colorFrom( data["colorFrom"] );
            //settings.set_colorTo  ( data["colorTo"]   );
            settings.set_color_from( data["colorFrom"][0].get<int>(),
                                     data["colorFrom"][1].get<int>(), 
                                     data["colorFrom"][2].get<int>() );

            settings.set_color_to(   data["colorTo"][0].get<int>(),
                                     data["colorTo"][1].get<int>(), 
                                     data["colorTo"][2].get<int>() );

            settings.set_erode_dilate( data["erode"].get<int>(), data["dilate"].get<int>() );

            settings.set_maxRows           ( data["maxRows"]            .get<int>() );
            settings.set_rowSpacingPx      ( data["rowSpacingPx"]       .get<int>() );
            settings.set_rowPerspectivePx  ( data["rowPerspectivePx"]   .get<int>() );
            settings.set_rowThresholdPx    ( data["rowThresholdPx"]     .get<int>() );
            settings.set_minimalContourArea( data["minimalContourArea"] .get<int>() );

            std::string detecting = data["detecting"];
            if ( detecting.compare("start") == 0) 
            {
                settings.detecting.store ( true );
                printf("I: detecting is now: ON\n");
            }
            else if ( detecting.compare("stop") == 0) 
            {
                settings.detecting.store ( false );
                printf("I: detecting is now: OFF\n");
            }
            printf("I: half_row_count: %d\n", settings.getReflineSettings().get_half_row_count() );

            
        }
        catch(const std::exception& e)
        {
            fprintf(stderr, "/applyChanges: %s\n", e.what());
        }
    });
    svr.Post("/detect/list",      [&](const Request &req, Response &res) {
    });
    svr.Post("/detect/load/(.+)", [&](const Request &req, Response &res) {
    });
    svr.Post("/detect/save/(.+)", [&](const Request &req, Response &res) {
        try
        {
            std::string filename_to_save("./detect/");
            filename_to_save.append(req.matches[1].str());

            std::string msg("n/a");
            std::ofstream fp(filename_to_save, std::ios::out);
            if (!fp.is_open())
            {
                msg = "unable to open file";
                res.status = 400;
            }
            else {
                fp << req.body;
                if ( fp.fail() ) {
                    msg = "error writing to file";
                    res.status = 400;
                }
                else {
                    msg = "ok";
                    res.status = 200;
                }
            }
            
            if ( res.status == 200 ) { std::cout << "I: save. file " << filename_to_save << std::endl;
            } else                   { std::cerr << "E: save. file " << filename_to_save << std::endl;
            }

            res.set_content(msg, "text/plain");
        }
        catch(const std::exception& e)
        {
            fprintf(stderr, "/save: %s\n", e.what());
        }
    });

    const char* host = "0.0.0.0";
    printf("I: webserver start listening on %s:%d\n", host, port);
    //svr.listen("0.0.0.0", port);
    svr.listen(host, port);

    return 0;
}

/*

void TraktorHandler::onRequest(const Pistache::Http::Request& req, Pistache::Http::ResponseWriter response)
{
    if ( req.resource() == "/video" ) {
        response
            .headers()
            .add<Pistache::Http::Header::Server>("pistache/0.1")
            .add<Pistache::Http::Header::ContentType>("multipart/x-mixed-replace;boundary=Ba4oTvQMY8ew04N8dcnM");

        static const std::string boundary("--Ba4oTvQMY8ew04N8dcnM\r\nContent-Type: image/jpeg\r\n\r\n");
        static const std::string CRLF("\r\n");

        Pistache::Http::ResponseStream stream = response.stream(Pistache::Http::Code::Ok);

        thread_send_jpeg(
                this->_shared,
                [&stream](std::vector<unsigned char>& jpegBytes) {
                // yield(b'--Ba4oTvQMY8ew04N8dcnM\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(encodedImage) + b'\r\n')
                stream.write( boundary.data(), boundary.length() );
                jpegBytes.insert(jpegBytes.end(), CRLF.begin(), CRLF.end());
                stream.write( (char*)jpegBytes.data(), jpegBytes.size() );
                stream.flush();
        });
    }
    else if ( req.resource() == "/data" ) {
    }
    else if ( req.resource() == "/applyChanges" ) {
        response.send(Pistache::Http::Code::Ok);
    }
    else {
        if (req.method() == Pistache::Http::Method::Get) {
            std::string toServe("static");

            if ( req.resource() == "/" ) {
                toServe.append("/index.html");
            }
            else {
                toServe.append(req.resource());
            }

            struct stat buffer;
            if ( stat(toServe.c_str(), &buffer) == 0) {
                Pistache::Http::serveFile(response, toServe);
                printf("I: static-web: [%s]\n", toServe.c_str());
            }
            else {
                response.send(Pistache::Http::Code::Not_Found);
            }
        }
    }
}*/
