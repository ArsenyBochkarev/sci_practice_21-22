#include <QFileDialog>
#include "mainwindow.h"
#include "main.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QMessageBox>
#include <QLabel>
#include <regex>


/// Сохраняем все кадры видео в отдельную папку с названием таким же, как у текущего видео
/// Папка будет располагаться в build директории и удалится в момент закрытия программы
void MainWindow::save_all_frames(unsigned long long all_frames_num, QProgressDialog& progress)
{
    VideoCapture cap(current_file);
    Mat frame;

    if(!cap.isOpened())
    {
        (new QErrorMessage(this))->showMessage("Error opening video stream or file!");
        return;
    }


    QFileInfo fileInfo(QString::fromStdString(current_file));
    QString filename{fileInfo.baseName()};

    if ((QDir(filename).exists()) && (QDir(filename).count() == all_frames_num))
        return;

    if (!QDir(filename).exists())
            QDir().mkdir(filename);

    QString text{"Saving all frames..."};

    for(unsigned long long i{0}; i < all_frames_num; i++)
    {
        progress.setValue(progress.value() + 1);
        progress.setLabelText(text + "\nCurrently on frame number " + QString::number(i));

        cap.read(frame);

        if (frame.empty())
        {
            (new QErrorMessage(this))->showMessage("Error opening video stream or file!");
            return;
        }

        imwrite((filename + "/" + QString::number(i) + ".jpg").toStdString(), frame);

        frame.release();
    }



    cap.release();
}


/// Спросить пользователя, не хочет ли он сохранить свою текущую работу
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


/// Фильтр форматов файлов
bool filter_formats(const std::string& s)
{
    std::regex additional_filter(".*\.(mp4|AVI|h264).*");

    return std::regex_match(s, additional_filter);
}


/// Перенос картинки с cv::Mat на QLabel
void mat_to_qlabel(const Mat& img, QLabel* img_lbl)
{
    cvtColor(img, img, COLOR_BGR2RGB);

    img_lbl->setPixmap(QPixmap::fromImage(QImage(img.data, img.cols, img.rows, img.step, QImage::Format_RGB888)));
}

/// Конструктор
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), process_started(false), process_going(false), delay(10), saved(0), frame_num(0), change_rate(3),
    horizon_detection_method(0), max_sum_horizon_diff(1200), max_single_point_horizon_diff(400), max_wrong_fp_num(5), max_horizon_correctness_diff(20), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    set_objects_size();

    change_buttons_visibility(0);


    // Выставляем дефолтное значение для поля с названием метода поиска горизонта
    ui->current_hor_detect_method_label->setText("Current horizon detection method:\nCanny/Hough");


    all_found_fp_vec.clear();
    all_horizon_coords.clear();
    smooth_transforms.clear();
}

/// Деструктор
MainWindow::~MainWindow()
{
    QFileInfo fileInfo(QString::fromStdString(current_file));
    QString filename{fileInfo.baseName()};

    std::cout << "QDir(filename).count() == " << QDir(filename).count() << " all_found_fp_vec.size() == " << all_found_fp_vec.size() << "\n";

    if (QDir(filename).exists())
        QDir(filename).removeRecursively();

    delete ui;
}


/// Спросить у пользователя, не хочет ли он сохранить свою текущую работу перед выходом из программы (в случае, если она ещё не сохранена)
void MainWindow::closeEvent(QCloseEvent *event)
{
    /// Спрашивать про сохранение только в случае, если пре-процесс уже прошёл (иначе нет смысла)
    if ((!saved) && (process_started))
        if (!save_dialog())
        {
            event->ignore();
            return;
        }

    event->accept();
    QMainWindow::closeEvent(event);
}

/// Задаём размеры объектов в зависимости от размера экрана
void MainWindow::set_objects_size()
{
    /// Хотим, чтобы "панель" с кнопками занимала в длину примерно 1/8 экрана
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



/// Изменение видимости объектов
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

    // Изменяем видимость кнопки для выбора другого файла
    ui->change_file_pushButton->setVisible(v);

    // Изменяем видимость поля с названием текущего метода определения горизонта
    ui->current_hor_detect_method_label->setVisible(v);

    // Изменяем видимость кнопки переключения текущего метода определения горизонта
    ui->hor_detection_method_pushButton->setVisible(v);


    // ------------------------------------------------------------------------------


    // Изменяем видимость поля ввода имени файла (на противоположную тому, что выше!)
    ui->lineEdit->setVisible(!v);

    // Изменяем видимость кнопки Start (на противоположную тому, что выше!)
    ui->pushButton->setVisible(!v);

    // Изменяем видимость кнопки Browse (на противоположную тому, что выше!)
    ui->pushButton_2->setVisible(!v);

}


/// Отрисовка горизонта и вывод текущего кадра
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

    // Берём папку, в которой лежит текущий кадр
    QFileInfo fileInfo(QString::fromStdString(current_file));
    QString filename{fileInfo.baseName()};

    painted_frame = imread(filename.toStdString() + "/" + std::to_string(frame_num) + ".jpg");
    if (frame_num % change_rate == 0)
        line(painted_frame, Point2f(horizon_x1, horizon_y1), Point2f(horizon_x2, horizon_y2), Scalar(0,255,255), 3, 8 );
    else
        line(painted_frame, Point2f(horizon_x1, horizon_y1), Point2f(horizon_x2, horizon_y2), Scalar(0,0,255), 3, 8 );


    // Выставляем значения горизонта в QSpinbox'ах
    ui->left_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].first));
    ui->right_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].second));

    // Выставляем текущий кадр в соответствующем QSpinbox'е
    ui->curr_frame_spinBox->setValue(frame_num);


    mat_to_qlabel(painted_frame, ui->image_label);
}


/// Отслеживание горизонта на кадре
std::pair<double, double> MainWindow::detect_on_frame()
{
    double horizon_y1{all_horizon_coords[frame_num-1].first};
    double horizon_y2{all_horizon_coords[frame_num-1].second};

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    double horizon_x1{0};
    double horizon_x2{static_cast<double>(abs(screenGeometry.width()))};


    Mat prev_frame, prev_gray_frame;
    Mat current_frame, current_gray_frame;


    QFileInfo fileInfo(QString::fromStdString(current_file));
    QString filename{fileInfo.baseName()};

    prev_frame = imread(filename.toStdString() + "/" + std::to_string(frame_num - 1) + ".jpg");
    get_stabilized_frame(prev_frame, smooth_transforms[frame_num-1]);
    cvtColor(prev_frame, prev_gray_frame, COLOR_BGR2GRAY);

    current_frame = imread(filename.toStdString() + "/" + std::to_string(frame_num) + ".jpg");
    get_stabilized_frame(current_frame, smooth_transforms[frame_num]);
    cvtColor(current_frame, current_gray_frame, COLOR_BGR2GRAY);


    std::vector<Point2f> prev_found_fp{all_found_fp_vec[frame_num-1]};


    std::vector <unsigned char> status;
    std::vector <float> err;
    std::vector<Point2f> current_changed_fp;


    // Считаем optical flow
    calcOpticalFlowPyrLK(prev_gray_frame, current_gray_frame, prev_found_fp, current_changed_fp, status, err);

    unsigned long long fp_num{prev_found_fp.size()};



    /// Сравним положение "фич" на двух соседних кадрах.
    /// Если положение "фич" лишь немного изменилось, то пересчитывать горизонт смысла нет, и нужно лишь немного скорректировать его положение.
    /// Иначе - полностью перестраиваем его
    bool rebuild_horizon{0};


    /// Также сравним суммарные расстояния до горизонта, отмеченного на предыдущем кадре.
    /// Это делается, чтобы понять, нужно ли пересчитывать горизонт полностью, либо достаточно лишь немного подкорректировать его относительно нового положения "фич"

    // Считаем коэффициенты в уравнении прямой горизонта
    double A{(horizon_x2 - horizon_x1 != 0) ? 1/(horizon_x2 - horizon_x1) : 1/(horizon_x2 + 1 - horizon_x1)};
    double B{(horizon_y2 - horizon_y1 != 0) ? 1/(horizon_y2 - horizon_y1) : 1/(horizon_y2 + 1 - horizon_y1)};
    double C{B - A};



    // Суммированное, для каждой конкретной точки и среднее расстояния до горизонта на старом и новом кадрах
    double old_sum_horizon_dist{0};
    double old_single_point_dist;
    double old_avg_single_p_dist{0};

    double new_sum_horizon_dist{0};
    double new_single_point_dist;
    double new_avg_single_p_dist{0};


    /// Будем запоминать количество точек, у которых расстояние до горизонта сильно разнится по сравнению с предыдущим кадром
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

        /// В случае, если хотя бы одна точка "ускакала" куда-то вверх или вниз - запомним её и в случае, если
        /// подобных точек не будет много, просто не будем на них ориентироваться
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




    /// Если разница в расстояниях до горизонта велика (что равносильно большому количеству "плохих фич") -
    /// это основание для полной перестройки горизонта
    if ( (abs(new_sum_horizon_dist - old_sum_horizon_dist) > max_sum_horizon_diff) || (wrong_fp_num > max_wrong_fp_num) )
        rebuild_horizon = 1;
    else
        /// Если можем определить конкретное направление корректировки - делаем ее относительно средней разницы расстояний
        if ( (abs(horizon_correctness) > fp_num - max_horizon_correctness_diff) && (!rebuild_horizon) )
        {
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
        std::vector<std::pair<double, double> > coords{get_horizon_coordinates(current_frame, horizon_detection_method)};
        horizon_x1 = coords[0].first;
        horizon_y1 = coords[0].second;

        horizon_x2 = coords[1].first;
        horizon_y2 = coords[1].second;
    }

    std::pair<double, double> res(horizon_y1, horizon_y2);

    all_horizon_coords[frame_num] = res;

    return res;
}


/// Пре-проход по всем кадрам
int MainWindow::start_process()
{

     Mat prev_frame;

     current_file = ui->lineEdit->text().toStdString();

     VideoCapture pre_cap1(current_file);
     VideoCapture pre_cap2(current_file);


     QScreen *screen = QGuiApplication::primaryScreen();
     QRect screenGeometry = screen->geometry();
     double horizon_x1{0}, horizon_x2{static_cast<double>(abs(screenGeometry.width()))};
     double horizon_y1{static_cast<double>(abs(screenGeometry.height()))/2}, horizon_y2{static_cast<double>(abs(screenGeometry.height()))/2};


     // Пре-проходы по всем кадрам для сохранения всех кадров и поиска и сохранения "фич" на них
     // Узнаем общее количество кадров в видео
     unsigned long long all_frames_num{static_cast<unsigned long long>(pre_cap1.get(CAP_PROP_FRAME_COUNT))};


     QProgressDialog progress("Saving all frames...", "Abort", 0, 2*all_frames_num + 2, this);
     progress.setWindowModality(Qt::WindowModal);

     save_all_frames(all_frames_num, progress);

     QString text{"Saving all feature points..."};
     Mat pre_cap_frame, pre_cap_gray_frame;

     for(unsigned long long i{0}; i < all_frames_num; i++)
     {
         progress.setValue(progress.value()+1);
         progress.setLabelText(text + "\nCurrently on frame number " + QString::number(i));

         if (progress.wasCanceled())
             return -1;

         pre_cap1.read(pre_cap_frame);
         if (pre_cap_frame.empty())
         {
             (new QErrorMessage(this))->showMessage("Error opening video stream or file!");
             return -1;
         }

         /// Инициализируем все координаты как -100, чтобы потом понять, до какого кадра добрались, а до какого нет
         all_horizon_coords.push_back(std::pair<double, double>(-100, -100));

         std::vector<Point2f> tmp_found_fp;
         cvtColor(pre_cap_frame, pre_cap_gray_frame, COLOR_BGR2GRAY);
         goodFeaturesToTrack(pre_cap_gray_frame, tmp_found_fp, 300, 0.2, 2);
         all_found_fp_vec.push_back(tmp_found_fp);

         pre_cap_frame.release();
         pre_cap_gray_frame.release();
     }

     progress.setLabelText("Doing shake compensation...");
     waitKey(0);

     // Строим "сглаженные" матрицы изменений между кадрами, которые в дальнейшем будем применять к каждому отдельно взятому кадру
     smooth_transforms = get_smooth_transforms_func(all_frames_num, pre_cap2, all_found_fp_vec );

     if (smooth_transforms.size() != all_frames_num-1)
     {
         progress.setValue(2*all_frames_num + 5);
         (new QErrorMessage(this))->showMessage("Unable to do shake compensation");
         return -1;
     }

     progress.setValue(progress.value() + 1);



     // Текущий кадр
     frame_num = 0;


     // Выставляем максимумы для change_rate и frame_num
     ui->frame_rate_spinBox->setMaximum(all_frames_num);
     ui->curr_frame_spinBox->setMaximum(all_frames_num);


     QFileInfo fileInfo(QString::fromStdString(current_file));
     QString filename{fileInfo.baseName()};

     // Берём первый кадр для того, чтобы пометить горизонт на нём
     prev_frame = imread(filename.toStdString() + "/0.jpg", IMREAD_COLOR);


     // Вычисляем горизонт на первом кадре
     std::vector<std::pair<double, double> > coords{get_horizon_coordinates(prev_frame, horizon_detection_method)};

     if ((coords[0].second < 0 ) && (coords[1].second < 0))
         coords[0].second = coords[1].second = horizon_y1;

     // В случае, если горизонт отличается от записанного ранее -- файл нужно сохранить заново
     if ((coords[0].second != all_horizon_coords[frame_num].first) || (coords[1].second != all_horizon_coords[frame_num].second))
         saved = 0;

     all_horizon_coords[frame_num].first = coords[0].second;
     all_horizon_coords[frame_num].second = coords[1].second;


     // Регулируем размер QLabel, в котором будут отображаться кадры
     ui->image_label->setGeometry(QRect(ui->image_label->geometry().left(), ui->image_label->geometry().top(), prev_frame.cols, prev_frame.rows));

     return 1;
 }


/// Цикл по отслеживанию горизонта и выводу картинки на экран
void MainWindow::detect_and_show_cycle()
{
    while(frame_num < all_found_fp_vec.size()-1)
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
    if (frame_num == all_found_fp_vec.size()-1)
        on_pushButton_10_clicked();


    ui->left_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].first));
    ui->right_hor_spinBox->setValue(static_cast<int>(all_horizon_coords[frame_num].second));
}



// ---------------------------------------------------------------------------------------------------------------------
// QPushButton

/// Кнопка Start
void MainWindow::on_pushButton_clicked()
{
    std::string s{ui->lineEdit->text().toStdString()};

    current_file = s;

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
    }
}

/// Кнопка Browse
void MainWindow::on_pushButton_2_clicked()
{
    QString filter{"MP4 (*.mp4) ;; AVI (*.AVI) ;; H264 (*.h264) ;; All Files (*)"};
    QString file{QFileDialog::getOpenFileName(this, "Choose a file", QDir::currentPath(), filter)};


    if (filter_formats(file.toStdString()))
        ui->lineEdit->setText(file);
    else
        ui->lineEdit->setText("Inappropriate format!");
}


/// Кнопка Pause/Run
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
        if ((frame_num > 0) && (frame_num != all_found_fp_vec.size()-1))
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


/// Кнопка перехода на предыдущий кадр
void MainWindow::on_prev_frame_button_clicked()
{
    if ((!process_going) && (frame_num > 0))
    {
        frame_num--;
        show_frame();
    }
}


/// Кнопка перехода на следующий кадр
void MainWindow::on_next_frame_button_clicked()
{
    if ((!process_going) && (frame_num < all_found_fp_vec.size() - 1))
    {
        frame_num++;
        if (frame_num % change_rate == 0)
            detect_on_frame();
        show_frame();
    }

}


/// Кнопка определения горизонта программно
void MainWindow::on_detect_horizon_pushButton_clicked()
{
    Mat frame;

    QFileInfo fileInfo(QString::fromStdString(current_file));
    QString filename{fileInfo.baseName()};

    frame = imread(filename.toStdString() + "/" + std::to_string(frame_num) + ".jpg");

    std::vector<std::pair<double, double> > coords{get_horizon_coordinates(frame, horizon_detection_method)};
    all_horizon_coords[frame_num].first = coords[0].second;
    all_horizon_coords[frame_num].second = coords[1].second;
    show_frame();
}



/// Кнопка Save
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

     for (int i{0}; i < all_horizon_coords.size(); i++)
         out << QString::number(all_horizon_coords[i].first) << " " << QString::number(all_horizon_coords[i].second) << Qt::endl;

     file.close();
     saved = 1;
 }
}


/// Кнопка Import from file
void MainWindow::on_import_hor_pushButton_clicked()
{
    /// Сначала предлагаем сохранить текущий файл
    bool continue_or_not{1};
    if (!saved)
        continue_or_not = save_dialog();

    if (continue_or_not)
    {
        /// Потом вызываем диалог выбора нового файла
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

            std::vector<std::pair<double, double> > tmp_all_horizon_coords;

            while(!in.atEnd())
            {
                double tmp1{-100}, tmp2{-100};
                in >> tmp1 >> tmp2;
                tmp_all_horizon_coords.push_back(std::pair(tmp1, tmp2));
            }

            // Всегда получается на один больше чем нужно, поэтому удаляем последний добавленный элемент
            tmp_all_horizon_coords.erase(tmp_all_horizon_coords.end());

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


/// Кнопка смены файла, с которым работаем
void MainWindow::on_change_file_pushButton_clicked()
{

    QMessageBox msg_box;
    msg_box.setText("Warning");
    msg_box.setInformativeText("Your current work will be closed. Are you sure you want to continue?");
    msg_box.setStandardButtons(QMessageBox::Cancel | QMessageBox::Open);
    msg_box.setDefaultButton(QMessageBox::Cancel);
    auto ret{msg_box.exec()};

    bool continue_or_not{1};

    switch (ret)
    {
        case QMessageBox::Cancel:
            continue_or_not = 0;
            break;
        default:
            continue_or_not = 1;
    }

    if ((continue_or_not) && (!saved) && (save_dialog()))
    {
        // Выбираем новый файл
        on_pushButton_2_clicked();

        // Меняем видимость объектов
        change_buttons_visibility(0);

        // Удаляем папку с кадрами (если она есть)
        QFileInfo fileInfo(QString::fromStdString(current_file));
        QString filename{fileInfo.baseName()};

        if (QDir(filename).exists())
            QDir(filename).removeRecursively();

        // Приводим this к изначальному состоянию
        all_found_fp_vec.clear();
        all_horizon_coords.clear();
        smooth_transforms.clear();

        process_started = false;
        process_going = false;
        delay = 10;
        saved = 0;
        frame_num = 0;
        change_rate = 3;
        horizon_detection_method = 0;
        max_sum_horizon_diff = 1200;
        max_single_point_horizon_diff = 400;
        max_wrong_fp_num = 5;
        max_horizon_correctness_diff = 20;


        // Очищаем image_label
        ui->image_label->clear();
    }
}



// ---------------------------------------------------------------------------------------------------------------------
// QSpinBox

/// Изменение значения задержки между кадрами
void MainWindow::on_delay_spinBox_valueChanged(int arg1)
{
    delay = arg1;
}

/// Изменение значения частоты проверки горизонта на кадре
void MainWindow::on_frame_rate_spinBox_valueChanged(int arg1)
{
    change_rate = arg1;
}


/// Альтернатива кнопкам перехода на следующий кадр -- стрелочки в QSpinbox
void MainWindow::on_curr_frame_spinBox_valueChanged(int arg1)
{
    if ((arg1 >= 0) && (arg1 < all_found_fp_vec.size()))
        frame_num = arg1;

    show_frame();
}


/// Изменение положение левой точки горизонта
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

/// Изменение положения правой точки горизонта
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



void MainWindow::on_hor_detection_method_pushButton_clicked()
{
    horizon_detection_method = (horizon_detection_method+1)%2;

    QString s{"Current horizon detection method:\n"};
    ui->current_hor_detect_method_label->setText(s + (horizon_detection_method == 0 ? "Canny/Hough" : "Gauss/threshold"));
}

