#include <iostream> 
#include <vector> 
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video/tracking.hpp>

using namespace std;
using namespace cv;

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
    VideoCapture cap{VideoCapture("video_in.AVI")};
    Mat first_frame, first_gray_frame, prev_frame, next_frame, current_frame, gray_current_frame, 
        prev_pts, next_pts, status, err, good_old_fp, good_new_fp;

    cap.read(first_frame);
    cvtColor(first_frame, first_gray_frame, COLOR_BGR2GRAY);

    goodFeaturesToTrack(first_gray_frame, prev_pts, 300, 0.2, 2);

    while(cap.isOpened())
    {
        cap.read(current_frame);
        cvtColor(current_frame, gray_current_frame, COLOR_BGR2GRAY);

        calcOpticalFlowPyrLK(first_gray_frame, gray_current_frame, prev_pts, next_pts, status, err);

        find_good_fp(status, prev_pts, good_old_fp);
        find_good_fp(status, next_pts, good_new_fp);

        

    }


    


}
