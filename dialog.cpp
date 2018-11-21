#include "dialog.h"
#include "ui_dialog.h"
#include <QDesktopWidget>
#include <QDebug>
#include <QDateTime>

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
    ui->setupUi(this);
    ui->labelOn->setStyleSheet("color:white");
    ui->labelOff->setStyleSheet("color:yellow");
    ui->labelClockIn->setStyleSheet("color:white");
    ui->labelClockOff->setStyleSheet("color:yellow");
    ui->labelUrl->setOpenExternalLinks(true);
    ui->labelUrl->setText(tr("<style>a{text-decoration: none} </style><a href=\"http://personalinfo.sunmedia.com.cn/Attendance.aspx\">查询"));

    this->setGeometry(QApplication::desktop()->width()-this->width()+10, 30, width(), height());
    HWND hdesktop = findDesktopIconWnd();
    if(hdesktop){
        WId wid = this->winId();
        SetParent((HWND)wid, hdesktop);
    }

//    if (QSysInfo::windowsVersion() == QSysInfo::WV_WINDOWS10) {
//        QPalette pal;
//        pal.setColor(QPalette::Background, QColor(0, 0, 0, 60));
//        setPalette(pal);
//    }

    if (QSysInfo::windowsVersion() == QSysInfo::WV_WINDOWS7) {
        //setWindowOpacity(0);
        //this->setAttribute(Qt::WA_TranslucentBackground, true);
        this->setWindowFlags(Qt::FramelessWindowHint|Qt::Tool);
    }
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::receiveData(QString data)
{
    QString clockOffStr;

    if(data == "?"){
        ui->labelClockIn->setText("待查询");
        return;
    }
    if(data == "weekdays"){
        ui->labelClockIn->setText("非工作日");
        ui->labelClockOff->setText("手动查询");
        return;
    }

    if(data == "未刷卡"){
        clockOffStr="???";
    }else if(data == "正常"){
            clockOffStr = "18:00:00";
    }else if(QDateTime::fromString(data, "hh:mm:ss") < QDateTime::fromString(QString("08:46:00"), "hh:mm:ss")){
            clockOffStr="18:00:00";
    }else if(QDateTime::fromString(data, "hh:mm:ss") < QDateTime::fromString(QString("09:01:00"), "hh:mm:ss")){
            clockOffStr="18:15:00";
    }else{
        clockOffStr="18:30:00";
    }

    qDebug() << "上班时间" << data << ",下班时间" << clockOffStr;

    ui->labelClockIn->setText(data);
    ui->labelClockOff->setText(clockOffStr);
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
    //*resultHwnd = hwnd;//set to workerW
    return FALSE;
}

HWND findDesktopIconWnd()
{
    HWND resultHwnd = NULL;
    EnumWindows((WNDENUMPROC)enumUserWindowsCB, (LPARAM)&resultHwnd);
    return resultHwnd;
}
