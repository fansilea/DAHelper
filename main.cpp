﻿#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationName("DAHelper");
    QCoreApplication::setApplicationVersion("1.0.0");

    MainWindow w;

    //隐藏边框
    w.setWindowFlags(Qt::FramelessWindowHint);

    //设置窗口背景图片
    w.setStyleSheet("QMainWindow#MainWindow{border-image: url(:/images/bg.jpg)}");

    //w.show();

    return a.exec();
}
