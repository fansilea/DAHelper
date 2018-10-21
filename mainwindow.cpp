#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextCodec>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include <QSettings>
#include <QMouseEvent>
#include "xml.h"
#include "dialogabout.h"

//避免msvc编译时将utf8转为GB2312造成文显示中文乱码
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#define DA_URL "http://personalinfo.sunmedia.com.cn/Attendance.aspx"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    manager = new QNetworkAccessManager(this);

    connect(&timer_init, SIGNAL(timeout()), this, SLOT(on_init()));
    timer_init.setInterval(10);
    timer_init.start();
}

MainWindow::~MainWindow()
{
    timer.stop();
    timer_newDay.stop();

    if(QSystemTrayIcon::isSystemTrayAvailable()){
        delete trayIcon;
        delete trayMenu;
        delete quitAction;
    }
    delete manager;
    delete ui;
}

void MainWindow::on_init()
{
    mousePressed = false;
    timer_init.stop();

    //向信息面板显示打卡查询结果
    connect(this, SIGNAL(sendData(QString)), &dlg, SLOT(receiveData(QString)));
    connect(&timer, SIGNAL(timeout()), this, SLOT(on_Timeout()));
    connect(this, SIGNAL(showRemindWindow()), &form, SLOT(on_showRemindWindow()));
    connect(&form, SIGNAL(todayDoNotRemind()), this, SLOT(on_todayDoNotRemind()));
    connect(&timer_newDay, SIGNAL(timeout()), this, SLOT(on_newDayInit()));

    //任务栏通知图标选项
    if(QSystemTrayIcon::isSystemTrayAvailable()){
        quitAction = new QAction(this);
        quitAction->setText("退出程序");
        disWinUpdateAction = new QAction(this);
        disWinUpdateAction->setText("禁止windows自动更新");
        disWinUpdateAction->setChecked(false);
        disWinUpdateAction->setCheckable(true);

        trayMenu = new QMenu((QWidget*)QApplication::desktop());
        trayMenu->addAction(disWinUpdateAction);
        trayMenu->addAction(quitAction);

        trayIcon = new QSystemTrayIcon(this);
        trayIcon->setContextMenu(trayMenu);
        trayIcon->setToolTip(tr("打卡助手"));

        trayIcon->setIcon(QIcon(":/images/tray.png"));
        trayIcon->showMessage("打卡查询助手", "单击显示设置窗口，右击退出应用程序");
        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason)));
        connect(quitAction, SIGNAL(triggered()), this, SLOT(on_quitAction()));
        connect(disWinUpdateAction, SIGNAL(triggered()), this, SLOT(on_disWinUpdate()));
        trayIcon->setVisible(true);
    }

    ui->checkBoxPanel->setChecked(true);
    ui->statusBar->setStyleSheet("color:blue");
    ui->labelTitle->setStyleSheet("color:blue");
    ui->comboBoxRemindStartTime->setEditText("08:30:00");
    ui->lineEditName->setStyleSheet("background:transparent;border-width:0;border-style:outset");
    ui->lineEditPwd->setStyleSheet("background:transparent;border-width:0;border-style:outset");

    //读取配置，恢复界面上次设置状态
    readConfig();

    timer.setInterval(1000);
    timer.start();
    timer_newDay.setInterval(100);
    timer_newDay.start();

    //检查是否是开机启动
    QStringList arguments = QCoreApplication::arguments();
    if(arguments.count() == 1){
        this->show();
    }

    //form.show();
}

void MainWindow::startRequest()
{
    getReply = manager->get(QNetworkRequest(QUrl(DA_URL)));
    connect(getReply, SIGNAL(finished()), this, SLOT(httpGetFinished()));
}

void MainWindow::httpGetFinished()
{
    int statusCode = getReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(getReply->error() != QNetworkReply::NoError){
        qDebug() << "get reply error, status code:" << statusCode;
        return;
    }

    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QString all = codec->toUnicode(getReply->readAll());
    getReply->deleteLater();

    //find today NO.
    QString findTodayNoSubStr("#FFCC66\" width=\"14%\"><font face=\"Verdana\" color=\"White\" size=\"1\"><a href=\"javascript:__doPostBack('Calendar1','");
    qint32 todayNoIndexof1=all.indexOf(findTodayNoSubStr) + findTodayNoSubStr.size();
    qint32 todayNoIndexof2=all.indexOf(QString("'"), todayNoIndexof1);
    QString todayNo=all.mid(todayNoIndexof1, todayNoIndexof2-todayNoIndexof1);
    qDebug() << "Today NO.:" << todayNo;

    //find __VIEWSTATE VALUE
    QString findViewStateSubStr("id=\"__VIEWSTATE\" value=\"");
    qint32 viewStateValueIndexof1=all.indexOf(findViewStateSubStr)+findViewStateSubStr.size();
    qint32 viewStateValueIndexof2=all.indexOf(QString("\""), viewStateValueIndexof1);
    QString viewStateValue=all.mid(viewStateValueIndexof1, viewStateValueIndexof2-viewStateValueIndexof1);
    qDebug() << "__VIEWSTATE:" << viewStateValue;

    //find __EVENTVALIDATION VALUE
    QString findEventValiDationSubStr("id=\"__EVENTVALIDATION\" value=\"");
    qint32 eventValiDationValueIndexof1=all.indexOf(findEventValiDationSubStr)+findEventValiDationSubStr.size();
    qint32 eventValiDationValueIndexof2=all.indexOf(QString("\""), eventValiDationValueIndexof1);
    QString eventValiDationValue=all.mid(eventValiDationValueIndexof1, eventValiDationValueIndexof2-eventValiDationValueIndexof1);
    qDebug() << "__EVENTVALIDATION:" << eventValiDationValue;

    //post
    QNetworkRequest request;
    request.setUrl(QUrl(DA_URL));
    request.setRawHeader("Connection","keep-alive");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");
    request.setRawHeader("Referer", DA_URL);
    request.setRawHeader("Accept-Encoding","gzip, deflate");

    QByteArray postData;
    postData.append("__EVENTTARGET=Calendar1");
    postData.append("&__EVENTARGUMENT=" + todayNo);
    postData.append("&__VIEWSTATE=" + viewStateValue);
    //将'+'转为网页编码表示的'%2b'，否则会被自动转为空格
    eventValiDationValue.replace("+", "%2b");
    postData.append("&__EVENTVALIDATION=" + eventValiDationValue);

    postReply = manager->post(request, postData);
    connect(postReply, SIGNAL(finished()), this, SLOT(httpPostFinished()));
}

void MainWindow::httpPostFinished()
{
    int statusCode = postReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(postReply->error() != QNetworkReply::NoError){
        qDebug() << "post reply error, status code:" << statusCode;
        return;
    }

    QTextCodec *codec = QTextCodec::codecForName("utf8");
    for(;;){
        QString line = codec->toUnicode(postReply->readLine());
        if(postReply->atEnd()){
            postReply->deleteLater();
            return;
        }
        if(line.contains(userName)){
            line.replace("</td><td>", ",");
            line.replace("<td>", "");
            line.replace("</td>", "");
            line.replace("<font color=\"#333333\">", "");
            line.replace("</font>", "");
            date = line.section(',', 0, 0).trimmed();
            name = line.section(',', 1, 1).trimmed();
            shortName = line.section(',', 2, 2).trimmed();
            id = line.section(',', 3, 3).trimmed();
            clockIn = line.section(',', 4, 4).trimmed();
            clockOff = line.section(',', 5, 5).trimmed();
            postReply->deleteLater();
            break;
        }
    }

    qDebug() << "date:" << date << "name:" << name << "short name:" << shortName << "id:" << id << "clock in:" << clockIn << "clock off:" << clockOff;

    //显示查询结果
    if(clockIn.isEmpty()){
        return;
    }

    timer.stop();
    emit sendData(clockIn);
    qint32 nextIntervalInSec = 0;

    if(clockIn == QString("未刷卡") && todayDoNotRemind == false){
        QString workDay = QDateTime::currentDateTime().toString("ddd");
        if(remindOnlyWorkDay == true && (workDay == "周六" || workDay == "周日")){
            emit sendData(QString("weekends"));
        }else{
            QDateTime remindStartTime;
            QDateTime remindEndTime;
            QString currentTimeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
            QDateTime currentTime = QDateTime::fromString(currentTimeStr, "hh:mm:ss");

            if(ui->comboBoxRemindEndTime->currentIndex() != 0){
                remindEndTime = QDateTime::fromString(ui->comboBoxRemindEndTime->currentText().trimmed(), "hh:mm:ss");
            }else{
                remindEndTime = QDateTime::fromString("10:00:00", "hh:mm:ss");
            }

            if(currentTime < remindEndTime){
                //弹出提醒框
                emit showRemindWindow();
                if(currentTime > QDateTime::fromString("11:00:00", "hh:mm:ss")){
                    nextIntervalInSec = 600;
                }else if(currentTime > QDateTime::fromString("09:15:00", "hh:mm:ss")){
                    nextIntervalInSec = 300;
                }else{
                    nextIntervalInSec = 60;
                }
            }
        }
    }

    //计算下一次查询时间
    if(nextIntervalInSec == 0){
        qint32 remindStartTimeInSec;
        if(ui->comboBoxRemindStartTime->currentIndex() != 0){
            remindStartTimeInSec = QDateTime::fromString(ui->comboBoxRemindStartTime->currentText().trimmed(), "hh:mm:ss").time().msecsSinceStartOfDay()/1000;
        }else{
            remindStartTimeInSec = QDateTime::fromString("08:30:00", "hh:mm:ss").time().msecsSinceStartOfDay()/1000;
        }
        qint32 secsSinceStartOfDay = QTime::currentTime().msecsSinceStartOfDay()/1000;
        qint32 timerIntervalInSec = 24 * 3600 - secsSinceStartOfDay + remindStartTimeInSec;
        nextIntervalInSec = timerIntervalInSec;
    }

    qDebug() << "nextInterval" << nextIntervalInSec << "s";
    timer.setInterval(nextIntervalInSec * 1000);
    timer.start();
}

void MainWindow::on_pushButtonTray_clicked()
{
    this->hide();
}

void MainWindow::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reson)
{
    switch(reson){
        case QSystemTrayIcon::Trigger:
            this->showNormal();
            break;
        case QSystemTrayIcon::DoubleClick:
            break;
        default:
            break;
    }
}

void MainWindow::on_quitAction()
{
    qApp->quit();
}

void MainWindow::on_disWinUpdate()
{
    if(disWinUpdateAction->isChecked()){
        QProcess process;
        QString app = "wuauserv.exe";
        QString all;

        process.start("tasklist.exe", QStringList() << "/fi" << QString("imagename eq ") + app);
        if(process.waitForFinished()){
            all = QString::fromLocal8Bit(process.readAllStandardOutput()).simplified();
            if(all.contains(app)){
                process.start("taskkill.exe", QStringList() << "/im" << app << "/f");
                if(process.waitForFinished()){
                    all = QString::fromLocal8Bit(process.readAllStandardOutput()).simplified();
                    if(all.contains("成功")){
                        trayIcon->showMessage("温馨提示", "停止windows update应用");
                    }
                }
            }
        }

        process.start("sc.exe", QStringList() << "query" << "wuauserv");
        if(process.waitForFinished()){
            all = QString::fromLocal8Bit(process.readAllStandardOutput()).simplified();
            qDebug() << all;
            if(all.contains("RUNNING")){
                process.start("sc.exe", QStringList() << "stop" << "wuauserv");
                process.waitForFinished();
                process.start("sc.exe", QStringList() << "query" << "wuauserv");
                process.waitForFinished();
                all = QString::fromLocal8Bit(process.readAllStandardOutput()).simplified();
                if(all.contains("STOPPED")){
                    trayIcon->showMessage("温馨提示", "windows update服务已停止");
                }
            }
            process.start("sc.exe", QStringList() << "qc" << "wuauserv");
            process.waitForFinished();
            all = QString::fromLocal8Bit(process.readAllStandardOutput()).simplified();
            if(all.isEmpty() == false && all.contains("DISABLED") == false){
                process.start("sc.exe", QStringList() << "config" << "wuauserv" << "start=" << "disabled");
                process.waitForFinished();
                process.start("sc.exe", QStringList() << "qc" << "wuauserv");
                process.waitForFinished();
                all = QString::fromLocal8Bit(process.readAllStandardOutput()).simplified();
                if(all.contains("DISABLED")){
                    trayIcon->showMessage("温馨提示", "windows update服务已禁用");
                }
            }
        }
    }
    saveConfig();
}

void MainWindow::on_Timeout()
{
    startRequest();
}

void MainWindow::on_pushButtonQuit_clicked()
{
    qApp->quit();
}

void MainWindow::webAutoAuth()
{
    if(ui->checkBoxWebAuth->isChecked() == false){
        return;
    }

    //post
    QNetworkRequest request;
    request.setUrl(QUrl("http://172.28.253.252/ac_portal/login.php"));
    request.setRawHeader("Connection","keep-alive");
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded; charset=UTF-8");
    request.setRawHeader("Accept","*/*");
    request.setRawHeader("X-Requested-with", "XMLHttpRequest");
    request.setRawHeader("DNT", "1");
    request.setRawHeader("Referer", "http://172.28.253.252/ac_portal/default/pc.html?tabs=pwd");
    request.setRawHeader("Accept-Encoding","gzip, deflate");
    request.setRawHeader("Cookie", "ac_login_info=passwork");

    QByteArray postData;
    postData.append("opr=pwdLogin");
    postData.append("&userName=" + userName);
    postData.append("&pwd=" + password);
    postData.append("&rememberPwd=0");

    webLoginReply = manager->post(request, postData);
    connect(webLoginReply, SIGNAL(finished()), this, SLOT(webAutoAuthFinished()));
}

void MainWindow::webAutoAuthFinished()
{
    if(webAuthed == true){
        return;
    }

    int statusCode = webLoginReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(webLoginReply->error() != QNetworkReply::NoError){
        qDebug() << "failed to web auth, status code:" << statusCode;
        return;
    }

    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QString webAuthReplyStr = codec->toUnicode(webLoginReply->readAll());
    if(webAuthReplyStr.contains("logon success")){
        qDebug() << "web auth success";
        trayIcon->showMessage("温馨提示", "上网认证成功");
        //ui->statusBar->showMessage("上网认证成功！", 3000);
        webAuthed = true;
    }else{
        qDebug() << "web auth fail";
        webAuthed = false;
        //ui->statusBar->showMessage("上网认证失败！", 3000);
    }
}

void MainWindow::on_checkBoxWebAuth_stateChanged(int arg1)
{
    (void)arg1;

    if(ui->checkBoxWebAuth->isChecked()){
        if(ui->lineEditName->text().isEmpty() || ui->lineEditPwd->text().isEmpty()){
            ui->checkBoxWebAuth->setCheckState(Qt::Unchecked);
            ui->statusBar->showMessage("请先设置用户名和密码！", 3000);
            return;
        }
        if(userName != ui->lineEditName->text().simplified() || password != ui->lineEditPwd->text().simplified()){
            userName = ui->lineEditName->text().simplified();
            password = ui->lineEditPwd->text().simplified();
        }
        if(webAuthed == false){
            webAutoAuth();
        }
    }
}

void MainWindow::on_checkBoxPanel_stateChanged(int arg1)
{
    (void)arg1;

    if(ui->checkBoxPanel->isChecked()){
        dlg.show();
    }else{
        dlg.hide();
    }

}

void MainWindow::on_todayDoNotRemind()
{
    todayDoNotRemind = true;
}

void MainWindow::on_newDayInit()
{
    //每天早上7:00更新
    qDebug() << "new day";
    timer_newDay.stop();
    webAuthed = false;
    todayDoNotRemind = false;
    remindOnlyWorkDay = false;
    webAutoAuth();
    emit sendData("weekdays");
    qint32 currentSecs = QDateTime::currentDateTime().time().msecsSinceStartOfDay() / 1000;
    timer_newDay.setInterval((24*3600-currentSecs+7*3600)*1000);
    timer_newDay.start();
}

void MainWindow::on_checkBoxRemindOnlyWorkDay_stateChanged(int arg1)
{
    (void)arg1;

    if(ui->checkBoxRemindOnlyWorkDay->isChecked()){
        remindOnlyWorkDay = true;
    }else{
        remindOnlyWorkDay = false;
    }
}

void MainWindow::on_checkBoxAutoRun_stateChanged(int arg1)
{
    (void)arg1;
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);

    if(ui->checkBoxAutoRun->isChecked()){
        reg.setValue("DAHelper", "\"" + QDir::toNativeSeparators(QApplication::applicationFilePath()) + "\" start");

    }else{
        reg.setValue("DAHelper", "");
    }
}

void MainWindow::readConfig(){

    Xml xml(QApplication::applicationDirPath() + "/da.xml");
    QMap<QString, QString> data;
    bool autoRun=false;
    bool showPanel=false;
    bool webAuth=false;
    bool workDayOnly=false;

    if(xml.init() == true){
        xml.subNode("webauth", data);
        if(data.isEmpty() == false){
            qDebug() << "webauth: username" << data["username"] << ",pwd" << data["password"] << ",enable" << data["enable"];
            if(data["username"].isEmpty() == false && data["password"].isEmpty() == false){
                ui->lineEditName->setText(data["username"]);
                ui->lineEditPwd->setText(data["password"]);
                if(data["enable"] == "true"){
                    webAuth = true;
                }
            }
        }

        data.clear();
        xml.subNode("remind", data);
        if(data.isEmpty() == false){
            qDebug() << "remind: workdayonly" << data["workdayonly"] << ",remindstart" << data["remindstart"] << ",remindend" << data["remindend"];
            if(data["workdayonly"] == "true"){
                workDayOnly = true;
            }
            QString placeholders("     ");
            if(data["remindstart"].isEmpty() == false){
                ui->comboBoxRemindStartTime->setCurrentText(placeholders + data["remindstart"]);
            }
            if(data["remindend"].isEmpty() == false){
                ui->comboBoxRemindEndTime->setCurrentText(placeholders + data["remindend"]);
            }
        }

        data.clear();
        xml.subNode("other", data);
        if(data.isEmpty() == false){
            qDebug() << "other: autorun" << data["autorun"] << ",showpanel" << data["panel"];
            if(data["autorun"] == "true"){
                QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);
                if(reg.value("DAHelper").toString() != QApplication::applicationFilePath()){
                    reg.setValue("DAHelper", "\"" + QDir::toNativeSeparators(QApplication::applicationFilePath()) + "\" start");
                }
                autoRun = true;
            }
            if(data["panel"] == "true"){
                showPanel = true;
            }
            if(data["windowsupdate"] == "false"){
                disWinUpdateAction->setChecked(true);
                on_disWinUpdate();
            }
        }
    }else{
        showPanel = true;
        workDayOnly = true;
    }

    //获取当前登陆用户名
    if(userName.isEmpty()){
        userName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).section("/", -1, -1);
    }

    ui->checkBoxAutoRun->setChecked(autoRun);
    ui->checkBoxPanel->setChecked(showPanel);
    ui->checkBoxWebAuth->setChecked(webAuth);
    ui->checkBoxRemindOnlyWorkDay->setChecked(workDayOnly);
    ui->lineEditName->setText(userName);
}

void MainWindow::saveConfig()
{
    Xml xml(QApplication::applicationDirPath() + "/da.xml");
    QMap<QString, QString> data;

    xml.init(false);

    if(ui->lineEditName->text().isEmpty() == false &&
       ui->lineEditPwd->text().isEmpty() == false){
        userName = ui->lineEditName->text().simplified();
        password = ui->lineEditPwd->text().simplified();
        data.insert("username", userName);
        data.insert("password", password);
        if(ui->checkBoxWebAuth->isChecked()){
            data.insert("enable", "true");
        }
        xml.addSubNode("webauth", data);
    }

    data.clear();
    if(ui->comboBoxRemindStartTime->currentIndex() != 0){
        data.insert("remindstart", ui->comboBoxRemindStartTime->currentText().trimmed());
    }
    if(ui->comboBoxRemindEndTime->currentIndex() != 0){
        data.insert("remindend", ui->comboBoxRemindEndTime->currentText().trimmed());
    }
    if(ui->checkBoxRemindOnlyWorkDay->isChecked()){
        data.insert("workdayonly", "true");
    }
    xml.addSubNode("remind", data);

    data.clear();
    if(ui->checkBoxAutoRun->isChecked()){
        data.insert("autorun", "true");
    }
    if(ui->checkBoxPanel->isChecked()){
        data.insert("panel", "true");
    }
    if(disWinUpdateAction->isChecked()){
        data.insert("windowsupdate", "false");
    }
    if(data.isEmpty() == false){
        xml.addSubNode("other", data);
    }

    xml.save();

    //更新timer倒计时
    timer.stop();
    timer.setInterval(5000);
    timer.start();
}

void MainWindow::on_pushButtonSaveConfig_clicked()
{
    saveConfig();
}

void MainWindow::on_pushButtonAbout_clicked()
{
   DialogAbout about(this);
   about.exec();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    mousePressed = true;
    mousePressedPos = event->pos();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    (void*)event;
    mousePressed = false;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(mousePressed && (event->buttons() == Qt::LeftButton)){
        QPoint newPos(QCursor::pos() - mousePressedPos);
        this->move(newPos);
    }
}
