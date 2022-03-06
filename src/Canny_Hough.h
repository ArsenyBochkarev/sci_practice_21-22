#include <vector>
#include <utility>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
		
std::vector<std::pair<double, double> > get_coordinates_Canny_Hough(cv::Mat src);
