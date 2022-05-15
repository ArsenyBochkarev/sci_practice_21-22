#include <iostream>  
#include <cmath> 
#include "shake_compensation.h"

using namespace std;
using namespace cv;





// Функция подсчёта изменений для каждого кадра
std::vector<transform_parameters> transf_build(VideoCapture cap, long long frame_num, std::vector<std::vector<Point2f> > all_frames_fp)
{
    Mat prev_frame, prev_gray_frame, current_frame, current_gray_frame, status, err;

	// Храним изменения между кадрами в этом векторе
	std::vector <transform_parameters> transforms; 

	// "Бэкап"-матрица на случай, если estimateRigidTransform ничего не выдаст
    Mat last_transf;

	
    cap.read(prev_frame);
    if (prev_frame.empty())
    {
    	std::cout << "Unable to open video stream or file in function transf_build()  (prev_frame) ! \n";
    	return transforms;
    }

    cvtColor(prev_frame, prev_gray_frame, COLOR_BGR2GRAY);



	for(int i{0}; i < frame_num; i++)
    {
	    std::vector<Point2f> prev_found_fp;
        std::vector<Point2f> current_changed_fp;



        prev_found_fp = all_frames_fp[i];
		
        cap.read(current_frame);
        if (current_frame.empty())
	    {
            std::cout << "Unable to open video stream or file in function transf_build() (current_frame)!\n";
            return transforms;
	    }


		cvtColor(current_frame, current_gray_frame, COLOR_BGR2GRAY);
 
	    
	    // Векторы-параметры для calcOpticalFlowPyrLK
	    std::vector <unsigned char> status;
		std::vector <float> err;

		// Считаем optical flow на текущем кадре
		calcOpticalFlowPyrLK(prev_gray_frame, current_gray_frame, prev_found_fp, current_changed_fp, status, err);


		// Берём только "хорошие фичи" (т.е. такие, у которых в векторе status стоит единица)
		auto prev_it{prev_found_fp.begin()}; 
		auto curr_it{current_changed_fp.begin()}; 
		for(size_t k{0}; k < status.size(); k++)  
		    if(status[k]) 
		    {
				prev_it++; 
				curr_it++; 
		    }
		    else 
		    {
				prev_it = prev_found_fp.erase(prev_it);
				curr_it = current_changed_fp.erase(curr_it);
		    } 



		// Ищем матрицу изменений между предыдущим и текущим кадром
		Mat current_transf{estimateRigidTransform(prev_found_fp, current_changed_fp, false)}; 

		// В некоторых редких случаях такая матрица может быть не найдена, так что просто используем последний "бэкап" 
		if (current_transf.data == NULL) 
			last_transf.copyTo(current_transf);
		current_transf.copyTo(last_transf);

		// Выбираем нужные нам изменения из матрицы изменений
		double dx{current_transf.at<double>(0,2)};
		double dy{current_transf.at<double>(1,2)}; 
		double da{atan2(current_transf.at<double>(1,0), current_transf.at<double>(0,0))};

		// Сохраняем эти изменения
		transforms.push_back(transform_parameters(dx, dy, da));

		// Переходим к следующему кадру
		current_gray_frame.copyTo(prev_gray_frame); 
	}


	return std::move(transforms);
}





// Функция подсчёта промежуточных сумм - "траектории" для каждого кадра
std::vector<trajectory> cumulative_sum(std::vector<transform_parameters> &transforms)
{

	// "Траектория" для всех кадров
	std::vector <trajectory> traj;

	double a{0};
	double x{0};
	double y{0};

	for(size_t i{0}; i < transforms.size(); i++) 
	{
		x += transforms[i].dx;
		y += transforms[i].dy;
		a += transforms[i].da;

		traj.push_back(trajectory(x,y,a));
	}

	return std::move(traj); 
}





// Функция подсчёта скользящей средней для нашей траектории используя moving average filter
std::vector<trajectory> smooth(std::vector <trajectory>& traj, int radius)
{
	std::vector <trajectory> smoothed_trajectory; 
	for(size_t i{0}; i < traj.size(); i++) 
	{
		double sum_x{0};
		double sum_y{0};
		double sum_a{0};
		int count{0};

		for(int j{-radius}; j <= radius; j++) 
		{
			if ( (i+j >= 0) && (i+j < traj.size()) ) 
			{
				sum_x += traj[i+j].x;
				sum_y += traj[i+j].y;
				sum_a += traj[i+j].a;

				count++;
			}
		}

		double avg_a{sum_a / count};
		double avg_x{sum_x / count};
		double avg_y{sum_y / count};

		smoothed_trajectory.push_back(trajectory(avg_x, avg_y, avg_a));
	}

	return std::move(smoothed_trajectory); 
}





// Функция для построения "сглаженных" изменений для каждого кадра
std::vector<transform_parameters> get_smooth_transforms(std::vector<trajectory> smoothed_trajectory, std::vector<trajectory> original_trajectory, std::vector<transform_parameters> transforms)
{
	std::vector<transform_parameters> transforms_smooth;

	double diff_x;
	double diff_y;
	double diff_a;

	double dx;
	double dy;
	double da;

	for(size_t i{0}; i < transforms.size(); i++)
	{
		// Ищем разницу между сглаженной и изначальной траекторией
		diff_x = smoothed_trajectory[i].x - original_trajectory[i].x;
		diff_y = smoothed_trajectory[i].y - original_trajectory[i].y;
		diff_a = smoothed_trajectory[i].a - original_trajectory[i].a;

		// Создаём "сглаженные" изменения для каждого кадра
		dx = transforms[i].dx + diff_x;
		dy = transforms[i].dy + diff_y;
		da = transforms[i].da + diff_a;

		transforms_smooth.push_back(transform_parameters(dx, dy, da));
	}

	return std::move(transforms_smooth);	
}





// Функция, убирающая черные полосы, которые могут появиться при преобразовании кадра
void fixBorder(Mat &frame_stabilized)
{
	Mat T {getRotationMatrix2D(Point2f(frame_stabilized.cols/2, frame_stabilized.rows/2), 0, 1.04)}; 
	warpAffine(frame_stabilized, frame_stabilized, T, frame_stabilized.size()); 
}

 



// Функция для применения "сглаженных" изменений к текущему кадру
void get_stabilized_frame(Mat& current_frame, transform_parameters current_frame_transf_smooth)
{
	Mat transforms(2, 3, CV_64F); 

	// По "сглаженным" преобразованиям строим матрицу перехода между кадрами 
	current_frame_transf_smooth.get_transf_mat(transforms); 

	// Применяем аффинные преобразования по построенной матрице к текущему кадру
	warpAffine(current_frame, current_frame, transforms, current_frame.size());

	fixBorder(current_frame); 
}
