#ifndef CONTACTINFOWINDOW_H
#define CONTACTINFOWINDOW_H

#include <QMainWindow>

#include "Contacts/contact.h"

namespace Ui {
    class ContactInfoWindow;
}

class ContactInfoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ContactInfoWindow(Contact *contact,QWidget *parent = 0);
    ~ContactInfoWindow();

public slots:
    void setContactPhoto();
    void setContactName();
    void setContactStatus();
    void showPhoto();
    void previewPhotoReceived(QString jid);
    void photoReceived(QString jid, QImage photo, QString photoId);
    void showPhotoInImageViewer();
    void copyPhoneToClipboard();
    void dialPhone();
    void onlineStatusChanged(QString jid);
    void userStatusUpdated(QString jid);

signals:
    void photoRefresh(QString jid, QString expectedPhotoId, bool largeFormat);

private:
    Ui::ContactInfoWindow *ui;
    bool photoDownloaded;
    bool isDownloading;

    Contact *contact;

};

#endif // CONTACTINFOWINDOW_H
