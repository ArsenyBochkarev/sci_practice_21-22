#include <iostream>  
#include <cmath> 
#include "shake_compensation.h"
#include "Canny_Hough.h"
#include "Gauss_threshold.h"

using namespace cv;

// Функция первоначально определения горизонта
std::vector<std::pair<double, double> > get_horizon_coordinates(Mat src, bool way_to_get_horizon)
{
    // 0 - метод через Canny/Hough
    // 1 - метод через Gauss/threshold
    return std::move((way_to_get_horizon == 0 ? get_coordinates_Canny_Hough(src) : get_coordinates_Gauss_threshold(src)));
}

 
int main()
{
    Mat prev_frame, prev_gray_frame, current_frame, current_gray_frame;
    std::vector <unsigned char> status;
    std::vector <float> err;
    
 
    VideoCapture cap("video_in2.mp4");
    VideoCapture pre_cap1("video_in2.mp4");
    VideoCapture pre_cap2("video_in2.mp4");

    if(!cap.isOpened())
    {
        std::cout << "Error opening video stream or file \n\n\n";
        return -1;
    }


    double horizon_x1, horizon_y1{0};
    double horizon_x2, horizon_y2{0};




    // Пре-проход по всем кадрам для построения матриц перехода и "сглаженных" матриц перехода между кадрами
    // Нужен для стабилизации видео (shake compensation)
    // Узнаем общее количество кадров в видео
    long long all_frames_num{static_cast<long long>(pre_cap1.get(CAP_PROP_FRAME_COUNT))};
    
    // Строим матрицу изменений между кадрами
    std::vector<transform_parameters> transforms;
    transforms = transf_build(pre_cap1, all_frames_num-1);

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




    // Второй пре-проход по всем кадрам: сохраняем их в массив.
    // Также создаём массив с соответствующими парами y-координат горизонта (x-координаты -- левая и правая границы кадра)
    // Причём неважно, что мы проходим по не стабилизированным кадрам -- стабилизация влияет лишь на работу по отслеживанию перемещений горизонта
    Mat pre_cap_frame;
    std::vector<Mat> all_frames_vec(all_frames_num);
    std::vector<std::pair<double, double> > all_frames_coords_vec(all_frames_num);

    std::cout << "Saving all frames...\n";

    for(long long i{0}; i < all_frames_num; i++)
    {
        pre_cap2.read(pre_cap_frame);
        if (pre_cap_frame.empty())
        {
            std::cout << "unable to read pre_cap_frame on frame number " << i << "\n";
            return -1;
        }
        pre_cap_frame.copyTo(all_frames_vec[i]);

        // Пока что не важно, чем они инициализируются
        all_frames_coords_vec[i].first = horizon_y1;
        all_frames_coords_vec[i].second = horizon_y2;
    }




    // Способ построения горизонта
    // В дальнейшем сделать регулируемым
    bool horizon_detection_method{0};




    // Берём первый кадр для того, чтобы пометить горизонт на нём
    all_frames_vec[0].copyTo(prev_frame);




    // Вычисляем горизонт
    std::vector<std::pair<double, double> > coords{get_horizon_coordinates(prev_frame, horizon_detection_method)};
    horizon_x1 = coords[0].first;
    horizon_y1 = coords[0].second;

    horizon_x2 = coords[1].first;
    horizon_y2 = coords[1].second; 

    std::cout << "horizon_x1 == " << horizon_x1 << " horizon_y1 == " << horizon_y1 << "\n";
    std::cout << "horizon_x2 == " << horizon_x2 << " horizon_y2 == " << horizon_y2 << "\n\n\n\n\n\n\n\n";

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
    unsigned int change_rate{5};


    // Наибольшая разрешенная разница между суммированными расстояниями до горизонта между "фичами" на прошлом и текущем кадре
    // В дальнейшем сделать регулируемым
    double max_sum_horizon_diff{1100};


    // Наибольшая разрешенная разница между соответствующими расстояниями до горизонта у одной конкретной "фичи" на прошлом и текущем кадре
    // В дальнейшем сделать регулируемым
    double max_single_point_horizon_diff{300};


    // Наибольшее разрешенное число точек, у которых разница между соответствующими расстояниями до горизонта на первом и втором кадре превышает max_single_point_horizon_diff
    // В дальнейшем сделать регулируемым
    unsigned int max_wrong_fp_num{3};


    // Наибольшая разрешенная разница между общим количеством "фич" и числом точек, сместившихся в конкретном направлении (вверх или вниз)
    // В дальнейшем сделать регулируемым
    long long max_horizon_correctness_diff{10};




    for(;frame_num < all_frames_num-1; frame_num++)
    {
        
        all_frames_vec[frame_num].copyTo(current_frame);
 

        // Стабилизируем текущий кадр относительно предыдущего (shake compensation) 
        get_stabilized_frame(current_frame, smooth_transforms[frame_num]);


        cvtColor(current_frame, current_gray_frame, COLOR_BGR2GRAY);


        if (frame_num % change_rate == 0)
        {  
            // Считаем optical flow
            calcOpticalFlowPyrLK(prev_gray_frame, current_gray_frame, prev_found_fp, current_changed_fp, status, err); 

            unsigned long long fp_num{prev_found_fp.size()};


            
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




            // Если разница в расстояниях до горизонта велика (что равносильно большому количеству "плохих фич") - 
            // это основание для полной перестройки горизонта
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
                
                std::vector<std::pair<double, double> > coords{get_horizon_coordinates(current_frame, horizon_detection_method)};
                horizon_x1 = coords[0].first;
                horizon_y1 = coords[0].second;

                horizon_x2 = coords[1].first;
                horizon_y2 = coords[1].second;

                std::cout << "horizon_x1 == " << horizon_x1 << " horizon_y1 == " << horizon_y1 << "\n";
                std::cout << "horizon_x2 == " << horizon_x2 << " horizon_y2 == " << horizon_y2 << "\n\n\n\n\n\n\n\n";

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

            if (change_rate == 1) // иначе данные не будут обновляться в случае change_rate == 1
            {
                current_frame.copyTo(prev_frame);

                cvtColor(prev_frame, prev_gray_frame, COLOR_BGR2GRAY);

                // Обновляем данные
                prev_found_fp.clear(); 
                goodFeaturesToTrack(prev_gray_frame, prev_found_fp, 300, 0.2, 2);

            }  
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
        Mat painted_frame;
        current_frame.copyTo(painted_frame);
        line(painted_frame, Point2f(horizon_x1, horizon_y1), Point2f(horizon_x2, horizon_y2), Scalar(0,0,255), 3, 8 );



        // Показываем картинку
        imshow("video_in.AVI", painted_frame); 
        



        // Если пользователь нажмет клавишу "Escape" - цикл прервётся и программа завершится 
        char c{(char)waitKey(25)};
        if (c == 27)
            break; 

    }


    cap.release();
    destroyAllWindows();
}
