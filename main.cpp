#include <iostream>
#include <stdlib.h>
#include "BitmapRawConverter.h"
#include "tbb/task_group.h"

#define __ARG_NUM__                6
#define FILTER_SIZE                3
#define THRESHOLD                128

using namespace std;

// Prewitt operators
int filterHor[FILTER_SIZE * FILTER_SIZE] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
int filterVer[FILTER_SIZE * FILTER_SIZE] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};

/**
* @brief Serial version of edge detection algorithm implementation using Prewitt operator
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param width image width
* @param height image height
**/

void calculate_filter(int *inBuffer, int *outBuffer, int start_width, int start_height, int stop_width, int stop_height,
                      int full_width) {
    int filter_x[3][3] = {{-1, 0, 1},
                          {-1, 0, 1},
                          {-1, 0, 1}};

    int filter_y[3][3] = {{-1, -1, -1},
                          {0,  0,  0},
                          {1,  1,  1}};

    for (int i = start_width; i < stop_width; ++i) {
        for (int j = start_height; j < stop_height; ++j) {
            double color_x = 0;
            double color_y = 0;
            for (int k = 0; k < 3; ++k) {
                for (int l = 0; l < 3; ++l) {
                    int xn = i + k - 1;
                    int yn = j + l - 1;

                    int index = xn + yn * full_width;
                    color_x += inBuffer[index] * filter_x[k][l];
                    color_y += inBuffer[index] * filter_y[k][l];


                }

            }
            if (abs(color_y) + abs(color_x) >= 128)
                outBuffer[i + j * full_width] = 255;
            else
                outBuffer[i + j * full_width] = 0;

        }
    }
}

void filter_serial_prewitt(int *inBuffer, int *outBuffer, int width, int height)  //TODO obrisati
{
//    int filter_x[3][3] = {{-1, 0, 1},
//                          {-1, 0, 1},
//                          {-1, 0, 1}};
//
//    int filter_y[3][3] = {{-1, -1, -1},
//                          {0,  0,  0},
//                          {1,  1,  1}};
//
//    for (int i = 1; i < width - 1; ++i) {
//        for (int j = 1; j < height - 1; ++j) {
//            double color_x = 0;
//            double color_y = 0;
//            for (int k = 0; k < 3; ++k) {
//                for (int l = 0; l < 3; ++l) {
//                    int xn = i + k - 1;
//                    int yn = j + l - 1;
//
//                    int index = xn + yn * width;
//                    color_x += inBuffer[index] * filter_x[k][l];
//                    color_y += inBuffer[index] * filter_y[k][l];
//
//
//                }
//
//            }
//            if (abs(color_y) + abs(color_x) >= 128)
//                outBuffer[i + j * width] = 255;
//            else
//                outBuffer[i + j * width] = 0;
//
//
//        }
//    }
    calculate_filter(inBuffer,outBuffer,1,1,width-1,height-1,width);
}



/**
* @brief Parallel version of edge detection algorithm implementation using Prewitt operator
* 
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param width image width
* @param height image height
*/
void filter_parallel_prewitt(int *inBuffer, int *outBuffer, int start_width, int start_height, int stop_width,
                             int stop_height, int full_width) {
    tbb::task_group g;
    int middle_width = start_width + (stop_width - start_width) / 2;
    int middle_height = start_height + (stop_height - start_height) / 2;
    if ((stop_height - start_height) > THRESHOLD) {
        // 1
        g.run([=]() {
            filter_parallel_prewitt(inBuffer, outBuffer, start_width, start_height, middle_width, middle_height,
                                    full_width);
        });
        // 2
        g.run([=]() {
            filter_parallel_prewitt(inBuffer, outBuffer, middle_width, start_height, stop_width, middle_height,
                                    full_width);
        });
        // 3
        g.run([=]() {
            filter_parallel_prewitt(inBuffer, outBuffer, start_width, middle_height,
                                    middle_width, stop_height, full_width);
        });
        // 4
        g.run([=]() {
            filter_parallel_prewitt(inBuffer, outBuffer, middle_width, middle_height, stop_width, stop_height,
                                    full_width);
        });
        g.wait();
    } else {
        calculate_filter(inBuffer, outBuffer, start_width, start_height, stop_width, stop_height, full_width);
    }
}

/**
* @brief Serial version of edge detection algorithm
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param width image width
* @param height image height
*/
void filter_serial_edge_detection(int *inBuffer, int *outBuffer, int width, int height)    //TODO obrisati
{
}

/**
* @brief Parallel version of edge detection algorithm
* 
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param width image width
* @param height image height
*/
void filter_parallel_edge_detection(int *inBuffer, int *outBuffer, int width, int height) {
}

/**
* @brief Function for running test.
*
* @param testNr test identification, 1: for serial version, 2: for parallel version
* @param ioFile input/output file, firstly it's holding buffer from input image and than to hold filtered data
* @param outFileName output file name
* @param outBuffer buffer of output image
* @param width image width
* @param height image height
*/


void run_test_nr(int testNr, BitmapRawConverter *ioFile, char *outFileName, int *outBuffer, unsigned int width,
                 unsigned int height) {

    // TODO: start measure


    switch (testNr) {
        case 1:
            cout << "Running serial version of edge detection using Prewitt operator" << endl;
            filter_serial_prewitt(ioFile->getBuffer(), outBuffer, width, height);
            break;
        case 2:
            cout << "Running parallel version of edge detection using Prewitt operator" << endl;
            filter_parallel_prewitt(ioFile->getBuffer(), outBuffer, 1, 1, width-1, height-1, width);
            break;
        case 3:
            cout << "Running serial version of edge detection" << endl;
            filter_serial_edge_detection(ioFile->getBuffer(), outBuffer, width, height);
            break;
        case 4:
            cout << "Running parallel version of edge detection" << endl;
            filter_parallel_edge_detection(ioFile->getBuffer(), outBuffer, width, height);
            break;
        default:
            cout << "ERROR: invalid test case, must be 1, 2, 3 or 4!";
            break;
    }
    // TODO: end measure and display time
    ioFile->setBuffer(outBuffer);
    ioFile->pixelsToBitmap(outFileName);
}

int main(int argc, char *argv[]) {

    BitmapRawConverter inputFile("../color.bmp");
    BitmapRawConverter outputFileSerialPrewitt("../color1.bmp");
    BitmapRawConverter outputFileParallelPrewitt("../color2.bmp");
    BitmapRawConverter outputFileSerialEdge("../color.bmp");
    BitmapRawConverter outputFileParallelEdge("../color.bmp");

    unsigned int width, height;

    int test;

    width = inputFile.getWidth();
    height = inputFile.getHeight();

    int *outBufferSerialPrewitt = new int[width * height];
    int *outBufferParallelPrewitt = new int[width * height];

    memset(outBufferSerialPrewitt, 0x0, width * height * sizeof(int));
    memset(outBufferParallelPrewitt, 0x0, width * height * sizeof(int));

    int *outBufferSerialEdge = new int[width * height];
    int *outBufferParallelEdge = new int[width * height];

    memset(outBufferSerialEdge, 0x0, width * height * sizeof(int));
    memset(outBufferParallelEdge, 0x0, width * height * sizeof(int));

    // serial version Prewitt
    run_test_nr(1, &outputFileSerialPrewitt, "../color1.bmp", outBufferSerialPrewitt, width, height);

    // parallel version Prewitt
    run_test_nr(2, &outputFileParallelPrewitt, "../color2.bmp", outBufferParallelPrewitt, width, height);

//    // serial version special
//    run_test_nr(3, &outputFileSerialEdge, argv[4], outBufferSerialEdge, width, height);
//
//    // parallel version special
//    run_test_nr(4, &outputFileParallelEdge, argv[5], outBufferParallelEdge, width, height);

    // verification
    cout << "Verification: ";
    test = memcmp(outBufferSerialPrewitt, outBufferParallelPrewitt, width * height * sizeof(int));

    if (test != 0) {
        cout << "Prewitt FAIL!" << endl;
    } else {
        cout << "Prewitt PASS." << endl;
    }

    test = memcmp(outBufferSerialEdge, outBufferParallelEdge, width * height * sizeof(int));

    if (test != 0) {
        cout << "Edge detection FAIL!" << endl;
    } else {
        cout << "Edge detection PASS." << endl;
    }

    // clean up
    delete outBufferSerialPrewitt;
    delete outBufferParallelPrewitt;

    delete outBufferSerialEdge;
    delete outBufferParallelEdge;

    return 0;
}