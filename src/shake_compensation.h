#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video/tracking.hpp> 
#include "structures.h"

using namespace cv;

std::vector<transform_parameters> transf_build(VideoCapture cap, long long frame_num);
std::vector<trajectory> cumulative_sum(std::vector<transform_parameters> &transforms); 
std::vector<trajectory> smooth(std::vector <trajectory>& traj, int radius); 
std::vector<transform_parameters> get_smooth_transforms(std::vector<trajectory> smoothed_trajectory, std::vector<trajectory> original_trajectory, std::vector<transform_parameters> transforms);
void get_stabilized_frame(Mat& current_frame, transform_parameters current_frame_transf_smooth);