
#include <QFileDialog>
#include "mainwindow.h"
#include "main.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QMessageBox>
#include <QLabel>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->verticalLayout->setGeometry(QRect(0, 0, ui->verticalLayout->geometry().width(), ui->verticalLayout->geometry().height()));

    std::cout << "ui->pushButton->geometry().right() == " << ui->pushButton->geometry().right() << "\n";

    // не совсем понятно, почему pushButton увеличивается в два раза, но ок
    int x{ui->pushButton->geometry().right()*2 + 10};
    int y{ui->pushButton->geometry().top()};

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    int width{abs(screenGeometry.width())};
    int height{abs(screenGeometry.height())};

    ui->image_label->setGeometry(QRect(x, y, width, height));

    // Делаем невидимой кнопку Pause/Run
    ui->pushButton_10->setVisible(0);

    // Делаем невидимой поле с вводом номера кадра
    ui->curr_frame_lineEdit->setVisible(0);

    // Делаем невидимой кнопку перехода к введённому кадру
    ui->pushButton_10->setVisible(0);

}

MainWindow::~MainWindow()
{
    delete ui;
}

// Кнопка Start
void MainWindow::on_pushButton_clicked()
{

    if (!process_started)
    {
        std::string s{ui->lineEdit->text().toUtf8().constData()};

        process_started = 1;

        // Запускаем пре-проход по всем кадрам
        start_process(s, ui->image_label, ui->curr_frame_lineEdit);

        // Делаем видимой кнопку Pause/Run
        ui->pushButton_10->setVisible(1);

        // Делаем видимой поле с вводом номера кадра
        ui->curr_frame_lineEdit->setVisible(1);

        // Делаем видимой кнопку перехода к введённому кадру
        ui->pushButton_10->setVisible(1);

        // Запускаем основной цикл
        process_going = 1;
        on_pushButton_10_clicked();
    }
}

// Кнопка Browse
void MainWindow::on_pushButton_2_clicked()
{
    // Возможно, забыл какие-то форматы
    QString filter{"MP4 (*.mp4) ;; AVI (*.AVI)"};
    QString file{QFileDialog::getOpenFileName(this, "Choose a file", QDir::currentPath(), filter)};
    ui->lineEdit->setText(file);
}


// Кнопка Pause/Run
void MainWindow::on_pushButton_10_clicked()
{
    // Процесс идёт и была нажата кнопка
    if (process_going)
    {
        // Меняем название
        ui->pushButton_10->setText("Run");




        process_going = 0;
    }
    else
    {

    }
}

