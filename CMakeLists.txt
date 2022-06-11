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
target_link_libraries(Linux
        TBB::tbb
        )
list(APPEND CMAKE_MODULE_PATH "deps/tbb/cmake/")
find_package(TBB REQUIRED)
