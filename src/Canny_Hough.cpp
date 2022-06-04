#include <iostream>
#include <math.h>
#include "Canny_Hough.h"
 
using namespace cv;

#define _USE_MATH_DEFINES

float REJECT_DEGREE_TH = 4.0;

std::vector<std::vector<double>> FilterLines(std::vector<cv::Vec4i> Lines)
{
    std::vector<std::vector<double>> FinalLines;

    for (int i = 0; i < Lines.size(); i++)
    {
        cv::Vec4i Line = Lines[i];
        int x1 = Line[0], y1 = Line[1];
        int x2 = Line[2], y2 = Line[3];
        
        double m, c;

        // Calculating equation of the line : y = mx + c
        if (x1 != x2)
            m = (double)(y2 - y1) / (double)(x2 - x1);
        else
            m = 100000000.0;
        c = y2 - m * x2;
        
        // theta will contain values between - 90 -> + 90.
        double theta = atan(m) * (180.0 / M_PI);

        /*# Rejecting lines of slope near to 0 degree or 90 degree and storing others
        if REJECT_DEGREE_TH <= abs(theta) <= (90 - REJECT_DEGREE_TH):
            l = math.sqrt( (y2 - y1)**2 + (x2 - x1)**2 )    # length of the line
            FinalLines.append([x1, y1, x2, y2, m, c, l])*/
        // Rejecting lines of slope near to 0 degree or 90 degree and storing others
        if (REJECT_DEGREE_TH <= abs(theta) && abs(theta) <= (90.0 - REJECT_DEGREE_TH))
        {
            double l = pow((pow((y2 - y1), 2) + pow((x2 - x1), 2)), 0.5);   // length of the line
            std::vector<double> FinalLine{ (double)x1, (double)y1, (double)x2, (double)y2, m, c, l };
            FinalLines.push_back(FinalLine);
        }
    }

    // Removing extra lines
    // (we might get many lines, so we are going to take only longest 15 lines 
    // for further computation because more than this number of lines will only
    // contribute towards slowing down of our algo.)
    if (FinalLines.size() > 15)
    {
        std::sort(FinalLines.begin(), FinalLines.end(), 
                  [](const std::vector< double >& a, 
                     const std::vector< double >& b) 
                    { return a[6] > b[6]; });
        
        std::vector<std::vector<double>> FinalLines2;
        FinalLines = std::vector<std::vector<double>>(FinalLines.begin(), FinalLines.begin() + 15);
    }

    return FinalLines;
}


std::vector<std::vector<double>> GetLines(cv::Mat Image)
{
    cv::Mat GrayImage, BlurGrayImage, EdgeImage;
    // Converting to grayscale
    cv::cvtColor(Image, GrayImage, cv::COLOR_BGR2GRAY);
    // Blurring image to reduce noise.
    cv::GaussianBlur(GrayImage, BlurGrayImage, cv::Size(5, 5), 1);
    // Generating Edge image
    cv::Canny(BlurGrayImage, EdgeImage, 40, 255);

    // Finding Lines in the image
    std::vector<cv::Vec4i> Lines;
    cv::HoughLinesP(EdgeImage, Lines, 1, CV_PI / 180, 50, 15);
    
    // Check if lines found and exit if not.
    if (Lines.size() == 0)
    {
        std::cout << "Not enough lines found in the image for Vanishing Point detection." << std::endl;
        exit(3);
    }

    //Filtering Lines wrt angle
    std::vector<std::vector<double>> FilteredLines;
    FilteredLines = FilterLines(Lines);

    return FilteredLines ;
}


double* GetVanishingPoint(std::vector<std::vector<double>> Lines, bool first_time, double* VanishingPoint_prev)
{
    // We will apply RANSAC inspired algorithm for this. We will take combination
    // of 2 lines one by one, find their intersection point, and calculate the
    // total error(loss) of that point.Error of the point means root of sum of
    // squares of distance of that point from each line.
    double* VanishingPoint = new double[2];
    VanishingPoint[0] = -1; VanishingPoint[1] = -1;

    double MinError = 1000000000.0;

    for (int i = 0; i < Lines.size(); i++)
    {
        for (int j = i + 1; j < Lines.size(); j++)
        {
            double m1 = Lines[i][4], c1 = Lines[i][5];
            double m2 = Lines[j][4], c2 = Lines[j][5];

            if (m1 != m2)
            {
                double x0 = (c1 - c2) / (m2 - m1);
                double y0 = m1 * x0 + c1;

                double err = 0;
                for (int k = 0; k < Lines.size(); k++)
                {
                    double m = Lines[k][4], c = Lines[k][5];
                    double m_ = (-1 / m);
                    double c_ = y0 - m_ * x0;

                    double x_ = (c - c_) / (m_ - m);
                    double y_ = m_ * x_ + c_;

                    double l = pow((pow((y_ - y0), 2) + pow((x_ - x0), 2)), 0.5);

                    err += pow(l, 2);
                }

                err = pow(err, 0.5);


                if (first_time)
                {
                    if (MinError > err)
                    { 
                        MinError = err;
                        VanishingPoint[0] = x0;
                        VanishingPoint[1] = y0;
                    }
                }
                else
                { 
                    if (MinError > err)
                    {
                        
                        if (!((VanishingPoint_prev[0] == x0) && (VanishingPoint_prev[1] == y0)))
                        {
                            MinError = err;
                            VanishingPoint[0] = x0;
                            VanishingPoint[1] = y0;
                        }
                    }
                }
                    
            }
        }
    }



    return VanishingPoint;
}





/// Функция получения координат
std::vector<std::pair<double, double> > get_coordinates_Canny_Hough(Mat src)
{
    std::vector<std::pair<double, double> > result;

    double SCREEN_SIZE_X{static_cast<double>(src.cols)};
    double SCREEN_SIZE_Y{static_cast<double>(src.rows)};

    // Getting the lines form the image
    std::vector<std::vector<double>> Lines;
    Lines = GetLines(src);

    // Get vanishing point
    double* VanishingPoint1;
    VanishingPoint1 = GetVanishingPoint(Lines, 1, VanishingPoint1);
    double* VanishingPoint2 = GetVanishingPoint(Lines, 0, VanishingPoint1);


    // Checking if vanishing point found
    if ( (VanishingPoint1[0] == -1 && VanishingPoint1[1] == -1) || (VanishingPoint2[0] == -1 && VanishingPoint2[1] == -1))
    {
        std::cout << "Vanishing Point not found. Possible reason is that not enough lines are found in the image for determination of vanishing point." << "\n";

    }



    result.push_back(std::make_pair(0, VanishingPoint1[1]));
    result.push_back(std::make_pair(SCREEN_SIZE_X, VanishingPoint2[1]));
 

    return result;
}







