#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <vector>
#include <QtWidgets>
#include "structures.h"


using namespace cv;


std::vector<std::pair<double, double> > get_horizon_coordinates(Mat src, bool way_to_get_horizon);

std::vector<transform_parameters> get_smooth_transforms_func(unsigned long long all_frames_num, VideoCapture pre_cap1,
                                                             std::vector<std::vector<Point2f> > all_frames_fp );


int start_process(std::string file_name, QLabel* img_lbl, QLineEdit* curr_frame);

void show_and_detect_cycle(unsigned long long frame_num,
    std::vector<Mat> all_frames_vec, std::vector<transform_parameters> smooth_transforms, std::vector< std::vector<Point2f> >all_found_fp_vec,
    double horizon_x1, double horizon_y1, double horizon_x2, double horizon_y2, QLabel* img_lbl, QLineEdit* curr_frame);



#endif // MAIN_H
