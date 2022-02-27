#include <iostream>
#include <math.h>
#include "Canny_Hough.h"
 
using namespace cv;


std::vector<std::pair<double, double> > get_coordinates_Canny_Hough(Mat src, int thres)
{

    double SCREEN_SIZE_X{src.cols};
    double SCREEN_SIZE_Y{src.rows};

    Mat dst, color_dst;
     


    cvtColor( src, src, COLOR_BGR2GRAY );
    Canny( src, dst, 50, 200, 3 );

    // массив линий для HoughLines
    std::vector<Vec2f> lines;
    HoughLines( dst, lines, 1, CV_PI/180, thres);
    unsigned long long lines_num{lines.size()};
    // нужно не забыть подумать, как регулировать последний параметр в HoughLines так, чтобы line_pts получился не слишком большим
    std::cout << "Canny_Hough, lines_num{lines.size()} == " << lines_num << "\n";

    
    // массив пар точек, определяющих каждую из линий
    std::vector<std::pair<Point2f, Point2f> >line_pts; 
    
    
    // строим точки, определяющие линии
    for( long long i{0}; i < lines_num; i++ )
    {
        float rho{lines[i][0]};
        float theta{lines[i][1]};  

        double a{cos(theta)}, b{sin(theta)};
        double x0{a*rho}, y0{b*rho};

        Point2f pt1(cvRound(x0 + 1000*(-b)), cvRound(y0 + 1000*(a)));
        Point2f pt2(cvRound(x0 - 1000*(-b)), cvRound(y0 - 1000*(a)));

        line_pts.push_back(std::make_pair(pt1, pt2));
    }

    // массив возможных vanishing points
    std::vector<Point2f> possible_vp;

    // строим возможные vanishing points
    if ((lines_num > 3)) 
        for (long long i {0}; i < lines_num; i++)
            for (long long j{i+1}; j < lines_num; j++)
            {
                double x1, y1, x2, y2;
                x1 = line_pts[i].first.x; y1 = line_pts[i].first.y;
                x2 = line_pts[i].second.x; y2 = line_pts[i].second.y;

                double x3, y3, x4, y4;
                x3 = line_pts[j].first.x; y3 = line_pts[j].first.y;
                x4 = line_pts[j].second.x; y4 = line_pts[j].second.y;

                if (((y2 - y1)/(x2 - x1) - (y4 - y3)/(x4 - x3)) && (x2 - x1)) 
                {
                    Point2f cross; 
                    
                    cross.x = ((y3 - y1) - (y4 - y3)*x3/(x4 - x3) + x1*(y2 - y1)/(x2 - x1)) /
                            ((y2 - y1)/(x2 - x1) - (y4 - y3)/(x4 - x3));

                    cross.y = (cross.x - x1)/(x2 - x1) * (y2 - y1) + y1;

                    if ( (abs(cross.x) < abs(4*SCREEN_SIZE_X)) && (abs(cross.y) < abs(4*SCREEN_SIZE_Y)))
                        possible_vp.push_back(cross);
                }
            }
    // используем метод наименьших квадратов чтобы найти горизонт 
    unsigned long long count{possible_vp.size()};
    std::cout << "Canny_Hough, count{possible_vp.size()} == " << count << "\n";
    double sumX{0}, sumY{0}, sumXY{0}, sumXX{0};
    for(int i{0}; i < count; i++) 
    {
        sumX += possible_vp[i].x;
        sumY += possible_vp[i].y;
        sumXY += possible_vp[i].x * possible_vp[i].y;
        sumXX += possible_vp[i].x * possible_vp[i].x;
    }
    double xMean {sumX / count};
    double yMean {sumY / count};
    double denom{sumXX - sumX * xMean};
     
    double slope {(sumXY - sumX * yMean) / denom};
    double yInt {yMean - slope * xMean};



    // получаем координаты
    double horizon_x1{0};
    double horizon_y1{yInt};

    double horizon_x2{SCREEN_SIZE_X};
    double horizon_y2{SCREEN_SIZE_X*slope + yInt};
    // line( color_dst, Point(x1, y1), Point(x2, y2), Scalar(0,0,255), 3, 8 ); 
 

    // namedWindow( "Detected Lines", 0.5 );
    // imshow( "Detected Lines", color_dst );

    // waitKey(0);


    std::vector<std::pair<double, double> > result = {
        std::make_pair(horizon_x1, horizon_y1),
        std::make_pair(horizon_x2, horizon_y2)
    };

    return std::move(result);
}
