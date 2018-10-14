#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QPalette>
#include <QSystemTrayIcon>
#include "dialog.h"
#include "form.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void startRequest();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *manager;
    QNetworkReply *getReply;
    QNetworkReply *postReply;
    QNetworkReply *webLoginReply;

    Dialog dlg;
    Form form;

    QString name;
    QString shortName;
    QString id;
    QString date;
    QString clockIn;
    QString clockOff;

    //通知栏相关
    QMenu *trayMenu;
    QAction *quitAction;
    QAction *settingAction;
    QSystemTrayIcon *trayIcon;

    //定时查询打卡记录
    QTimer timer;
    //初始化新的一天的一些状态值
    QTimer timer_newDay;
    QTimer timer_init;

    bool webAuthed;
    QString userName;
    QString password;
    bool todayDoNotRemind;
    bool remindOnlyWorkDay;

private slots:
    void httpGetFinished();
    void httpPostFinished();

    void on_pushButtonTray_clicked();
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason);
    void on_quitAction();
    void on_Timeout();
    void on_todayDoNotRemind();
    void on_newDayInit();

    void on_pushButtonQuit_clicked();
    void webAutoAuthFinished();

    void on_checkBoxWebAuth_stateChanged(int arg1);
    void on_checkBoxPanel_stateChanged(int arg1);

    void on_checkBoxRemindOnlyWorkDay_stateChanged(int arg1);

    void on_checkBoxAutoRun_stateChanged(int arg1);

    void on_init();

    void on_pushButtonSaveConfig_clicked();

    void on_pushButtonAbout_clicked();

signals:
    void sendData(QString);
    void showRemindWindows();

private:
    void webAutoAuth();
    void saveConfig();
    void readConfig();
};

#endif // MAINWINDOW_H
