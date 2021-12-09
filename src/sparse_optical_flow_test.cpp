#include <iostream> 
#include <vector> 
#include <cmath>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video/tracking.hpp>
#include "shake_compensation.h"

using namespace std;
using namespace cv;

double SCREEN_SIZE_X{1920};
double SCREEN_SIZE_Y{1080};

 
int main()
{
    Mat prev_frame, prev_gray_frame, current_frame, current_gray_frame;
    std::vector <unsigned char> status;
    std::vector <float> err;
    
 
    VideoCapture cap("video_in2.mp4");
    VideoCapture pre_cap("video_in2.mp4");
    //VideoCapture cap("video_in.AVI");
    //VideoCapture pre_cap("video_in.AVI");

    if(!cap.isOpened())
    {
        std::cout << "Error opening video stream or file \n\n\n";
        return -1;
    }


    double horizon_x1, horizon_y1;
    double horizon_x2, horizon_y2;


 
    // Пре-проход по всем кадрам для построения матриц перехода и "сглаженных" матриц перехода между кадрами
    // Нужен для стабилизации видео (shake compensation)
    // Узнаем общее количество кадров в видео
    long long all_frames_num{static_cast<long long>(pre_cap.get(CAP_PROP_FRAME_COUNT))};
    
    // Строим матрицу изменений между кадрами
    std::vector<transform_parameters> transforms;
    transforms = transf_build(pre_cap, all_frames_num-1);

    // Строим "траекторию" для каждого кадра - промежуточные суммы изменений по x, y и углу
    std::vector<trajectory> traj;
    traj = cumulative_sum(transforms);

    // Строим "сглаженную" траекторию - к промежуточным суммам применяются moving average filter
    std::vector<trajectory> smooth_trajectory;

    // Радиус для moving average filter 
    // В дальнейшем сделать регулируемым
    int radius{5}; 

    smooth_trajectory = smooth(traj, radius);

    // Строим "сглаженные" матрицы изменений между кадрами, которые в дальнейшем будем применять к каждому отдельно взятому кадру
    std::vector<transform_parameters> smooth_transforms;
    smooth_transforms = get_smooth_transforms(smooth_trajectory, traj, transforms);

    std::cout << "\n\n\n\nall frames number == " << all_frames_num << "\n";
    std::cout << "smooth_transforms size == " << smooth_transforms.size() << "\n\n\n\n";
    


    // Считываем первый кадр
    cap.read(prev_frame);
    if (prev_frame.empty())
    {
        std::cout << "unable to read prev_frame ! \n";
        return -1;
    }

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
    std::vector<Point2f> current_changed_fp;


    // Ищем "фичи" на первом кадре
    goodFeaturesToTrack(prev_gray_frame, prev_found_fp, 300, 0.2, 2); 


    // Счётчик кадров
    unsigned long long frame_num{1};


    // Раз в сколько кадров будет возможно пересчитывание горизонта - сделано в угоду производительности
    // В дальнейшем сделать регулируемым
    unsigned int change_rate{3};


    // Наибольшая разрешенная разница между суммированными расстояниями до горизонта между "фичами" на прошлом и текущем кадре
    // В дальнейшем сделать регулируемым
    double max_sum_horizon_diff{1200};


    // Наибольшая разрешенная разница между соответствующими расстояниями до горизонта у одной конкретной "фичи" на прошлом и текущем кадре
    // В дальнейшем сделать регулируемым
    double max_single_point_horizon_diff{400};


    // Наибольшее разрешенное число точек, разница между соответствующими расстояниями до горизонта на первом и втором кадре превышает max_single_point_horizon_diff
    // В дальнейшем сделать регулируемым
    unsigned int max_wrong_fp_num{5};


    // Наибольшая разрешенная разница между общим количеством "фич" и числом точек, сместившихся в конкретном направлении (вверх или вниз)
    // В дальнейшем сделать регулируемым
    long long max_horizon_correctness_diff{20};
 




    for(;frame_num < all_frames_num-1; frame_num++)
    {
        


        cap.read(current_frame);
        if (current_frame.empty())
        {
            std::cout << "unable to read current_frame ! \n";
            break;
        }


        // Стабилизируем текущий кадр относительно предыдущего (shake compensation) 
        get_stabilized_frame(current_frame, smooth_transforms[frame_num]);


        cvtColor(current_frame, current_gray_frame, COLOR_BGR2GRAY);


        if (frame_num % change_rate == 0)
        {  
            // Считаем optical flow
            calcOpticalFlowPyrLK(prev_gray_frame, current_gray_frame, prev_found_fp, current_changed_fp, status, err); 

            long long fp_num{prev_found_fp.size()};


            
            // Сравним положение "фич" на двух соседних кадрах
            // Если положение "фич" лишь немного изменилось, то пересчитывать горизонт смысла нет, и нужно лишь немного скорректировать его положение
            // Иначе - полностью перестраиваем его
            bool rebuild_horizon{0};

            
            // Сравним суммарные расстояния до горизонта, отмеченного на предыдущем кадре
            // Это делается, чтобы понять, нужно ли пересчитывать горизонт полностью, либо достаточно лишь немного подкорректировать его относительно нового положения "фич"

            // Считаем коэффициенты в уравнении прямой горизонта
            double A{1/(horizon_x1 - horizon_x2)};
            double B{1/(horizon_y2 - horizon_y1)};
            double C{1/(horizon_y1 - horizon_y2) - 1/(horizon_x1 - horizon_x2)};


            // Суммированное, для каждой конкретной точки и среднее расстояния до горизонта на старом и новом кадрах
            double old_sum_horizon_dist{0};
            double old_single_point_dist;
            double old_avg_single_p_dist{0};

            double new_sum_horizon_dist{0};
            double new_single_point_dist;
            double new_avg_single_p_dist{0};


            // Будем запоминать количество точек, у которых расстояние до горизонта сильно разнится по сравнению с предыдущим кадром
            unsigned int wrong_fp_num{0};


            // Переменная для определения того, куда следует сместить горизонт
            // Если какая-то точка поднялась выше относительно своего положения на предыдущем кадре - прибавляем единицу, если опустилась - отнимаем
            // В случае, если почти все точки были внесены в переменную с одним знаком, можно определить, в какую сторону стоит корректировать горизонт
            // Если же изменение положения точек будет разнится - корректировать нет смысла 
            long long horizon_correctness{0};


            // Подсчёт расстояния
            for (long long i{0}; i < fp_num; i++) 
            {
                old_single_point_dist = abs( (A*prev_found_fp[i].x + B*prev_found_fp[i].y + C) / sqrt(A*A + B*B) );
                new_single_point_dist = abs( (A*current_changed_fp[i].x + B*current_changed_fp[i].y + C) / sqrt(A*A + B*B) );

                // В случае, если хотя бы одна точка "ускакала" куда-то вверх или вниз - запомним её и в случае, если подобных точек не будет много, просто не будем на них ориентироваться
                if (abs(old_single_point_dist - new_single_point_dist) >= max_single_point_horizon_diff) 
                    wrong_fp_num++;  
                else 
                {
                    old_sum_horizon_dist += old_single_point_dist;  
                    new_sum_horizon_dist += new_single_point_dist;  


                    // Вносим точки для проверки корректировки горизонта
                    if (new_single_point_dist > old_single_point_dist)
                        horizon_correctness++;
                    else
                        horizon_correctness--;                   
                }
            }

            // Корректируем число "хороших" точек
            fp_num = fp_num - wrong_fp_num;            


            old_avg_single_p_dist = ( (old_sum_horizon_dist) / fp_num );
            new_avg_single_p_dist = ( (new_sum_horizon_dist) / fp_num );




            // Если разница в расстояниях до горизонта велика (что равносильно большому количеству "плохих фич") - это основание для полной перестройки горизонта
            if ( (abs(new_sum_horizon_dist - old_sum_horizon_dist) > max_sum_horizon_diff) || (wrong_fp_num > max_wrong_fp_num) )
            {
                std::cout << "rebuild !!! \n";
                rebuild_horizon = 1;
            }
            else
                // Если можем определить конкретное направление корректировки - делаем ее относительно средней разницы расстояний
                if ( (abs(horizon_correctness) > fp_num - max_horizon_correctness_diff) && (!rebuild_horizon) )
                { 
                    std::cout << "frame number " << frame_num << " asked to correct the horizon !\n\n";

                    if (horizon_correctness > 0)
                    {
                        horizon_y1 += abs(old_avg_single_p_dist - new_avg_single_p_dist);
                        horizon_y2 += abs(old_avg_single_p_dist - new_avg_single_p_dist);
                    }
                    else
                    {
                        horizon_y1 -= abs(old_avg_single_p_dist - new_avg_single_p_dist);
                        horizon_y2 -= abs(old_avg_single_p_dist - new_avg_single_p_dist);
                    }
                }



            if (rebuild_horizon)
            { 
                std::cout << "frame number " << frame_num << " asked to rebuild the horizon !\n\n";
                // some code
                
                // ВРЕМЕННО !!! Выставляю значения примерно посередине кадра
                horizon_x1 = 0;
                horizon_y1 = prev_frame.size().height/2 - 1;

                horizon_x2 = prev_frame.size().width;
                horizon_y2 = prev_frame.size().height/2 + 1;
            }


            // Вывод технических параметров
            std::cout << "frame number == " << frame_num << "\n\n";
            std::cout << "fp_num == " << fp_num << "\n";
            std::cout << "wrong_fp_num == " << wrong_fp_num << "\n";
            std::cout << "old_sum_horizon_dist == " << old_sum_horizon_dist << "\n";
            std::cout << "new_sum_horizon_dist == " << new_sum_horizon_dist << "\n";
            std::cout << "old_avg_single_p_dist == " << old_avg_single_p_dist << "\n";
            std::cout << "new_avg_single_p_dist == " << new_avg_single_p_dist << "\n";
            std::cout << "horizon_correctness == " << horizon_correctness << "\n\n";


        }
        else  
        {
            current_frame.copyTo(prev_frame);

            cvtColor(prev_frame, prev_gray_frame, COLOR_BGR2GRAY);

            // Обновляем данные
            prev_found_fp.clear(); 
            goodFeaturesToTrack(prev_gray_frame, prev_found_fp, 300, 0.2, 2);

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
