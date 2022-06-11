cmake_minimum_required(VERSION 3.22)
project(Linux)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(Linux
        BitmapRawConverter.cpp
        BitmapRawConverter.h
        EasyBMP.cpp
        EasyBMP.h
        EasyBMP_BMP.h
        EasyBMP_DataStructures.h
        EasyBMP_VariousBMPutilities.h
        main.cpp)
list(APPEND CMAKE_MODULE_PATH "deps/tbb/cmake/")

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

find_package(TBB REQUIRED)
