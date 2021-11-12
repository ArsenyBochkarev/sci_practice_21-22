#include <iostream> 
#include <vector> 
#include <cmath>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video/tracking.hpp>

using namespace std;
using namespace cv;

double SCREEN_SIZE_X{1920};
double SCREEN_SIZE_Y{1080};

 
int main()
{
    Mat prev_frame, prev_gray_frame, current_frame, current_gray_frame, status, err;
    
 
    VideoCapture cap("video_in.AVI");


    if(!cap.isOpened())
    {
        std::cout << "Error opening video stream or file \n\n\n";
        return -1;
    }


    double horizon_x1, horizon_y1;
    double horizon_x2, horizon_y2;


    // Считываем первый кадр
    cap.read(prev_frame);

    // Вычисляем горизонт
    // some code

    // ВРЕМЕННО !!! Выставляю дефолтные значения примерно посередине кадра
    horizon_x1 = 0;
    horizon_y1 = prev_frame.size().height/2 - 1;

    horizon_x2 = prev_frame.size().width;
    horizon_y2 = prev_frame.size().height/2 + 1;


    // Меняем цвет
    cvtColor(prev_frame, prev_gray_frame, COLOR_BGR2GRAY); 


    // Векторы, хранящие в себе "фичи" - feature points
    std::vector<Point2f> prev_found_fp;
    std::vector<Point2f> current_found_fp;
    std::vector<Point2f> current_changed_fp;


    // Ищем "фичи" на первом кадре
    goodFeaturesToTrack(prev_gray_frame, prev_found_fp, 300, 0.2, 2);


    // Счётчик кадров
    unsigned long long frame_num{0};


    // Раз в сколько кадров будет возможно пересчитывание горизонта - сделано в угоду производительности
    // В дальнейшем сделать регулируемым
    int change_rate{3};


    // Наибольшая разрешенная разница между суммированными расстояниями от "фич" до горизонта на прошлом и текущем кадре
    // В дальнейшем сделать регулируемым
    long double dist_diff{600};


    // Наибольшая разрешенная разница между количеством старых feature points и новых
    // В дальнейшем сделать регулируемым
    int fp_diff{10};


    while(cap.isOpened())
    {
        frame_num++;

        cap.read(current_frame);
        if (current_frame.empty())
            break;

        if (frame_num % change_rate == 0)
        {
            // Меняем цвет, затем считаем optical flow 
            cvtColor(current_frame, current_gray_frame, COLOR_BGR2GRAY);
            calcOpticalFlowPyrLK(prev_gray_frame, current_gray_frame, prev_found_fp, current_changed_fp, status, err); 


            // Ещё раз ищем "фичи", только уже на новом кадре
            goodFeaturesToTrack(current_gray_frame, current_found_fp, 300, 0.2, 2);


            // Сравниваем старые "фичи" на новом кадре и только что найденные "фичи"
            // Если положение "фич" и/или их число лишь немного изменилось, то менять горизонт смысла нет, иначе - пересчитываем
            bool rebuild_horizon{0};

            // Сначала сравним их количество
            if (abs(prev_found_fp.size() - current_found_fp.size()) <= fp_diff)
            {
                // Если количество прошло - сравним суммарные расстояния до горизонта, отмеченного на предыдущем кадре
                // Считаем коэффициенты в уравнении прямой горизонта
                double A{1/(horizon_x1 - horizon_x2)};
                double B{1/(horizon_y2 - horizon_y1)};
                double C{1/(horizon_y1 - horizon_y2) - 1/(horizon_x1 - horizon_x2)};


                double old_dist{1};
                double new_dist{1};


                // Старые точки
                for (long long i{0}; i < current_changed_fp.size(); i++) 
                    old_dist += abs( (A*current_changed_fp[i].x + B*current_changed_fp[i].y + C) / sqrt(A*A + B*B) ); 

                // Новые точки
                for (long long i{0}; i < current_found_fp.size(); i++)
                    new_dist += abs( (A*current_found_fp[i].x + B*current_found_fp[i].y + C) / sqrt(A*A + B*B) );

                /*
                std::cout << "\n\nframe_num == " << frame_num << "\n";
                std::cout << "old_dist == " << old_dist << "\n";
                std::cout << "new_dist == " << new_dist << "\n";
                */


                if (abs(old_dist - new_dist) > dist_diff)
                {
                    rebuild_horizon = 1;
                }

            }
            else
                rebuild_horizon = 1;


            if (rebuild_horizon)
            {
                std::cout << "frame number " << frame_num << " has found the difference !\n\n";
                // some code
                
                // ВРЕМЕННО !!! Выставляю значения примерно посередине кадра
                horizon_x1 = 0;
                horizon_y1 = prev_frame.size().height/2 - 1;

                horizon_x2 = prev_frame.size().width;
                horizon_y2 = prev_frame.size().height/2 + 1;
            }


            // Новые "фичи" - теперь старые 
            prev_found_fp.resize(current_found_fp.size());
            for (long long i{0}; i < current_found_fp.size(); i++)
                prev_found_fp[i] = current_found_fp[i]; 
        }

        

        // Рисуем горизонт
        line(current_frame, Point2f(horizon_x1, horizon_y1), Point2f(horizon_x2, horizon_y2), Scalar(0,0,255), 3, 8 );



        // Показываем картинку
        imshow("video_in.AVI", current_frame); 


        // Если пользователь нажмет клавишу "Escape" - цикл прервётся и программа завершится 
        char c{(char)waitKey(25)};
        if (c == 27)
            break; 

    }


    cap.release();
    destroyAllWindows();


}
