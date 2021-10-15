#include <vector>
#include <utility>
#include <iostream>
#include <math.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

int main()
{
    Mat src, dst, grey_dst, blurred_dst, thres_dst, closed_dst;
    
    src = imread("C:/sci_practice_21-22/build/sample1.jpg");

    /*
    cvtColor(src, grey_dst, COLOR_GRAY2BGR);

    GaussianBlur(grey_dst, blurred_dst, Size(3, 3), 0);


    threshold(blurred_dst, thres_dst, 0, 1, THRESH_BINARY + THRESH_OTSU);

    Mat kernel {getStructuringElement(MORPH_RECT, Size(9, 9))};

    morphologyEx(thres_dst, closed_dst, MORPH_CLOSE, kernel);

    
    Size s{grey_dst.size()};
    long long horizon_x1{0};
    long long horizon_y1;

    double max{0};
    long long cols{closed_dst.cols};
    long long rows{closed_dst.rows};
    if (closed_dst.isContinuous())
    {
        cols *= rows;
        rows = 1;
    }
    for(long long i{0}; i < rows; i++)
    {
        const double* Mi = closed_dst.ptr<double>(i);
        for(int j = 0; j < cols; j++)
            if (std::max(Mi[j], 0.) > max)
            {
                max = j;
            }
    }
    horizon_y1 = max;



    long long horizon_x2{s.width};
    long long horizon_y2{0};
    
    double max2{0};
    long long cols2{closed_dst.cols};
    long long rows2{closed_dst.rows};
    if (closed_dst.isContinuous())
    {
        cols2 *= rows2;
        rows2 = 1;
    }
    for(long long i{0}; i < rows2; i++)
    {
        const double* Mi = closed_dst.ptr<double>(i);
        for(int j = 0; j < cols2; j++)
            if (std::max(Mi[j], 0.) > max)
            {
                max2 = j;
            }
    }
    horizon_y2 = max2;

    line(grey_dst, Point(horizon_x1, horizon_y1), Point(horizon_x2, horizon_y2), Scalar(0,0, 0, 255));

    */

    imshow("grey_dst", src);

}