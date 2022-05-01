#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <vector>
#include <QtWidgets>
#include "structures.h"


using namespace cv;

int start_process(std::string file_name, QLabel* img_lbl, QLineEdit* curr_frame);

void show_and_detect_cycle(unsigned long long frame_num,
    std::vector<Mat> all_frames_vec, std::vector<transform_parameters> smooth_transforms, std::vector< std::vector<Point2f> >all_found_fp_vec,
    double horizon_x1, double horizon_y1, double horizon_x2, double horizon_y2, QLabel* img_lbl, QLineEdit* curr_frame);

#endif // MAIN_H
