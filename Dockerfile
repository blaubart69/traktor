ARG ALPINE_VERSION=3.19

FROM node:22-bookworm as fe-build

WORKDIR /fe-app

COPY ./frontend .

RUN yarn global add @quasar/cli
RUN yarn --no-fund
RUN quasar build

# ARG OPENCV_VERSION=4.10.0
# 2024-12-31 Spindler
#   Alpine is using a fixed version of OpenCV for each of it's versions
#   so it doesn't make sense to specify a OpenCV version.
#   It's backed into the Alpine version

FROM alpine:${ALPINE_VERSION} AS build

#ARG OPENCV_VERSION

# opencv-dev in a seperate RUN step, because it pulls in >600 packages
RUN apk add --no-cache  \
    opencv-dev
    #opencv-dev~=${OPENCV_VERSION}

    # apk update         \
#  && apk upgrade        \
RUN apk add --no-cache  \
    wget                \
    clang               \
    alpine-sdk          \
    cmake               \
    libgpiod-dev    

RUN     mkdir -p    /traktor/deps/cpp-httplib   \
                    /traktor/deps/json          \
    &&  wget -O     /traktor/deps/cpp-httplib/httplib.h https://github.com/yhirose/cpp-httplib/raw/master/httplib.h                     \
    &&  wget -O     /traktor/deps/json/json.hpp         https://github.com/nlohmann/json/raw/develop/single_include/nlohmann/json.hpp

COPY ./traktor/src              /traktor/src
COPY ./traktor/CMakeLists.txt   /traktor/CMakeLists.txt

WORKDIR /traktor/build
RUN     cmake -DCMAKE_BUILD_TYPE=Release .. \
    &&  make -j`nproc`                      \
    &&  strip --strip-all ./traktor
#
# MAIN image
#
FROM alpine:${ALPINE_VERSION}

ARG OPENCV_VERSION

RUN apk add --no-cache  \
    libgpiod            \
    opencv

#
# 2024-01-16 Spindler
#   libopencv_aruco.so is needed (warum a imma) and not part of the "opencv" package from alpine
#   dirty hack: copy it from the build stage. (ned schee)
#
COPY --from=build   /usr/lib/libopencv_aruco.so.4.8.1 /usr/lib/libopencv_aruco.so.4.8.1
RUN ln -s           /usr/lib/libopencv_aruco.so.4.8.1 /usr/lib/libopencv_aruco.so.408

COPY --from=build       /traktor/build/traktor  /app/
COPY --from=fe-build    /fe-app/dist/spa        /app/static/

WORKDIR /app
EXPOSE 9080
ENTRYPOINT ["./traktor"]


