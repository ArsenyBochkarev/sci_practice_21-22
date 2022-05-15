#include <iostream>
#include <math.h>
#include "Gauss_threshold.h"

using namespace cv;

/// Функция получения координат
std::vector<std::pair<double, double> > get_coordinates_Gauss_threshold(Mat src)
{
    Mat dst, grey_dst, blurred_dst, thres_dst, closed_dst;
    

    cvtColor( src, grey_dst, COLOR_BGR2GRAY );


    GaussianBlur(grey_dst, blurred_dst, Size(3, 3), 0);

    threshold(blurred_dst, thres_dst, 0, 255, THRESH_BINARY + THRESH_OTSU);
    
    
    
    Mat kernel {getStructuringElement(MORPH_RECT, Size(9, 9))};

    morphologyEx(thres_dst, closed_dst, MORPH_CLOSE, kernel);

    
    Size s{closed_dst.size()};
    long long horizon_x1{0};
    long long horizon_y1{0};

    long long horizon_x2{s.width - 1};
    long long horizon_y2{0};
    
  
    uint8_t* pixel_map{static_cast<uint8_t*> (closed_dst.data)};
    long long channels{closed_dst.channels()};
    uint8_t current_pixel[3];

    for (long long i{0}; i < s.height - 1; i++)
    {
        bool tmp{1};
        for (long long j{0}; j < 1; j++)
        {
            current_pixel[0] = pixel_map[i*closed_dst.cols*channels + j*channels + 0];
            current_pixel[1] = pixel_map[i*closed_dst.cols*channels + j*channels + 1];
            current_pixel[2] = pixel_map[i*closed_dst.cols*channels + j*channels + 2];



            if (current_pixel[0] + current_pixel[1] + current_pixel[2])
                horizon_y1 = i;
            else 
                tmp = 0;
        }
        if (!tmp)
            break;
    }

    for (long long i{0}; i < s.height - 1; i++)
    { 
        bool tmp{1};
        for (long long j{s.width-1}; j < s.width; j++)
        {

            current_pixel[0] = pixel_map[i*closed_dst.cols*channels + j*channels + 0];
            current_pixel[1] = pixel_map[i*closed_dst.cols*channels + j*channels + 1];
            current_pixel[2] = pixel_map[i*closed_dst.cols*channels + j*channels + 2];
        
            if (current_pixel[0] + current_pixel[1] + current_pixel[2])
                horizon_y2 = i;
            else 
                tmp = 0;
        } 
        if (!tmp)
            break;
    }



    std::vector<std::pair<double, double> > result = {
        std::make_pair(horizon_x1, horizon_y1),
        std::make_pair(horizon_x2, horizon_y2)
    };

    return result;

}
