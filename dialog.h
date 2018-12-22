#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTimer>
#include <windows.h>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
   explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    void embedDesktop();
    void leaveDesktop();
    void displayBoardPositionAdjust(bool adjust);
    void pointGet(QPoint& point);


private slots:
    void receiveData(QString);
    void on_Timeout();

private:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    bool displayBoardAdjust;
    bool mousePressed;
    bool embed;
    bool showCheckIn;
    int adjustOffset;
    QPoint newPos;
    QTimer timer;
    QString checkIn;
    QString checkOut;
    HWND wparent;
    QPoint mousePressedPos;
    Ui::Dialog *ui;
};

#endif // DIALOG_H
