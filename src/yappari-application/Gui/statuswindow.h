#ifndef STATUSWINDOW_H
#define STATUSWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

namespace Ui {
    class StatusWindow;
}

class StatusWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit StatusWindow(QWidget *parent = 0);
    ~StatusWindow();

signals:
    void changeStatus(QString status);

public slots:
    void statusSelected(QListWidgetItem *item);
    void verifyStatus();

private:
    Ui::StatusWindow *ui;

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // STATUSWINDOW_H
