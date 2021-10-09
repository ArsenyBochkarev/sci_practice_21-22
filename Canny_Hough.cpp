#include <vector>
#include <utility>
#include <iostream>
#include <math.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

// в будущем буду поправлять
long long SCREEN_SIZE_X{1920};
long long SCREEN_SIZE_Y{1080};

int main()
{
    Mat src, dst, color_dst;
    
    src = imread("C:/opencv_practice/build/sample4.jpg");


    Canny( src, dst, 50, 200, 3 );
    cvtColor( dst, color_dst, COLOR_GRAY2BGR );

    #if 1
    vector<Vec2f> lines;

    // массив пар точек, определяющих каждую из линий
    vector<std::pair<Point2f, Point2f> >line_pts; 
    // массив возможных vanishing points
    vector<Point2f> possible_vp;
    
    HoughLines( dst, lines, 1, CV_PI/180, 180 );

    long long lines_num = lines.size();

    // не забыть отрегулировать последний параметр в HoughLines так, \
    // чтобы line_pts получился не слишком большим
    for( long long i = 0; i < lines_num; i++ )
    {
        float rho = lines[i][0];
        float theta = lines[i][1];  

        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;

        Point2f pt1(cvRound(x0 + 1000*(-b)),
                  cvRound(y0 + 1000*(a)));
        Point2f pt2(cvRound(x0 - 1000*(-b)),
                  cvRound(y0 - 1000*(a)));

        line_pts.push_back(std::make_pair(pt1, pt2));

        //line( color_dst, pt1, pt2, Scalar(0,0,255), 3, 8 );
    }
    
    // на данный момент не придумал ничего лучше, чем просто найти персечения всех со всеми, отсекая только совсем уж нереалистичные варианты
    if ((lines_num > 3))
        for (long long i {0}; i < lines_num; i++)
            for (long long j{1}; j < lines_num; j++)
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

                    if ( (abs(cross.x) < abs(5*SCREEN_SIZE_X)) && (abs(cross.y) < abs(5*SCREEN_SIZE_Y)))
                        possible_vp.push_back(cross);
                }
            }
    
    // используем метод наименьших квадратов чтобы найти горизонт 
    unsigned long long count{possible_vp.size()};
    double sumX{0}, sumY{0}, sumXY{0}, sumX2{0};
    
    for(int i{0}; i < count; i++) 
    {
        sumX += possible_vp[i].x;
        sumY += possible_vp[i].y;
        sumXY += possible_vp[i].x * possible_vp[i].y;
        sumX2 += possible_vp[i].x * possible_vp[i].x;
    }
    
    double xMean{sumX / count};
    double yMean{sumY / count};
    double denominator{sumX2 - sumX * xMean};
    
    double slope{(sumXY - sumX * yMean) / denominator};
    double yInt{yMean - slope * xMean};

    line( color_dst, Point(0, yInt), Point(SCREEN_SIZE_X, SCREEN_SIZE_X*slope + yInt), Scalar(0,0,255), 3, 8 );

    
    /*
    std::cout << "\n\n\n\n\n\n\n\n\n\n";

    std::cout << "slope = " << slope << " yint = " << _yInt;
    
    //std::cout << "\n\n\n\n\n\n\n\n\n\n";
    //for (int i{0}; i < possible_vp.size(); i++)
        //std::cout << possible_vp[i].x << " " << possible_vp[i].y << "\n";
    std::cout << "\n\n\n\n\n\n\n\n\n\n";
    */

    #else
    // пока что ненужный пример с probabilistic Hough transform
    
    vector<Vec4i> lines;
    HoughLinesP( dst, lines, 1, CV_PI/180, 80, 30, 10 );
    for( size_t i = 0; i < lines.size(); i++ )
    {
        line( color_dst, Point(lines[i][0], lines[i][1]),
        Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
    }
    
    #endif
    //namedWindow( "Source", 1 );
    //imshow( "Source", src );

    namedWindow( "Detected Lines", 0.5 );
    imshow( "Detected Lines", color_dst );

    waitKey(0);
}
