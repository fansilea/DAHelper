#include <QDesktopWidget>
#include <QDebug>
#include <QDateTime>
#include <QMouseEvent>
#include "dialog.h"
#include "ui_dialog.h"

//避免msvc编译时将utf8转为GB2312造成显示中文乱码
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

HWND findDesktopIconWnd();
static BOOL enumUserWindowsCB(HWND hwnd, LPARAM lParam);

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    displayBoardAdjust = false;
    embed = false;
    checkIn = "待查询";
    checkOut = "";
    showCheckIn = true;
    timer.setInterval(10); //10s
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_Timeout()));
    ui->setupUi(this);
    ui->label->setOpenExternalLinks(true);
    ui->label->setStyleSheet("background-color:rgb(46,47,48)");
    QRect rect = QApplication::desktop()->screenGeometry(1);
    adjustOffset = abs(rect.x());

    if (QSysInfo::windowsVersion() == QSysInfo::WV_WINDOWS7) {
        //this->setAttribute(Qt::WA_TranslucentBackground, true);
        this->setWindowFlags(Qt::FramelessWindowHint|Qt::Tool);
    }

    timer.start();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_Timeout()
{
    QString styleColorPink  = "<style>a{text-decoration: none;background: transparent; color: rgb(255, 0, 255)} </style>";
    QString styleColorGreen = "<style>a{text-decoration: none;background: transparent; color: rgb(0, 255, 0)} </style>";
    QString styleUrl = "<a href=\"http://personalinfo.sunmedia.com.cn/Attendance.aspx\">";

    if(timer.interval() == 10){
        timer.setInterval(10000);
    }

    if(showCheckIn || checkOut.isEmpty()){
        if(checkIn.contains("查询") == false){
            showCheckIn = false;
        }
        ui->label->setText(styleColorPink + styleUrl + checkIn);
        ui->label->setToolTip("上班打卡时间");
    }else{
        showCheckIn = true;
        ui->label->setText(styleColorGreen + styleUrl + checkOut);
        ui->label->setToolTip("预计下班时间");
    }
}

void Dialog::embedDesktop()
{
    HWND hdesktop = findDesktopIconWnd();
    if(hdesktop){
        WId wid = this->winId();
        SetParent((HWND)wid, hdesktop);
        embed = true;
        if(displayBoardAdjust){
            QPoint point(this->pos());
            point.setX(point.x()+adjustOffset);
            this->move(point);
        }
    }
}

void Dialog::receiveData(QString data)
{
    QString clockOffStr;

    if(data == "?"){
        checkIn = "待查询";
        checkOut = "";
        return;
    }
    if(data == "weekdays"){
        checkIn  = "手动查询";
        checkOut = "";
        return;
    }

    if(data == "未刷卡"){
    }else if(data == "正常"){
            checkOut = "18:00:00";
    }else if(QDateTime::fromString(data, "hh:mm:ss") < QDateTime::fromString(QString("08:46:00"), "hh:mm:ss")){
            checkOut="18:00:00";
    }else if(QDateTime::fromString(data, "hh:mm:ss") < QDateTime::fromString(QString("09:01:00"), "hh:mm:ss")){
            checkOut="18:15:00";
    }else{
        checkOut="18:30:00";
    }

    checkIn = data;
    qDebug() << "上班时间" << checkIn << ",下班时间" << checkOut;
}

void Dialog::mousePressEvent(QMouseEvent *event)
{
    mousePressed = true;
    mousePressedPos = event->pos();
}

void Dialog::mouseReleaseEvent(QMouseEvent *event)
{
    (void*)event;
    mousePressed = false;
}

void Dialog::mouseMoveEvent(QMouseEvent *event)
{
    if(mousePressed && (event->buttons() == Qt::LeftButton)){
        newPos.setX(QCursor::pos().x());
        newPos.setY(QCursor::pos().y());
        newPos -= mousePressedPos;
        if(displayBoardAdjust && embed){
            newPos.setX(newPos.x()+adjustOffset);
        }
        this->move(newPos);
        qDebug() << newPos;
    }
}

void Dialog::displayBoardPositionAdjust(bool adjust)
{
    if(embed){
        QPoint point;
        point.setX(QApplication::desktop()->width()*0.75);
        point.setY(QApplication::desktop()->height()*0.25);
        this->move(point);
    }
    displayBoardAdjust = adjust;    
}

void Dialog::pointGet(QPoint& point)
{
    if(embed){;
        point = newPos;
    }else{
        point = this->pos();
    }
}

static BOOL enumUserWindowsCB(HWND hwnd, LPARAM lParam)
{
    long wflags = GetWindowLong(hwnd, GWL_STYLE);
    if (!(wflags & WS_VISIBLE)) return TRUE;

    HWND sndWnd;
    if (!(sndWnd = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL)))
        return TRUE;

    HWND targetWnd;
    if (!(targetWnd = FindWindowEx(sndWnd, NULL, L"SysListView32", L"FolderView")))
        return TRUE;

    HWND* resultHwnd = (HWND*)lParam;
    *resultHwnd = targetWnd;

    return FALSE;
}

HWND findDesktopIconWnd()
{
    HWND resultHwnd = NULL;
    EnumWindows((WNDENUMPROC)enumUserWindowsCB, (LPARAM)&resultHwnd);
    return resultHwnd;
}
