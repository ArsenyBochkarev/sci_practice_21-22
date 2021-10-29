#include <iostream> 
#include <vector> 
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video/tracking.hpp>

using namespace std;
using namespace cv;

double SCREEN_SIZE_X{1920};
double SCREEN_SIZE_Y{1080};


// функция, которая ищет "хорошие" feature points, т.е. такие, у которых status == 1, 
// и передаёт их массив good
void find_good_fp(Mat& status, Mat& prev, Mat& good)
{
    Size s{prev.size()};
    good = Mat::zeros(Size(s.width, s.height), CV_16U);
    for (long long i{0}; i < s.width; i++)
        for (long long j{0}; j < s.height; j++)
        {
            if (status.at<unsigned char>(i, j) == 1)
                good.at<unsigned char>(i, j) = prev.at<unsigned char>(i, j); 
        }
}

int main()
{
    Mat first_frame, first_gray_frame, prev_frame, next_frame, current_frame, gray_current_frame, 
        prev_pts, next_pts, status, err, good_old_fp, good_new_fp;
    
    VideoCapture cap("video_in.AVI");


    if(!cap.isOpened())
    {
        std::cout << "Error opening video stream or file \n\n\n";
        return -1;
    }


    cap.read(first_frame);
    cvtColor(first_frame, first_gray_frame, COLOR_BGR2GRAY);

    //goodFeaturesToTrack(first_gray_frame, prev_pts, 300, 0.2, 2);

    double horizon_x1{0}, horizon_y1{SCREEN_SIZE_Y/2};
    double horizon_x2{SCREEN_SIZE_X}, horizon_y2{SCREEN_SIZE_Y/2};
    // как-то определяем изначальный горизонт (нынешние значения по дефолту стоят на середние экрана)

    Mat horizon_pts;
    horizon_pts = Mat::zeros(Size(2, 2), CV_16U);
    horizon_pts.at<double>(0, 0) = horizon_x1;
    horizon_pts.at<double>(1, 0) = horizon_y1;
    horizon_pts.at<double>(0, 1) = horizon_x2;
    horizon_pts.at<double>(1, 1) = horizon_y2;


    while(cap.isOpened())
    {
        cap.read(current_frame);
        if (current_frame.empty())
            break;

        cvtColor(current_frame, gray_current_frame, COLOR_BGR2GRAY);

        //calcOpticalFlowPyrLK(first_gray_frame, gray_current_frame, prev_pts, next_pts, status, err);

        //find_good_fp(status, prev_pts, good_old_fp);
        //find_good_fp(status, next_pts, good_new_fp);

        calcOpticalFlowPyrLK(first_gray_frame, gray_current_frame, horizon_pts, next_pts, status, err);

        //std::cout << "new horizon_x1 == " << next_pts.at<unsigned char>(0, 0) << 
        //    " new horizon_y1 == " << next_pts.at<unsigned char>(0, 1) << "\n";
        //std::cout << "new horizon_x2 == " << next_pts.at<unsigned char>(0, 1) << 
        //    " new horizon_y2 == " << next_pts.at<unsigned char>(1, 1) << "\n\n\n";


        line( current_frame, 
        Point(next_pts.at<unsigned char>(0, 0), next_pts.at<unsigned char>(1, 0)),
        Point(next_pts.at<unsigned char>(0, 1), next_pts.at<unsigned char>(1, 1)),
        Scalar(0,0,255), 3, 8 ); 

        imshow("video_in.AVI", current_frame);

        horizon_pts.at<double>(0, 0) = next_pts.at<unsigned char>(0, 0);
        horizon_pts.at<double>(1, 0) = next_pts.at<unsigned char>(1, 0);

        horizon_pts.at<double>(0, 1) = next_pts.at<unsigned char>(0, 1);
        horizon_pts.at<double>(1, 1) = next_pts.at<unsigned char>(1, 1);

        char c{(char)waitKey(25)};
        if (c == 27)
            break;

    }


    cap.release();
    destroyAllWindows();


}
