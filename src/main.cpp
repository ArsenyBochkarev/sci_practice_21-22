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

 

std::vector<transform_parameters> get_smooth_transforms_func(unsigned long long all_frames_num, VideoCapture pre_cap1)
{
    std::vector<transform_parameters> smooth_transforms;

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

    smooth_transforms = get_smooth_transforms(smooth_trajectory, traj, transforms);

    return smooth_transforms;
}



void show_and_detect_cycle(long long frame_num,  
    std::vector<Mat> all_frames_vec, std::vector<transform_parameters> smooth_transforms, std::vector< std::vector<Point2f> >all_found_fp_vec, 
    double horizon_x1, double horizon_y1, double horizon_x2, double horizon_y2)
{
 

    unsigned long long all_frames_num{all_frames_vec.size()};



    // --------------------------------------------------------------------------------------------------------------------------------------
    // Эти параметры будут выниматься каждый раз из соответствующих специальных полей

    // Раз в сколько кадров будет возможно пересчитывание горизонта - сделано в угоду производительности
    // В дальнейшем сделать регулируемым
    unsigned int change_rate{3};


    // Способ построения горизонта
    // В дальнейшем сделать регулируемым
    bool horizon_detection_method{0};


    // Наибольшая разрешенная разница между суммированными расстояниями до горизонта между "фичами" на прошлом и текущем кадре
    // В дальнейшем сделать регулируемым
    double max_sum_horizon_diff{1200};


    // Наибольшая разрешенная разница между соответствующими расстояниями до горизонта у одной конкретной "фичи" на прошлом и текущем кадре
    // В дальнейшем сделать регулируемым
    double max_single_point_horizon_diff{400};


    // Наибольшее разрешенное число точек, у которых разница между соответствующими расстояниями до горизонта на 
    // первом и втором кадре превышает max_single_point_horizon_diff
    // В дальнейшем сделать регулируемым
    unsigned int max_wrong_fp_num{5};


    // Наибольшая разрешенная разница между общим количеством "фич" и числом точек, сместившихся в конкретном направлении (вверх или вниз)
    // В дальнейшем сделать регулируемым
    long long max_horizon_correctness_diff{20}; 
    // -------------------------------------------------------------------------------------------------------------------------------------- 


    Mat current_frame, current_gray_frame;

    all_frames_vec[frame_num].copyTo(current_frame);

    cvtColor(current_frame, current_gray_frame, COLOR_BGR2GRAY);


    for(;frame_num < all_frames_num-1; frame_num++)
    {
        
        all_frames_vec[frame_num].copyTo(current_frame);
 

        // Стабилизируем текущий кадр относительно предыдущего (shake compensation) 
        get_stabilized_frame(current_frame, smooth_transforms[frame_num]);
        cvtColor(current_frame, current_gray_frame, COLOR_BGR2GRAY);
 

        if (frame_num % change_rate == 0)
        {
            Mat prev_frame, prev_gray_frame;  
            
            all_frames_vec[frame_num-1].copyTo(prev_frame);    
            get_stabilized_frame(prev_frame, smooth_transforms[frame_num-1]);
            cvtColor(prev_frame, prev_gray_frame, COLOR_BGR2GRAY); 
    
            std::vector<Point2f> prev_found_fp{all_found_fp_vec[frame_num]};



            std::vector <unsigned char> status;
            std::vector <float> err;
            std::vector<Point2f> current_changed_fp;

 
            // Считаем optical flow
            calcOpticalFlowPyrLK(prev_gray_frame, current_gray_frame, prev_found_fp, current_changed_fp, status, err); 

            unsigned long long fp_num{prev_found_fp.size()};


            
            // Сравним положение "фич" на двух соседних кадрах
            // Если положение "фич" лишь немного изменилось, то пересчитывать горизонт смысла нет, и нужно лишь немного скорректировать его положение
            // Иначе - полностью перестраиваем его
            bool rebuild_horizon{0};

            
            // Сравним суммарные расстояния до горизонта, отмеченного на предыдущем кадре
            // Это делается, чтобы понять, нужно ли пересчитывать горизонт полностью, либо достаточно лишь немного 
            // подкорректировать его относительно нового положения "фич"

            // Считаем коэффициенты в уравнении прямой горизонта
            double A{(horizon_x2 - horizon_x1 != 0) ? 1/(horizon_x2 - horizon_x1) : 1/(horizon_x2 + 1 - horizon_x1)};
            double B{(horizon_y2 - horizon_y1 != 0) ? 1/(horizon_y2 - horizon_y1) : 1/(horizon_y2 + 1 - horizon_y1)};
            double C{B - A};



            /*
            double A{1/(horizon_x1 - horizon_x2)};
            double B{1/(horizon_y2 - horizon_y1)};
            double C{1/(horizon_y1 - horizon_y2) - 1/(horizon_x1 - horizon_x2)};
            */



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

                // В случае, если хотя бы одна точка "ускакала" куда-то вверх или вниз - запомним её и в случае, если
                // подобных точек не будет много, просто не будем на них ориентироваться
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
}















int main()
{
    Mat prev_frame;
    
 
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

    // Строим "сглаженные" матрицы изменений между кадрами, которые в дальнейшем будем применять к каждому отдельно взятому кадру
    std::cout << "Doing shake compensation...\n";
    std::vector<transform_parameters> smooth_transforms{get_smooth_transforms_func(all_frames_num, pre_cap1)};

    std::cout << "\n\n\n\nall frames number == " << all_frames_num << "\n";
    std::cout << "smooth_transforms size == " << smooth_transforms.size() << "\n\n\n\n";




    // Второй пре-проход по всем кадрам: сохраняем их и найденные на них фичи в соответствующие массивы.
    // Также создаём массив с соответствующими парами y-координат горизонта (x-координаты -- левая и правая границы кадра)
    // Причём неважно, что мы проходим по не стабилизированным кадрам -- стабилизация влияет лишь на работу по отслеживанию перемещений горизонта
    std::vector<Mat> all_frames_vec(all_frames_num);
    std::vector<std::vector<Point2f> > all_found_fp_vec;
    std::vector<std::pair<double, double> > all_frames_coords_vec(all_frames_num);

    std::cout << "Saving all frames and their feature points...\n";

    for(long long i{0}; i < all_frames_num; i++)
    {
        Mat pre_cap_frame, pre_cap_gray_frame;
        
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


        get_stabilized_frame(pre_cap_frame, smooth_transforms[i]);
        cvtColor(pre_cap_frame, pre_cap_gray_frame, COLOR_BGR2GRAY);
        std::vector<Point2f> tmp_fp;
        goodFeaturesToTrack(pre_cap_gray_frame, tmp_fp, 300, 0.2, 2); 
        all_found_fp_vec.push_back(tmp_fp);
    } 



    // Способ построения горизонта
    // В дальнейшем сделать регулируемым
    bool horizon_detection_method{0};

    // Счётчик кадров
    unsigned long long frame_num{20};



    // Берём первый кадр для того, чтобы пометить горизонт на нём
    all_frames_vec[frame_num-1].copyTo(prev_frame);




    // Вычисляем горизонт
    std::vector<std::pair<double, double> > coords{get_horizon_coordinates(prev_frame, horizon_detection_method)};
    horizon_x1 = coords[0].first;
    horizon_y1 = coords[0].second;

    horizon_x2 = coords[1].first;
    horizon_y2 = coords[1].second; 

    std::cout << "horizon_x1 == " << horizon_x1 << " horizon_y1 == " << horizon_y1 << "\n";
    std::cout << "horizon_x2 == " << horizon_x2 << " horizon_y2 == " << horizon_y2 << "\n\n\n\n\n\n\n\n";
  
 




    

    // Важно!!!! frame_num должен быть строго больше нуля !!!!
    show_and_detect_cycle(frame_num, all_frames_vec, smooth_transforms, all_found_fp_vec, horizon_x1, horizon_y1, horizon_x2, horizon_y2);
    


    cap.release();
    destroyAllWindows();
}
