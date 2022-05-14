#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include "structures.h"
#include "shake_compensation.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    // Аргументы для запуска основого цикла
    std::vector<Mat> all_frames_vec;
    std::vector<transform_parameters> smooth_transforms;
    std::vector<std::vector<Point2f> >all_found_fp_vec;
    std::vector<std::pair<double, double> > all_horizon_coords;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void change_buttons_visibility(bool v);
    void set_objects_size();

    void show_frame();
    std::pair<double, double> detect_on_frame();
    int start_process();
    void detect_and_show_cycle();



    // Процесс был запущен хотя бы раз
    bool process_started{false};

    // Процесс идёт
    bool process_going{false};


    // Задержка (в мс) между показами кадров при нажатии кнопки Run
    int delay{10};


    // Текущий кадр
    unsigned long long frame_num{0};


    // Следующие параметры вынимаются из соответствующих полей
    // Раз в сколько кадров будет возможно пересчитывание горизонта - сделано в угоду производительности
    unsigned int change_rate{3};


    // Способ построения горизонта
    bool horizon_detection_method{0};


    // Наибольшая разрешенная разница между суммированными расстояниями до горизонта между "фичами" на прошлом и текущем кадре
    double max_sum_horizon_diff{1200};


    // Наибольшая разрешенная разница между соответствующими расстояниями до горизонта у одной конкретной "фичи" на прошлом и текущем кадре
    double max_single_point_horizon_diff{400};


    // Наибольшее разрешенное число точек, у которых разница между соответствующими расстояниями до горизонта на
    // первом и втором кадре превышает max_single_point_horizon_diff
    unsigned int max_wrong_fp_num{5};


    // Наибольшая разрешенная разница между общим количеством "фич" и числом точек, сместившихся в конкретном направлении (вверх или вниз)
    unsigned long long max_horizon_correctness_diff{20};



private slots:
    // Кнопка Start
    void on_pushButton_clicked();

    // Кнопка Browse
    void on_pushButton_2_clicked();

    // Кнопка Pause/Run
    void on_pushButton_10_clicked();

    void on_prev_frame_button_clicked();

    void on_next_frame_button_clicked();

    void on_delay_spinBox_valueChanged(int arg1);

    void on_frame_rate_spinBox_valueChanged(int arg1);

    void on_curr_frame_spinBox_valueChanged(int arg1);

    void on_left_hor_spinBox_valueChanged(int arg1);

    void on_right_hor_spinBox_valueChanged(int arg1);

    void on_detect_horizon_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
