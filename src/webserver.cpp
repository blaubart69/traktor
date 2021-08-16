#include <stdio.h>
#include <vector>
#include <functional>

#include "httplib.h"

#include "shared.h"

void thread_send_jpeg(Shared* shared, std::function<bool(std::vector<uchar>&)> sendJPEGbytes);

int thread_webserver(int port, Shared* shared)
{
    using namespace httplib;
    Server svr;

    shared->webSvr = &svr;

    const char* webroot = "./static";

    // Mount /public to ./www directory
    if ( !svr.set_mount_point("/", webroot)) {
        printf("E: web root dir does not exist\n");
        return 1;
    }

    svr.Get("/video", [&](const Request &req, Response &res) {

        static const std::string boundary("--Ba4oTvQMY8ew04N8dcnM\r\nContent-Type: image/jpeg\r\n\r\n");
        static const std::string CRLF("\r\n");

        res.set_content_provider(
            "multipart/x-mixed-replace;boundary=Ba4oTvQMY8ew04N8dcnM", // Content type
            [&](size_t offset, DataSink &sink) {
                //
                // begin of JPEG streaming
                //
                thread_send_jpeg(
                    shared,
                    [&sink](std::vector<unsigned char>& jpegBytes) {
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

                        return true;
                    });

                return true; // return 'false' if you want to cancel the process.
            },
            [](bool success) {}
        );
    });

    printf("I: webserver start listening on port %d\n", port);
    svr.listen("0.0.0.0", port);

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