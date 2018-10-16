#include "form.h"
#include "ui_form.h"
#include <QDebug>

//避免msvc编译时将utf8转为GB2312造成显示中文乱码
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);

    QPalette palette = this->palette();
    palette.setBrush(this->backgroundRole(), QBrush(QPixmap(":/images/bg2.jpg")));
    this->setAutoFillBackground(true);
    this->setPalette(palette);

    countDown = 10;
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_hideRemindWindow()));
    timer.setInterval(1000);
    hint = "点我不再提醒  ";
    ui->pushButton->setText(hint + QString::number(countDown-1));
    //timer.start();
}

Form::~Form()
{
    delete ui;
}

void Form::on_hideRemindWindow()
{
    countDown--;
    ui->pushButton->setText(hint + QString::number(countDown-1));

    if(countDown == 0){
        countDown = 10;
        this->hide();
        timer.stop();
        ui->pushButton->setText(hint + QString::number(countDown-1));
    }
}

void Form::on_showRemindWindow()
{
    timer.start();
    this->show();
}

void Form::on_pushButton_clicked()
{
    this->hide();
    timer.stop();
    emit todayDoNotRemind();
}
