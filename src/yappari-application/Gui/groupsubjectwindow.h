#ifndef GROUPSUBJECTWINDOW_H
#define GROUPSUBJECTWINDOW_H

#include <QMainWindow>

#include "Contacts/group.h"

namespace Ui {
    class GroupSubjectWindow;
}

class GroupSubjectWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GroupSubjectWindow(Group *group, QWidget *parent = 0);
    ~GroupSubjectWindow();

signals:
    void changeSubject(QString gjid, QString newSubject);

public slots:
    void verifyNewSubject();

private:
    Ui::GroupSubjectWindow *ui;

    Group *group;
};

#endif // GROUPSUBJECTWINDOW_H
