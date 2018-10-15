#include "dialogabout.h"
#include "ui_dialogabout.h"

//避免msvc编译时将utf8转为GB2312造成显示中文乱码
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

DialogAbout::DialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAbout)
{
    ui->setupUi(this);

    ui->textBrowser->setStyleSheet("background-color:rgb(255,132,139,0);border-radius:3px;color:rgb(0,0,0);}");
}

DialogAbout::~DialogAbout()
{
    delete ui;
}
