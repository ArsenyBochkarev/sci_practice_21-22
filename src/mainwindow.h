#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool process_started{false};

    bool process_going{false};

private slots:
    // Кнопка Start
    void on_pushButton_clicked();

    // Кнопка Browse
    void on_pushButton_2_clicked();

    // Кнопка Pause/Run
    void on_pushButton_10_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
