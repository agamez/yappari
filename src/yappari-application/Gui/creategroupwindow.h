#ifndef CREATEGROUPWINDOW_H
#define CREATEGROUPWINDOW_H

#include <QMainWindow>

#include "Contacts/contactroster.h"

namespace Ui {
    class CreateGroupWindow;
}

class CreateGroupWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CreateGroupWindow(ContactRoster *roster,QWidget *parent = 0);
    ~CreateGroupWindow();

private:
    Ui::CreateGroupWindow *ui;
    QImage photo;
    QString photoFileName;
    ContactRoster *roster;

signals:
    void createGroupChat(QImage photo, QString subject, QStringList participants);

public slots:
    void verifyGroupName();
    void selectPicture();
    void finishedPhotoSelection(QImage image);
    void setPhoto(QImage image);
    void createProfilePictureWindow();
    void removePhoto();
    void createGroup(QStringList participants);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // CREATEGROUPWINDOW_H
