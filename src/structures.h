#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/video/tracking.hpp"

using namespace cv;

// Структура, хранящая параметры изменений кадров
struct transform_parameters
{
    transform_parameters() {}

    transform_parameters(double _dx, double _dy, double _da) : dx(std::move(_dx)), dy(std::move(_dy)), da(std::move(_da)) {} 

    // Метод для построения матрицы "перехода" (transformation matrix)
    void get_transf_mat(Mat &transf_mat)
    {

        transf_mat.at<double>(0,0) = cos(da);
        transf_mat.at<double>(0,1) = -sin(da);
        transf_mat.at<double>(1,0) = sin(da);
        transf_mat.at<double>(1,1) = cos(da);


        transf_mat.at<double>(0,2) = dx;
        transf_mat.at<double>(1,2) = dy;
    }

    double dx; // Изменение по x
    double dy; // Изменение по y
    double da; // Изменение угла
};




// Структура, хранящая промежуточные суммы значений изменений кадров
struct trajectory
{
    trajectory() {}    
    trajectory(double _x, double _y, double _a) : x(std::move(_x)), y(std::move(_y)), a(std::move(_a)) {}

    double x;
    double y;
    double a; 

};
