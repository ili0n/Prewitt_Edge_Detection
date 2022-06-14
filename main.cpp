#include <iostream>
#include <stdlib.h>
#include "BitmapRawConverter.h"
#include "tbb/task_group.h"
#include "tbb/tick_count.h"

#define __ARG_NUM__                6
#define FILTER_SIZE                3
#define THRESHOLD                128
#define cutoff                    32
#define edge_width                 1


using namespace std;

// Prewitt operators
int filterHor[FILTER_SIZE * FILTER_SIZE] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
int filterVer[FILTER_SIZE * FILTER_SIZE] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
int filter_size = 5;

/**
* @brief Serial version of edge detection algorithm implementation using Prewitt operator
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param start_width first width index to start
* @param start_height first height index to start
* @param stop_width last width index to start
* @param start_height last height index to start
* @param full_width image width
**/

void calculate_filter(int *inBuffer, int *outBuffer, int start_width, int start_height, int stop_width, int stop_height,
                      int full_width) {

//    int filter_x[3][3] = {{-1, 0, 1},
//                          {-1, 0, 1},
//                          {-1, 0, 1}};
//
//    int filter_y[3][3] = {{-1, -1, -1},
//                          {0,  0,  0},
//                          {1,  1,  1}};

//    if (filter_size == 5) {

    int filter_x[5][5] = {{-1, 0, 0, 0, 1},
                          {-1, 0, 0, 0, 1},
                          {-1, 0, 0, 0, 1},
                          {-1, 0, 0, 0, 1},
                          {-1, 0, 0, 0, 1}};

    int filter_y[5][5] = {{-1, -1, -1, -1, -1},
                          {0,  0,  0,  0,  0},
                          {0,  0,  0,  0,  0},
                          {0,  0,  0,  0,  0},
                          {1,  1,  1,  1,  1}};


//    }
    for (int i = start_width; i < stop_width; ++i) {
        for (int j = start_height; j < stop_height; ++j) {
            double color_x = 0;
            double color_y = 0;
            for (int k = 0; k < filter_size; ++k) {
                for (int l = 0; l < filter_size; ++l) {
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

void filter_serial_prewitt(int *inBuffer, int *outBuffer, int width, int height) {
    calculate_filter(inBuffer, outBuffer, 1, 1, width - filter_size / 2 - 1, height - filter_size / 2 - 1, width);
}


/**
* @brief Serial version of edge detection algorithm
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param start_width first width index to start
* @param start_height first height index to start
* @param stop_width last width index to start
* @param start_height last height index to start
* @param full_width image width
**/
void calculate_edge(int *inBuffer, int *outBuffer, int start_width, int start_height, int stop_width, int stop_height,
                    int full_width) {
    for (int i = start_width; i < stop_width; ++i) {
        for (int j = start_height; j < stop_height; ++j) {
            double o = 1;
            double p = 0;

            for (int k = -edge_width; k < edge_width + 1; ++k) {
                for (int l = -edge_width; l < edge_width + 1; ++l) {
                    if (k == 0 && l == 0) continue;
                    int xn = i + k;
                    int yn = j + l;
                    int index = xn + yn * full_width;
                    if (inBuffer[index] > 128) p = 1;
                    else
                        o = 0;
                }
            }


            if (abs(o - p) == 1)
                outBuffer[i + j * full_width] = 255;
            else
                outBuffer[i + j * full_width] = 0;

        }
    }

}


/**
* @brief Parallel version of edge detection algorithm implementation using Prewitt operator
* 
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param start_width first width index to start
* @param start_height first height index to start
* @param stop_width last width index to start
* @param start_height last height index to start
* @param full_width image width
*/
void filter_parallel_prewitt(int *inBuffer, int *outBuffer, int start_width, int start_height, int stop_width,
                             int stop_height, int full_width) {
    tbb::task_group g;
    int middle_width = start_width + (stop_width - start_width) / 2;
    int middle_height = start_height + (stop_height - start_height) / 2;
    if ((stop_height - start_height) > cutoff) {
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
void filter_serial_edge_detection(int *inBuffer, int *outBuffer, int width, int height) {
    calculate_edge(inBuffer, outBuffer, edge_width, edge_width, width - edge_width, height - edge_width, width);
}

/**
* @brief Parallel version of edge detection algorithm
* @param inBuffer buffer of input image
* @param outBuffer buffer of output image
* @param start_width first width index to start
* @param start_height first height index to start
* @param stop_width last width index to start
* @param start_height last height index to start
* @param full_width image width
*/
void filter_parallel_edge_detection(int *inBuffer, int *outBuffer, int start_width, int start_height, int stop_width,
                                    int stop_height, int full_width) {
    tbb::task_group g;
    int middle_width = start_width + (stop_width - start_width) / 2;
    int middle_height = start_height + (stop_height - start_height) / 2;
    if ((stop_height - start_height) > cutoff) {
        // 1
        g.run([=]() {
            filter_parallel_edge_detection(inBuffer, outBuffer, start_width, start_height, middle_width, middle_height,
                                           full_width);
        });
        // 2
        g.run([=]() {
            filter_parallel_edge_detection(inBuffer, outBuffer, middle_width, start_height, stop_width, middle_height,
                                           full_width);
        });
        // 3
        g.run([=]() {
            filter_parallel_edge_detection(inBuffer, outBuffer, start_width, middle_height,
                                           middle_width, stop_height, full_width);
        });
        // 4
        g.run([=]() {
            filter_parallel_edge_detection(inBuffer, outBuffer, middle_width, middle_height, stop_width, stop_height,
                                           full_width);
        });
        g.wait();
    } else {
        calculate_edge(inBuffer, outBuffer, start_width, start_height, stop_width, stop_height, full_width);
    }
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


    tbb::tick_count start_time;
    tbb::tick_count stop_time;

    switch (testNr) {
        case 1:
            cout << "Running serial version of edge detection using Prewitt operator" << endl;
            start_time = tbb::tick_count::now();
            filter_serial_prewitt(ioFile->getBuffer(), outBuffer, width, height);
            stop_time = tbb::tick_count::now();
            cout << "Time lasted: " << (stop_time - start_time).seconds() * 1000 << "ms\n";
            break;
        case 2:
            cout << "Running parallel version of edge detection using Prewitt operator" << endl;
            start_time = tbb::tick_count::now();
            filter_parallel_prewitt(ioFile->getBuffer(), outBuffer, 1, 1, width - filter_size/2-1, height - filter_size/2-1, width);
            stop_time = tbb::tick_count::now();
            cout << "Time lasted: " << (stop_time - start_time).seconds() * 1000 << "ms\n";
            break;
        case 3:
            cout << "Running serial version of edge detection" << endl;
            start_time = tbb::tick_count::now();
            filter_serial_edge_detection(ioFile->getBuffer(), outBuffer, width, height);
            stop_time = tbb::tick_count::now();
            cout << "Time lasted: " << (stop_time - start_time).seconds() * 1000 << "ms\n";
            break;
        case 4:
            cout << "Running parallel version of edge detection" << endl;
            start_time = tbb::tick_count::now();
            filter_parallel_edge_detection(ioFile->getBuffer(), outBuffer, edge_width, edge_width, width - edge_width,
                                           height - edge_width, width);
            stop_time = tbb::tick_count::now();
            cout << "Time lasted: " << (stop_time - start_time).seconds() * 1000 << "ms\n";
            break;
        default:
            cout << "ERROR: invalid test case, must be 1, 2, 3 or 4!";
            break;
    }

    ioFile->setBuffer(outBuffer);
    ioFile->pixelsToBitmap(outFileName);
}

int main(int argc, char *argv[]) {

    BitmapRawConverter inputFile("../color.bmp");
    BitmapRawConverter outputFileSerialPrewitt("../color.bmp");
    BitmapRawConverter outputFileParallelPrewitt("../color.bmp");
    BitmapRawConverter outputFileSerialEdge("../color.bmp");
    BitmapRawConverter outputFileParallelEdge("../color.bmp");


//    cout<<"Selecte Filter dimensions:\n0 3x3\n1 5x5\n";
//    cin>>input;
//    if (input[0] !='0' && input[0] !='1'){
//        cout<<"InvalidInput\n";
//        exit(-2);
//    }
//    if (input[0] =='0') filter_size =3;
//    else filter_size =5;

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

    // serial version special
    run_test_nr(3, &outputFileSerialEdge, "../color3.bmp", outBufferSerialEdge, width, height);

//    // parallel version special
    run_test_nr(4, &outputFileParallelEdge, "../color4.bmp", outBufferParallelEdge, width, height);

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