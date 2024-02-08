set(EXE_TEST traktor_test)
project(${EXE_TEST} LANGUAGES CXX)

set(SOURCES 
    traktor_test.cpp
    ../traktor/src/calculate.cpp
)

add_executable(${EXE_TEST} ${SOURCES})

target_include_directories(${EXE_TEST} PRIVATE "../deps/cpp-httplib")
target_include_directories(${EXE_TEST} PRIVATE "../traktor/src")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

option(BUILD_GMOCK OFF)
option(INSTALL_GTEST OFF)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

find_package(OpenCV REQUIRED)

# Link your application with OpenCV libraries
target_link_libraries     (${EXE_TEST} PRIVATE ${OpenCV_LIBS})
target_include_directories(${EXE_TEST} PRIVATE ${OpenCV_INCLUDE_DIRS})

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)


target_link_libraries(${EXE_TEST} PUBLIC GTest::GTest)

enable_testing()
include(GoogleTest)
gtest_discover_tests(traktor_test)