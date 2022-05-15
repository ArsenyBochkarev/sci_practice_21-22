#include <QFileDialog>
#include "mainwindow.h"
#include "main.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QMessageBox>
#include <QLabel>
#include <regex>


// Спросить пользователя, не хочет ли он сохранить свою текущую работу
bool MainWindow::save_dialog()
{
    QMessageBox msg_box;
    msg_box.setText("The document has been modified.");
    msg_box.setInformativeText("Do you want to save your changes?");
    msg_box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msg_box.setDefaultButton(QMessageBox::Save);
    auto ret{msg_box.exec()};

    bool continue_or_not{1};

    switch (ret)
    {
        case QMessageBox::Save:
            // Хотим сохранить и продолжить работу, как задумывали
            on_save_pushButton_clicked();
            continue_or_not = 1;
            break;
        case QMessageBox::Cancel:
            // Продолжать и сохранять не хотим
            continue_or_not = 0;
            break;
        default:
            // Хотим продолжать, но не сохранять
            continue_or_not = 1;
    }

    return continue_or_not;
}



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

    set_objects_size();

    change_buttons_visibility(0);
}


MainWindow::~MainWindow()
{
    delete ui;
}


// Спросить у пользователя, не хочет ли он сохранить свою текущую работу перед выходом из программы (в случае, если она ещё не сохранена)
void MainWindow::closeEvent(QCloseEvent *event)
{
    // Спрашивать про сохранение только в случае, если пре-процесс уже прошёл (иначе нет смысла)
    if ((!saved) && (process_started))
        if (!save_dialog())
        {
            event->ignore();
            return;
        }

    event->accept();
    QMainWindow::closeEvent(event);
}

// Задаём размеры объектов в зависимости от размера экрана
void MainWindow::set_objects_size()
{
    // Хотим, чтобы "панель" с кнопками занимала в длину примерно 1/8 экрана
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    int screen_width{abs(screenGeometry.width())};
    int screen_height{abs(screenGeometry.height())};

    int object_width{screen_width/8};

    // Выставляем ограничения на кнопки
    ui->verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
    ui->verticalLayout->setGeometry(QRect(10, 10, object_width, ui->verticalLayout->geometry().height()));


    // Координаты для image_label
    int right_x{ui->pushButton->geometry().right()*2 + 10};
    int top_y{ui->pushButton->geometry().top()};
    ui->image_label->setGeometry(QRect(right_x, top_y, screen_width, screen_height));
}



// Изменение видимости объектов
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
    ui->save_pushButton->setVisible(v);

    // Изменяем видимость кнопки Import from file
    ui->import_hor_pushButton->setVisible(v);

    // Изменяем видимость кнопки Detect horizon
    ui->detect_horizon_pushButton->setVisible(v);

    // Изменяем видимость поля для ввода задержки между кадрами
    ui->delay_spinBox->setVisible(v);

    // Изменяем видимость поля для ввода change_rate
    ui->frame_rate_spinBox->setVisible(v);


    // ------------------------------------------------------------------------------


    // Изменяем видимость поля ввода имени файла (на противоположную тому, что выше!)
    ui->lineEdit->setVisible(!v);

    // Изменяем видимость кнопки Start (на противоположную тому, что выше!)
    ui->pushButton->setVisible(!v);

    // Изменяем видимость кнопки Browse (на противоположную тому, что выше!)
    ui->pushButton_2->setVisible(!v);

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

    // Выставляем текущий кадр в соответствующем QSpinbox'е
    ui->curr_frame_spinBox->setValue(frame_num);


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

     current_file = ui->lineEdit->text().toStdString();

     std::cout << "file_name == " << current_file << "\n";

     VideoCapture pre_cap1(current_file);
     VideoCapture pre_cap2(current_file);
     VideoCapture pre_cap3(current_file);


     if(!pre_cap1.isOpened())
     {
         std::cout << "Error opening video stream or file \n\n\n";
         return -1;
     }


     QScreen *screen = QGuiApplication::primaryScreen();
     QRect screenGeometry = screen->geometry();
     double horizon_x1{0}, horizon_x2{static_cast<double>(abs(screenGeometry.width()))};
     double horizon_y1{0}, horizon_y2{0};



     // Пре-проход по всем кадрам для сохранения всех кадров и поиска и сохранения "фич" на них
     // Узнаем общее количество кадров в видео
     unsigned long long all_frames_num{static_cast<unsigned long long>(pre_cap1.get(CAP_PROP_FRAME_COUNT))};

     // Cоздаём массив с соответствующими парами y-координат горизонта (x-координаты -- левая и правая границы кадра)
     //std::vector<Mat> tmp_all_frames_vec(all_frames_num);
     //std::vector<std::vector<Point2f> > tmp_all_found_fp_vec;
     //std::vector<std::pair<double, double> > tmp_all_horizon_coords(all_frames_num);

     all_frames_vec = std::vector<Mat>(all_frames_num);
     all_horizon_coords = std::vector<std::pair<double, double> > (all_frames_num);


     std::cout << "Saving all frames and their feature points...\n";

     for(unsigned long long i{0}; i < all_frames_num; i++)
     {
         Mat pre_cap_frame, pre_cap_gray_frame;

         pre_cap1.read(pre_cap_frame);
         if (pre_cap_frame.empty())
         {
             std::cout << "unable to read pre_cap_frame on frame number " << i << "\n";
             return -1;
         }
         pre_cap_frame.copyTo(all_frames_vec[i]);

         // Инициализируем все координаты как -100, чтобы потом понять, до какого кадра добрались, а до какого нет
         all_horizon_coords[i].first = -100;
         all_horizon_coords[i].second = -100;


         cvtColor(pre_cap_frame, pre_cap_gray_frame, COLOR_BGR2GRAY);
         std::vector<Point2f> tmp_fp;
         goodFeaturesToTrack(pre_cap_gray_frame, tmp_fp, 300, 0.2, 2);
         all_found_fp_vec.push_back(tmp_fp);
     }


     // Строим "сглаженные" матрицы изменений между кадрами, которые в дальнейшем будем применять к каждому отдельно взятому кадру
     std::cout << "Doing shake compensation...\n";
     smooth_transforms = get_smooth_transforms_func(all_frames_num, pre_cap2, all_found_fp_vec);

     // Второй пре-проход по всем кадрам: применяем shake compensation
     for (unsigned long long i{0}; i < all_frames_num; i++)
     {

         Mat pre_cap_frame;

         pre_cap3.read(pre_cap_frame);
         if (pre_cap_frame.empty())
         {
             std::cout << "unable to read pre_cap_frame on frame number " << i << "\n";
             return -1;
         }

         get_stabilized_frame(pre_cap_frame, smooth_transforms[i]);
     }





     // Текущий кадр
     frame_num = 0;


     // Выставляем максимумы для change_rate и frame_num
     ui->frame_rate_spinBox->setMaximum(all_frames_num);
     ui->curr_frame_spinBox->setMaximum(all_frames_num);


     // Берём первый кадр для того, чтобы пометить горизонт на нём
     all_frames_vec[frame_num].copyTo(prev_frame);


     // Вычисляем горизонт на первом кадре
     std::vector<std::pair<double, double> > coords{get_horizon_coordinates(prev_frame, horizon_detection_method)};

     // В случае, если горизонт отличается от записанного ранее -- файл нужно сохранить заново
     if ((coords[0].second != all_horizon_coords[frame_num].first) || (coords[1].second != all_horizon_coords[frame_num].second))
         saved = 0;

     all_horizon_coords[frame_num].first = coords[0].second;
     all_horizon_coords[frame_num].second = coords[1].second;


     // Регулируем размер QLabel, в котором будут отображаться кадры
     ui->image_label->setGeometry(QRect(ui->image_label->geometry().left(), ui->image_label->geometry().top(), prev_frame.cols, prev_frame.rows));


     return 1;
 }


// Цикл по отслеживанию горизонта и выводу картинки на экран
void MainWindow::detect_and_show_cycle()
{

    //frame_num = ui->curr_frame_spinBox->text().toInt();
    // for (; frame_num < all_frames_vec.size(); frame_num++)
    while(frame_num < all_frames_vec.size()-1)
    {
        frame_num++;

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
    if (frame_num == all_frames_vec.size()-1)
        on_pushButton_10_clicked();


    ui->left_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].first));
    ui->right_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].second));
}



// ----------------------------------------------------------------
// QPushButton

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



        std::cout << "all_frames_vec.size() == " << all_frames_vec.size() << "\n";
    }
}

// Кнопка Browse
void MainWindow::on_pushButton_2_clicked()
{
    QString filter{"MP4 (*.mp4) ;; AVI (*.AVI) ;; H264 (*.h264) ;; All Files (*)"};
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


        // Чтобы выводить уже корректно отрисованный кадр (не работает, когда прошли все кадры!)
        if ((frame_num > 0) && (frame_num != all_frames_vec.size()-1))
            frame_num--;
        process_going = 0;
    }
    else
    {
        // Запрещаем изменять значения в QSpinbox
        ui->delay_spinBox->setReadOnly(1);
        ui->frame_rate_spinBox->setReadOnly(1);
        ui->left_hor_spinBox->setReadOnly(1);
        ui->right_hor_spinBox->setReadOnly(1);

        // Меняем текст и запускаем цикл
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
        show_frame();
    }
}


// Кнопка перехода на следующий кадр
void MainWindow::on_next_frame_button_clicked()
{
    if ((!process_going) && (frame_num < all_frames_vec.size() - 1))
    {
        frame_num++;
        show_frame();
    }

}


// Кнопка определения горизонта программно
void MainWindow::on_detect_horizon_pushButton_clicked()
{
    std::vector<std::pair<double, double> > coords{get_horizon_coordinates(all_frames_vec[frame_num], horizon_detection_method)};
    all_horizon_coords[frame_num].first = coords[0].second;
    all_horizon_coords[frame_num].second = coords[1].second;
    show_frame();
}


// ----------------------------------------------------------------
// QSpinBox

// Изменение значения задержки между кадрами
void MainWindow::on_delay_spinBox_valueChanged(int arg1)
{
    delay = arg1;
}

// Изменение значения частоты проверки горизонта на кадре
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


// Изменение положение левой точки горизонта
void MainWindow::on_left_hor_spinBox_valueChanged(int arg1)
{
    if (!process_going)
    {
        if (all_horizon_coords[frame_num].first != arg1)
            saved = 0;
        all_horizon_coords[frame_num].first = arg1;
        show_frame();
    }
}

// Изменение положения правой точки горизонта
void MainWindow::on_right_hor_spinBox_valueChanged(int arg1)
{
    if (!process_going)
    {
        if (all_horizon_coords[frame_num].second != arg1)
            saved = 0;
        all_horizon_coords[frame_num].second = arg1;
        show_frame();
    }
}



void MainWindow::on_save_pushButton_clicked()
{
    // Вызываем диалог
    QString filter{"txt (*.txt) ;; All Files (*)"};
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save file"), /*QDir::currentPath()*/ "*.txt", filter);

    if (file_name.isEmpty())
        return;
    else
    {
        QFile file(file_name);

        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
        }

        QTextStream out(&file);

        //out.setVersion(QDataStream::Qt_5_9);

        for (int i{0}; i < all_horizon_coords.size(); i++)
            out << QString::number(all_horizon_coords[i].first) << " " << QString::number(all_horizon_coords[i].second) << Qt::endl;

        file.close();
        saved = 1;
    }
}


void MainWindow::on_import_hor_pushButton_clicked()
{
    // Сначала предлагаем сохранить текущий файл
    bool continue_or_not{1};
    if (!saved)
        continue_or_not = save_dialog();

    if (continue_or_not)
    {
        // Вызываем диалог
        QString filter{"txt (*.txt) ;; All Files (*)"};
        QString file_name = QFileDialog::getOpenFileName(this, tr("Import from file"), QDir::currentPath(), filter);

        if (file_name.isEmpty())
            return;
        else
        {

            QFile file(file_name);

            if (!file.open(QIODevice::ReadOnly))
            {
                QMessageBox::information(this, tr("Unable to open file"), file.errorString());
                return;
            }

            // Дополнительно проверяем формат файла
            // Фильтр формата
            std::regex additional_filter(".*\.txt.*");

            if (!std::regex_match(file_name.toStdString(), additional_filter))
            {
                QMessageBox::information(this, file.errorString(), tr("Inappropriate file format"));
                return;
            }


            QTextStream in(&file);

            //in.setVersion(QDataStream::Qt_4_5);

            std::vector<std::pair<double, double> > tmp_all_horizon_coords;

            while(!in.atEnd())
            {
                double tmp1{-100}, tmp2{-100};
                in >> tmp1 >> tmp2;
                tmp_all_horizon_coords.push_back(std::pair(tmp1, tmp2));
            }

            // Всегда получается на один больше чем нужно, поэтому удаляем последний добавленный элемент
            tmp_all_horizon_coords.erase(tmp_all_horizon_coords.end());

            std::cout << "\n\n\ntmp_all_horizon_coords.size() == " << tmp_all_horizon_coords.size() << " all_horizon_coords.size() == " << all_horizon_coords.size() << "\n\n\n";

            for (int i{0}; i < tmp_all_horizon_coords.size(); i++)
                std::cout << tmp_all_horizon_coords[i].first << " " << tmp_all_horizon_coords[i].second << "\n";

            if (tmp_all_horizon_coords.size() != all_horizon_coords.size())
            {
                QMessageBox::information(this, file.errorString(), tr("The number of frames in file is not equal to the number of frames in current video"));
                return;
            }

            all_horizon_coords = tmp_all_horizon_coords;
            for (frame_num = 0; frame_num < tmp_all_horizon_coords.size(); frame_num++)
            {
                // Выходим на первом кадре, на котором горизонт не определён
                if ((tmp_all_horizon_coords[frame_num].first == -100) || (tmp_all_horizon_coords[frame_num].first == -100))
                    break;
            }

            if (frame_num == tmp_all_horizon_coords.size())
                frame_num--;

            file.close();

            show_frame();
        }
    }
}

