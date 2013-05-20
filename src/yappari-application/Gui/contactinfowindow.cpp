#include <QMaemo5InformationBox>
#include <QMaemo5Style>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QDir>

#include "contactinfowindow.h"
#include "ui_contactinfowindow.h"

#include "Dbus/dbusnokiaimageviewerif.h"
#include "Dbus/dbusnokiacsdcallif.h"

#include "Whatsapp/util/utilities.h"
#include "Whatsapp/util/datetimeutilities.h"

#include "mainwindow.h"
#include "client.h"
#include "globalconstants.h"

ContactInfoWindow::ContactInfoWindow(Contact *contact, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ContactInfoWindow)
{
    this->contact = contact;

    ui->setupUi(this);

    ui->phoneNumberLabel->setText(contact->phone);

    setContactPhoto();
    setContactName();
    setContactStatus();

    connect(ui->contactPhotoButton,SIGNAL(clicked()),
            this,SLOT(showPhoto()));

    connect(ui->copyButton,SIGNAL(clicked()),
            this,SLOT(copyPhoneToClipboard()));

    connect(ui->dialButton,SIGNAL(clicked()),
            this,SLOT(dialPhone()));

    QDir home = QDir::home();
    QString fileName = home.path() + CACHE_DIR"/"PHOTOS_DIR"/" + contact->jid
            + "/" + contact->photoId + ".png";
    QFile file(fileName);
    photoDownloaded = (file.exists());
    isDownloading = false;

    connect(Client::mainWin,SIGNAL(previewPhotoReceived(QString)),
            this,SLOT(previewPhotoReceived(QString)));

    connect(Client::mainWin,SIGNAL(largePhotoReceived(QString,QImage,QString)),
            this,SLOT(photoReceived(QString,QImage,QString)));

    connect(Client::mainWin,SIGNAL(onlineStatusChanged(QString)),
            this,SLOT(onlineStatusChanged(QString)));

    connect(Client::mainWin,SIGNAL(userStatusUpdated(QString)),
            this,SLOT(userStatusUpdated(QString)));

    connect(this,SIGNAL(photoRefresh(QString,QString,bool)),
            Client::mainWin,SLOT(requestPhotoRefresh(QString,QString,bool)));
}

ContactInfoWindow::~ContactInfoWindow()
{
    delete ui;
}

void ContactInfoWindow::setContactPhoto()
{
    QIcon icon;
    if (contact->photoId == "abook" || contact->photoId.isEmpty())
        icon = QIcon("/usr/share/icons/hicolor/64x64/hildon/general_default_avatar.png");
    else
        icon = QIcon(QPixmap::fromImage(contact->photo));

    ui->contactPhotoButton->setIcon(icon);
}

void ContactInfoWindow::setContactName()
{
    QString html = Utilities::WATextToHtml(contact->name);

    if (!contact->alias.isEmpty())
        html.append(" (" + Utilities::WATextToHtml(contact->alias) + ")");

    QString text;
    if (!contact->isOnline)
    {
        if (!contact->lastSeen)
            text = "Offline";
        else
            text = "Last seen " + DateTimeUtilities::formattedStamp(contact->lastSeen);
    }
    else
        text = "Online";

    QColor color = QMaemo5Style::standardColor("SecondaryTextColor");
    html.append("<div style=\"font-size:18px;color:" + color.name() +"\">" + text + "</div>");
    ui->contactNameLabel->setText(html);
}

void ContactInfoWindow::setContactStatus()
{
    ui->statusLabel->setText(Utilities::WATextToHtml(contact->status));
}

void ContactInfoWindow::showPhoto()
{
    if (contact->photoId == "abook" || contact->photoId.isEmpty())
    {
        QMaemo5InformationBox::information(this,"No profile photo");
    }
    else if (isDownloading)
    {
        QMaemo5InformationBox::information(this,"Please wait while the photo is being downloaded");
    }
    else if (!photoDownloaded)
    {
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, Qt::Checked);
        emit photoRefresh(contact->jid, QString(), true);
        isDownloading = true;
        QMaemo5InformationBox::information(this,"Downloading photo");
        ui->messageLabel->setText("Please wait while the photo is being downloaded...");
    }
    else
        showPhotoInImageViewer();
}

void ContactInfoWindow::photoReceived(QString jid, QImage photo, QString photoId)
{
    if (jid == contact->jid)
    {
        Utilities::logData("Photo received in Profile Window");

        QDir home = QDir::home();
        QString folder = home.path() + CACHE_DIR"/"PHOTOS_DIR"/" + contact->jid;

        // Check the directory is created
        if (!home.exists(folder))
            home.mkpath(folder);

        QString fileName = folder + "/" + photoId + ".png";
        photo.save(fileName,"PNG");

        photoDownloaded = true;
        isDownloading = false;
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, Qt::Unchecked);
        ui->messageLabel->clear();

        showPhotoInImageViewer();
    }
}

void ContactInfoWindow::showPhotoInImageViewer()
{
    QDir home = QDir::home();
    QString fileName = home.path() + CACHE_DIR"/"PHOTOS_DIR"/" + contact->jid
            + "/" + contact->photoId + ".png";

    QDBusConnection dbus = QDBusConnection::sessionBus();
    DBusNokiaImageViewerIf *imageViewerBus =
    new DBusNokiaImageViewerIf(NOKIA_IMAGEVIEWER_DBUS_NAME,
                               NOKIA_IMAGEVIEWER_DBUS_PATH,
                               dbus,this);

    imageViewerBus->mime_open("file://" + fileName);
}

void ContactInfoWindow::copyPhoneToClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(contact->phone);
    QMaemo5InformationBox::information(this,"Phone number copied to clipboard");
}

void ContactInfoWindow::dialPhone()
{

    QDBusConnection dbus = QDBusConnection::systemBus();
    DBusNokiaCsdCallIf *csdCallBus = new DBusNokiaCsdCallIf(NOKIA_CSD_CALL_SERVICE,
                                                            NOKIA_CSD_CALL_OBJECT,
                                                            dbus,this);

    csdCallBus->CreateWith(contact->phone, 0);
}

void ContactInfoWindow::onlineStatusChanged(QString jid)
{
    if (contact->jid == jid)
        setContactName();
}

void ContactInfoWindow::userStatusUpdated(QString jid)
{
    if (contact->jid == jid)
        setContactStatus();
}

void ContactInfoWindow::previewPhotoReceived(QString jid)
{
    if (contact->jid == jid)
        setContactPhoto();
}


