#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = 0);
    ~Form();

private slots:
    void on_showRemindWindow();
    void on_hideRemindWindow();

    void on_pushButton_clicked();

signals:
    void todayDoNotRemind();

private:
    Ui::Form *ui;

    QTimer timer;
    QString hint;
    qint32 countDown;
};

#endif // FORM_H
