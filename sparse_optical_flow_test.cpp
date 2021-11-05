#include <iostream> 
#include <vector> 
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

    // Считываем первый кадр, затем меняем цвет
    cap.read(prev_frame);
    cvtColor(prev_frame, prev_gray_frame, COLOR_BGR2GRAY); 


    // Векторы, хранящие в себе "фичи" - feature points
    std::vector<Point2f> prev_found_fp;
    std::vector<Point2f> current_found_fp;
    std::vector<Point2f> current_changed_fp;

    // Ищем "фичи" на первом кадре
    goodFeaturesToTrack(prev_gray_frame, prev_found_fp, 300, 0.2, 2);

    // Счётчик кадров
    unsigned long long frame_num{0};

    while(cap.isOpened())
    {
        frame_num++;

        cap.read(current_frame);
        if (current_frame.empty())
            break;


        // Меняем цвет, затем считаем optical flow
        // Насколько я понимаю, current_changed_fp имеет тот же размер, что и prev_found_fp
        // Но если нет - посмотреть в документации, там, кажется, был подходящий флаг
        cvtColor(current_frame, current_gray_frame, COLOR_BGR2GRAY);
        calcOpticalFlowPyrLK(prev_gray_frame, current_gray_frame, prev_found_fp, current_changed_fp, status, err); 


        // Ещё раз ищем "фичи", только уже на новом кадре
        goodFeaturesToTrack(current_gray_frame, current_found_fp, 300, 0.2, 2);


        // Сравниваем старые и новые "фичи"
        // Если положение фич и/или их число лишь немного изменилось, то менять горизонт смысла нет, иначе - пересчитываем
        // some code

        std::cout << "frame number " << frame_num << "\n";
        std::cout << "prev found fp num " << prev_found_fp.size() << "\n";
        std::cout << "current changed fp num " << current_changed_fp.size() << "\n";
        std::cout << "current found fp num " << current_found_fp.size() << "\n";


        // Показываем картинку
        imshow("video_in.AVI", current_frame); 

        // Новые "фичи" - теперь старые 
        prev_found_fp.resize(current_found_fp.size());
        for (long long i{0}; i < current_found_fp.size(); i++)
            prev_found_fp[i] = current_found_fp[i]; 

        // Если пользователь нажмет клавишу "Escape" - цикл прервётся и программа завершится 
        char c{(char)waitKey(25)};
        if (c == 27)
            break; 

    }


    cap.release();
    destroyAllWindows();


}
