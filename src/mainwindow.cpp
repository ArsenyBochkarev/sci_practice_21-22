#include <QFileDialog>
#include "mainwindow.h"
#include "main.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QMessageBox>
#include <QLabel>
#include <regex>


// Фильтр форматов файлов
bool filter_formats(const std::string& s)
{
    std::regex additional_filter(".*\.(mp4|AVI|h264).*");

    return std::regex_match(s, additional_filter);
}


// Перенос картинки с cv::Mat на QLabel
void mat_to_qlabel(const Mat& img, QLabel* img_lbl)
{
    cvtColor(img, img, COLOR_BGR2RGB);

    img_lbl->setPixmap(QPixmap::fromImage(QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888)));
}



MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->verticalLayout->setGeometry(QRect(0, 0, ui->verticalLayout->geometry().width(), ui->verticalLayout->geometry().height()));


    int x{ui->pushButton->geometry().right()*2 + 10};
    int y{ui->pushButton->geometry().top()};

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width{abs(screenGeometry.width())};
    int height{abs(screenGeometry.height())};

    ui->image_label->setGeometry(QRect(x, y, width, height));


    change_buttons_visibility(0);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::change_buttons_visibility(bool v)
{
    // Изменяем видимость кнопки Pause/Run
    ui->pushButton_10->setVisible(v);

    // Изменяем видимость поля с вводом номера кадра
    ui->curr_frame_spinBox->setVisible(v);

    // Изменяем видимость кнопок перехода между кадрами
    ui->next_frame_button->setVisible(v);
    ui->prev_frame_button->setVisible(v);

    // Изменяем видимость полей управления горизонтом
    ui->left_hor_spinBox->setVisible(v);
    ui->right_hor_spinBox->setVisible(v);

    // Изменяем видимость кнопки Save
    ui->save_button->setVisible(v);

    // Изменяем видимость поля для ввода задержки между кадрами
    ui->delay_spinBox->setVisible(v);

    // Изменяем видимость поля для ввода change_rate
    ui->frame_rate_spinBox->setVisible(v);

}


// Отрисовка горизонта и вывод текущего кадра
void MainWindow::show_frame()
{
    double horizon_y1{all_horizon_coords[frame_num].first};
    double horizon_y2{all_horizon_coords[frame_num].second};

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    double horizon_x1{0};
    double horizon_x2{static_cast<double>(abs(screenGeometry.width()))};


    // Рисуем горизонт
    Mat painted_frame;
    all_frames_vec[frame_num].copyTo(painted_frame);
    line(painted_frame, Point2f(horizon_x1, horizon_y1), Point2f(horizon_x2, horizon_y2), Scalar(0,0,255), 3, 8 );


    // Выставляем значения горизонта в QSpinbox'ах
    ui->left_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].first));
    ui->right_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].second));


    mat_to_qlabel(painted_frame, ui->image_label);
}


// Отслеживание горизонта на кадре
std::pair<double, double> MainWindow::detect_on_frame()
{
    /*
    ???
    double horizon_y1{all_horizon_coords[frame_num].first};
    double horizon_y2{all_horizon_coords[frame_num].second};
    */

    double horizon_y1{all_horizon_coords[frame_num-1].first};
    double horizon_y2{all_horizon_coords[frame_num-1].second};

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    double horizon_x1{0};
    double horizon_x2{static_cast<double>(abs(screenGeometry.width()))};


    Mat prev_frame, prev_gray_frame;
    Mat current_frame, current_gray_frame;


    all_frames_vec[frame_num-1].copyTo(prev_frame);
    get_stabilized_frame(prev_frame, smooth_transforms[frame_num-1]);
    cvtColor(prev_frame, prev_gray_frame, COLOR_BGR2GRAY);

    all_frames_vec[frame_num].copyTo(current_frame);
    get_stabilized_frame(current_frame, smooth_transforms[frame_num]);
    cvtColor(current_frame, current_gray_frame, COLOR_BGR2GRAY);


    std::vector<Point2f> prev_found_fp{all_found_fp_vec[frame_num-1]};


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
    for (unsigned long long i{0}; i < fp_num; i++)
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


        //std::cout << "horizon_x1 == " << horizon_x1 << " horizon_y1 == " << horizon_y1 << "\n";
        //std::cout << "horizon_x2 == " << horizon_x2 << " horizon_y2 == " << horizon_y2 << "\n\n\n\n\n\n\n\n";

    }

    std::pair<double, double> res(horizon_y1, horizon_y2);

    all_horizon_coords[frame_num] = res;

    return res;


    /*
    // Вывод технических параметров
    std::cout << "frame number == " << frame_num << "\n\n";
    std::cout << "fp_num == " << fp_num << "\n";
    std::cout << "wrong_fp_num == " << wrong_fp_num << "\n";
    std::cout << "old_sum_horizon_dist == " << old_sum_horizon_dist << "\n";
    std::cout << "new_sum_horizon_dist == " << new_sum_horizon_dist << "\n";
    std::cout << "old_avg_single_p_dist == " << old_avg_single_p_dist << "\n";
    std::cout << "new_avg_single_p_dist == " << new_avg_single_p_dist << "\n";
    std::cout << "horizon_correctness == " << horizon_correctness << "\n\n";
    */

}


// Пре-проход по всем кадрам
int MainWindow::start_process()
{
     Mat prev_frame;

     std::string file_name{ui->lineEdit->text().toStdString()};

     std::cout << "file_name == " << file_name << "\n";

     VideoCapture cap(file_name);
     VideoCapture pre_cap1(file_name);
     VideoCapture pre_cap2(file_name);



     if(!cap.isOpened())
     {
         std::cout << "Error opening video stream or file \n\n\n";
         return -1;
     }


     QScreen *screen = QGuiApplication::primaryScreen();
     QRect screenGeometry = screen->geometry();
     double horizon_x1{0}, horizon_x2{static_cast<double>(abs(screenGeometry.width()))};
     double horizon_y1{0}, horizon_y2{0};



     // Пре-проход по всем кадрам для построения матриц перехода и "сглаженных" матриц перехода между кадрами
     // Нужен для стабилизации видео (shake compensation)
     // Узнаем общее количество кадров в видео
     unsigned long long all_frames_num{static_cast<unsigned long long>(pre_cap1.get(CAP_PROP_FRAME_COUNT))};

     // Строим "сглаженные" матрицы изменений между кадрами, которые в дальнейшем будем применять к каждому отдельно взятому кадру
     std::cout << "Doing shake compensation...\n";
     std::vector<transform_parameters> tmp_smooth_transforms = get_smooth_transforms_func(all_frames_num, pre_cap1);




     // Второй пре-проход по всем кадрам: сохраняем их и найденные на них фичи в соответствующие массивы.
     // Также создаём массив с соответствующими парами y-координат горизонта (x-координаты -- левая и правая границы кадра)
     // Причём неважно, что мы проходим по не стабилизированным кадрам -- стабилизация влияет лишь на работу по отслеживанию перемещений горизонта
     std::vector<Mat> tmp_all_frames_vec(all_frames_num);
     std::vector<std::vector<Point2f> > tmp_all_found_fp_vec;
     std::vector<std::pair<double, double> > tmp_all_horizon_coords(all_frames_num);

     std::cout << "Saving all frames and their feature points...\n";

     for(unsigned long long i{0}; i < all_frames_num; i++)
     {
         Mat pre_cap_frame, pre_cap_gray_frame;

         pre_cap2.read(pre_cap_frame);
         if (pre_cap_frame.empty())
         {
             std::cout << "unable to read pre_cap_frame on frame number " << i << "\n";
             return -1;
         }
         pre_cap_frame.copyTo(tmp_all_frames_vec[i]);

         // Пока что не важно, чем они инициализируются
         tmp_all_horizon_coords[i].first = horizon_y1;
         tmp_all_horizon_coords[i].second = horizon_y2;


         get_stabilized_frame(pre_cap_frame, tmp_smooth_transforms[i]);
         cvtColor(pre_cap_frame, pre_cap_gray_frame, COLOR_BGR2GRAY);
         std::vector<Point2f> tmp_fp;
         goodFeaturesToTrack(pre_cap_gray_frame, tmp_fp, 300, 0.2, 2);
         tmp_all_found_fp_vec.push_back(tmp_fp);
     }



     // Способ построения горизонта
     // В дальнейшем сделать регулируемым
     bool horizon_detection_method{0};

     // Текущий кадр
     frame_num = 1;


     // Выставляем максимумы для change_rate и frame_num
     ui->frame_rate_spinBox->setMaximum(all_frames_num);
     ui->curr_frame_spinBox->setMaximum(all_frames_num);


     // Берём первый кадр для того, чтобы пометить горизонт на нём
     tmp_all_frames_vec[frame_num-1].copyTo(prev_frame);




     // Вычисляем горизонт на первом кадре
     std::vector<std::pair<double, double> > coords{get_horizon_coordinates(prev_frame, horizon_detection_method)};
     tmp_all_horizon_coords[frame_num - 1].first = coords[0].second;
     tmp_all_horizon_coords[frame_num - 1].second = coords[1].second;


     // Регулируем размер QLabel, в котором будут отображаться кадры
     ui->image_label->setGeometry(QRect(ui->image_label->geometry().left(), ui->image_label->geometry().top(), prev_frame.cols, prev_frame.rows));



     MainWindow::all_frames_vec = tmp_all_frames_vec;
     MainWindow::smooth_transforms = tmp_smooth_transforms;
     MainWindow::all_found_fp_vec = tmp_all_found_fp_vec;
     MainWindow::all_horizon_coords = tmp_all_horizon_coords;



     //detect_and_show_cycle();
     return 1;
 }


// Цикл по отслеживанию горизонта и выводу картинки на экран
void MainWindow::detect_and_show_cycle()
{

    //frame_num = ui->curr_frame_spinBox->text().toInt();
    for (; frame_num < all_frames_vec.size(); frame_num++)
    {
        ui->curr_frame_spinBox->setValue(frame_num);
        if (process_going)
        {
            // Процесс не на паузе
            if (frame_num % change_rate == 0)
                detect_on_frame();
            else
                all_horizon_coords[frame_num] = all_horizon_coords[frame_num-1];

            show_frame();
        }
        else
            break;

        ui->left_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].first));
        ui->right_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].second));

        waitKey(delay);
    }

    // Дошли до конца видео => нужно выполнить все те же самые дейстия, как и в случае нажатия кнопки Pause
    // А также уменьшить frame_num на единицу
    if (frame_num == all_frames_vec.size())
    {
        on_pushButton_10_clicked();
        frame_num--;
    }


    ui->left_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].first));
    ui->right_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].second));
}


// Кнопка Start
void MainWindow::on_pushButton_clicked()
{
    std::string s{ui->lineEdit->text().toStdString()};

    std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";

    if ((!process_started) && (filter_formats(s)))
    {


        // Запускаем пре-проход по всем кадрам
        int res{start_process()};

        if (res < 0)
        {
            (new QErrorMessage(this))->showMessage("Error opening video stream or file!");
            return;
        }


        process_started = 1;


        change_buttons_visibility(1);



        // Выводим первый кадр на экран
        frame_num = 0;

        ui->left_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].first));
        ui->right_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].second));
        ui->curr_frame_spinBox->setValue(0);

        show_frame();
        process_going = 1;
        on_pushButton_10_clicked();


        // Более эта кнопка в работе не нужна
        ui->pushButton->setVisible(0);


        std::cout << "all_frames_vec.size() == " << all_frames_vec.size() << "\n";
    }
}

// Кнопка Browse
void MainWindow::on_pushButton_2_clicked()
{
    // Возможно, забыл какие-то форматы
    QString filter{"MP4 (*.mp4) ;; AVI (*.AVI) ;; H264 (*.h264)"};
    QString file{QFileDialog::getOpenFileName(this, "Choose a file", QDir::currentPath(), filter)};


    if (filter_formats(file.toStdString()))
        ui->lineEdit->setText(file);
    else
        ui->lineEdit->setText("Inappropriate format!");
}


// Кнопка Pause/Run
void MainWindow::on_pushButton_10_clicked()
{

    // Процесс идёт и была нажата кнопка
    if (process_going)
    {
        // Меняем название
        ui->pushButton_10->setText("Run");

        // Разрешаем изменять значения в QSpinbox'ах
        ui->delay_spinBox->setReadOnly(0);
        ui->frame_rate_spinBox->setReadOnly(0);
        ui->left_hor_spinBox->setReadOnly(0);
        ui->right_hor_spinBox->setReadOnly(0);

        ui->left_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].first));
        ui->right_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].second));

        process_going = 0;
    }
    else
    {
        // Запрещаем изменять значения в QSpinbox
        ui->delay_spinBox->setReadOnly(1);
        ui->frame_rate_spinBox->setReadOnly(1);
        ui->left_hor_spinBox->setReadOnly(1);
        ui->right_hor_spinBox->setReadOnly(1);

        // Важно! Нужно, чтобы frame_num был строго больше нуля, если хотим нажимать Run
        if (frame_num == 0)
            frame_num++;

        ui->pushButton_10->setText("Pause");
        process_going = 1;
        detect_and_show_cycle();
    }
}


// Кнопка перехода на предыдущий кадр
void MainWindow::on_prev_frame_button_clicked()
{
    if ((!process_going) && (frame_num > 0))
    {
        frame_num--;
        ui->curr_frame_spinBox->setValue(frame_num);
        show_frame();
    }
}


// Кнопка перехода на следующий кадр
void MainWindow::on_next_frame_button_clicked()
{
    if ((!process_going) && (frame_num < all_frames_vec.size() - 1))
    {
        frame_num++;
        ui->curr_frame_spinBox->setValue(frame_num);
        show_frame();
    }

}


void MainWindow::on_delay_spinBox_valueChanged(int arg1)
{
    delay = arg1;
}


void MainWindow::on_frame_rate_spinBox_valueChanged(int arg1)
{
    change_rate = arg1;
}


// Альтернатива кнопкам перехода на следующий кадр -- стрелочки в QSpinbox
void MainWindow::on_curr_frame_spinBox_valueChanged(int arg1)
{
    if ((arg1 >= 0) && (arg1 < all_frames_vec.size()))
        frame_num = arg1;

    show_frame();
}


void MainWindow::on_left_hor_spinBox_valueChanged(int arg1)
{
    if (!process_going)
    {
        all_horizon_coords[frame_num].first = arg1;
        show_frame();
    }
}




void MainWindow::on_right_hor_spinBox_valueChanged(int arg1)
{
    if (!process_going)
    {
        all_horizon_coords[frame_num].second = arg1;
        show_frame();
    }
}

