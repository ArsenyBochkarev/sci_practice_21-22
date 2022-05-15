#include <iostream>
#include <cmath>

#include <QApplication>
#include <QPushButton>
#include <QMainWindow>
#include <QtWidgets>


#include "mainwindow.h"

#include "qmessagebox.h"
#include "shake_compensation.h"
#include "Canny_Hough.h"
#include "Gauss_threshold.h"

using namespace cv;

// Функция первоначального определения горизонта
std::vector<std::pair<double, double> > get_horizon_coordinates(Mat src, bool way_to_get_horizon)
{
    // 0 - метод через Canny/Hough
    // 1 - метод через Gauss/threshold
    return std::move((way_to_get_horizon == 0 ? get_coordinates_Canny_Hough(src) : get_coordinates_Gauss_threshold(src)));
}




std::vector<transform_parameters> get_smooth_transforms_func(unsigned long long all_frames_num, VideoCapture pre_cap1, std::vector<std::vector<Point2f> > all_frames_fp)
{
    std::vector<transform_parameters> smooth_transforms;

    // Строим матрицу изменений между кадрами
    std::vector<transform_parameters> transforms;
    transforms = transf_build(pre_cap1, all_frames_num-1, all_frames_fp);

    // Строим "траекторию" для каждого кадра - промежуточные суммы изменений по x, y и углу
    std::vector<trajectory> traj;
    traj = cumulative_sum(transforms);

    // Строим "сглаженную" траекторию - к промежуточным суммам применяются moving average filter
    std::vector<trajectory> smooth_trajectory;

    // Радиус для moving average filter
    // В дальнейшем сделать регулируемым
    int radius{5};

    smooth_trajectory = smooth(traj, radius);

    smooth_transforms = get_smooth_transforms(smooth_trajectory, traj, transforms);

    return smooth_transforms;
}



















int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    MainWindow w;

    w.showMaximized();

    return a.exec();
}
