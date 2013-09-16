/* Copyright 2013 Naikel Aparicio. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL EELI REILIN OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the author and should not be interpreted as representing
 * official policies, either expressed or implied, of the copyright holder.
 */

#include <QMaemo5InformationBox>
#include <QMessageBox>
#include <QMaemo5Style>
#include <QDir>

#include "Dbus/dbusnokiaimageviewerif.h"

#include "Whatsapp/util/utilities.h"
#include "Whatsapp/util/datetimeutilities.h"

#include "groupinfowindow.h"
#include "groupsubjectwindow.h"
#include "groupparticipantdelegate.h"
#include "profilepicturewindow.h"
#include "selectcontactdialog.h"
#include "ui_groupinfowindow.h"

#include "mainwindow.h"
#include "client.h"
#include "globalconstants.h"

GroupInfoWindow::GroupInfoWindow(Group *group, ContactRoster *roster, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GroupInfoWindow)
{
    ui->setupUi(this);

    model = new ContactSelectionModel(ui->listView);
    ui->listView->setModel(model);
    ui->listView->installEventFilter(this);

    GroupParticipantDelegate *delegate = new GroupParticipantDelegate(ui->listView);

    ui->listView->setItemDelegate(delegate);

    connect(ui->actionLeaveGroup,SIGNAL(triggered()),
            this,SLOT(leaveGroup()));

    connect(ui->actionChangeSubject,SIGNAL(triggered()),
            this,SLOT(openChangeSubjectWindow()));

    connect(ui->actionChangeGroupIcon,SIGNAL(triggered()),
            this,SLOT(selectPicture()));

    connect(ui->actionRemoveGroupIcon,SIGNAL(triggered()),
            this,SLOT(removeGroupIcon()));

    if (group->author == Client::myJid)
    {
        QAction *actionAddParticipant = new QAction(this);
        actionAddParticipant->setObjectName(QString::fromUtf8("actionAddParticipant"));
        actionAddParticipant->setText("Add Participant");

        QMenu *groupMenu = menuBar()->addMenu("&Group");
        groupMenu->addAction(actionAddParticipant);

        // Actions

        connect(actionAddParticipant,SIGNAL(triggered()),
                this,SLOT(addParticipant()));
    }

    QColor color = QMaemo5Style::standardColor("SecondaryTextColor");
    ui->labelParticipants->setStyleSheet("font-size:18px;color:" + color.name());

    this->group = group;
    this->roster = roster;

    setGroupIcon();
    setGroupName();
    showParticipants();

    connect(ui->contactPhotoButton,SIGNAL(clicked()),
            this,SLOT(showPhoto()));

    connect(Client::mainWin,SIGNAL(previewPhotoReceived(QString)),
            this,SLOT(previewPhotoReceived(QString)));

    connect(Client::mainWin,SIGNAL(largePhotoReceived(QString,QImage,QString)),
            this,SLOT(groupIconReceived(QString,QImage,QString)));

    connect(Client::mainWin,SIGNAL(groupParticipantAdded(QString,QString)),
            this,SLOT(groupParticipantAdded(QString,QString)));

    connect(Client::mainWin,SIGNAL(groupParticipantRemoved(QString,QString)),
            this,SLOT(groupParticipantRemoved(QString,QString)));

    connect(Client::mainWin,SIGNAL(groupSubjectUpdated(QString)),
            this,SLOT(groupSubjectUpdated(QString)));

    connect(this,SIGNAL(photoRefresh(QString,QString,bool)),
            Client::mainWin,SLOT(requestPhotoRefresh(QString,QString,bool)));

    connect(this,SIGNAL(requestLeaveGroup(QString)),
            Client::mainWin,SLOT(requestLeaveGroupFromChat(QString)));

    isDownloading = false;
}

GroupInfoWindow::~GroupInfoWindow()
{
    delete ui;
}

void GroupInfoWindow::setGroupIcon()
{
    QDir home = QDir::home();
    QString fileName = home.path() + CACHE_DIR"/"PHOTOS_DIR"/" + group->jid
            + "/" + group->photoId + ".png";
    QFile file(fileName);
    photoDownloaded = (file.exists());

    QIcon icon;
    if (group->photoId.isEmpty())
        icon = QIcon("/usr/share/yappari/icons/64x64/general_conference_avatar.png");
    else
        icon = QIcon(QPixmap::fromImage(group->photo));

    ui->contactPhotoButton->setIcon(icon);
}

void GroupInfoWindow::setGroupName()
{
    QString html = Utilities::WATextToHtml(group->name);

    QString text = "Created: " + DateTimeUtilities::formattedStamp(group->creationTimestamp * 1000);

    text.append("<br>Owned by ");

    if (group->author == Client::myJid)
        text.append("you");
    else
    {
        Contact &c = roster->getContact(group->author);

        text.append(Utilities::WATextToHtml((c.name.isEmpty() ? c.alias : c.name), 24));
    }

    QColor color = QMaemo5Style::standardColor("SecondaryTextColor");
    html.append("<div style=\"font-size:18px;color:" + color.name() +"\">" + text + "</div>");
    ui->labelParticipants->setText("Participants: " + QString::number(group->participants.size()) + " of 50");
    ui->contactNameLabel->setText(html);
}

void GroupInfoWindow::groupParticipantAdded(QString gjid, QString jid)
{
    if (gjid == group->jid)
    {
        addParticipantItemToModel(jid);
        setGroupName();
        model->sort(0);
    }
}

void GroupInfoWindow::addParticipantItemToModel(QString jid)
{
    if (!participants.contains(jid))
    {
        Contact &c = roster->getContact(jid);

        GroupParticipantItem *item = new GroupParticipantItem(&c);
        model->appendRow(item);
        participants.insert(c.jid,item);
    }
}

void GroupInfoWindow::showParticipants()
{
    foreach (QString jid, group->participants)
        addParticipantItemToModel(jid);

    model->sort(0);
}

void GroupInfoWindow::showPhoto()
{
    if (group->photoId.isEmpty())
    {
        QMaemo5InformationBox::information(this,"No group icon");
    }
    else if (isDownloading)
    {
        QMaemo5InformationBox::information(this,"Please wait while the group icon is being downloaded");
    }
    else if (!photoDownloaded)
    {
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, Qt::Checked);
        emit photoRefresh(group->jid, QString(), true);
        isDownloading = true;
        QMaemo5InformationBox::information(this,"Downloading group icon");
    }
    else
        showPhotoInImageViewer();
}

void GroupInfoWindow::groupIconReceived(QString gjid, QImage photo, QString photoId)
{
    if (gjid == group->jid)
    {
        Utilities::logData("Group Icon Received in Group Info Window");

        QDir home = QDir::home();
        QString folder = home.path() + CACHE_DIR"/"PHOTOS_DIR"/" + group->jid;

        // Check the directory is created
        if (!home.exists(folder))
            home.mkpath(folder);

        QString fileName = folder + "/" + photoId + ".png";
        photo.save(fileName,"PNG");

        photoDownloaded = true;
        isDownloading = false;
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, Qt::Unchecked);

        showPhotoInImageViewer();
    }
}

void GroupInfoWindow::showPhotoInImageViewer()
{
    QDir home = QDir::home();
    QString fileName = home.path() + CACHE_DIR"/"PHOTOS_DIR"/" + group->jid
            + "/" + group->photoId + ".png";

    QDBusConnection dbus = QDBusConnection::sessionBus();
    DBusNokiaImageViewerIf *imageViewerBus =
    new DBusNokiaImageViewerIf(NOKIA_IMAGEVIEWER_DBUS_NAME,
                               NOKIA_IMAGEVIEWER_DBUS_PATH,
                               dbus,this);

    imageViewerBus->mime_open("file://" + fileName);
}

void GroupInfoWindow::previewPhotoReceived(QString gjid)
{
    if (group->jid == gjid)
    {
        setGroupIcon();
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, Qt::Unchecked);
    }
}

void GroupInfoWindow::groupSubjectUpdated(QString gjid)
{
    if (group->jid == gjid)
        setGroupName();
}

void GroupInfoWindow::leaveGroup()
{
    QMessageBox msg(this);

    msg.setText("Are you sure you want to leave this group?");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

    if (msg.exec() == QMessageBox::Yes)
    {
        emit requestLeaveGroup(group->jid);
        close();
    }
}

void GroupInfoWindow::openChangeSubjectWindow()
{
    GroupSubjectWindow *groupSubjectWindow = new GroupSubjectWindow(group, this);

    connect(groupSubjectWindow,SIGNAL(changeSubject(QString, QString)),
            Client::mainWin,SLOT(sendSetGroupSubjectFromChat(QString,QString)));

    groupSubjectWindow->setAttribute(Qt::WA_Maemo5StackedWindow);
    groupSubjectWindow->setAttribute(Qt::WA_DeleteOnClose);
    groupSubjectWindow->setWindowFlags(groupSubjectWindow->windowFlags() | Qt::Window);

    groupSubjectWindow->show();
}

void GroupInfoWindow::selectPicture()
{
    QString mediaFolder = Utilities::getPathFor(FMessage::Image, true);

    photoFileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                 mediaFolder,
                                                 EXTENSIONS_IMAGE);

    if (!photoFileName.isNull())
    {
        QMaemo5InformationBox::information(this,"Loading image");

        QTimer::singleShot(100,this,SLOT(createProfilePictureWindow()));

    }
}

void GroupInfoWindow::createProfilePictureWindow()
{
    QImage photo(photoFileName);

    if (photo.width() < PREVIEW_WIDTH || photo.height() < PREVIEW_HEIGHT)
    {

        QMaemo5InformationBox::information(this,"The image is too small.  "\
                                           "Please select a photo with height and width of at least " +
                                           QString::number(PREVIEW_WIDTH) + " pixels",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else
    {
        ProfilePictureWindow *window = new ProfilePictureWindow(photo,this);

        connect(window,SIGNAL(finished(QImage)),
                this,SLOT(finishedPhotoSelection(QImage)));

        window->setAttribute(Qt::WA_Maemo5StackedWindow);
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->setWindowFlags(window->windowFlags() | Qt::Window);
        window->showFullScreen();
    }
}

void GroupInfoWindow::finishedPhotoSelection(QImage image)
{
    setAttribute(Qt::WA_Maemo5ShowProgressIndicator, Qt::Checked);

    // Save the thumbnail
    // It'll be updated in DB when the change is confirmed from WA Servers
    group->photo = image.scaled(QSize(64,64), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    Client::mainWin->requestSetPhoto(group->jid, image);
}

void GroupInfoWindow::removeGroupIcon()
{
    QMessageBox msg(this);

    msg.setText("Are you sure you want to remove the group icon?");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

    if (msg.exec() == QMessageBox::Yes)
    {
        setAttribute(Qt::WA_Maemo5ShowProgressIndicator, Qt::Checked);

        group->photo = QImage();
        Client::mainWin->requestSetPhoto(group->jid, group->photo);
    }
}

void GroupInfoWindow::addParticipant()
{
    ContactRoster *roster = Client::roster;

    if (!roster->size())
    {
        QMaemo5InformationBox::information(this,"Contacts haven't been synchronized yet.",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else if (participants.size() == 50)
    {
        QMaemo5InformationBox::information(this,"The maximum number of participants have been reached.",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else
    {
        SelectContactDialog selectContactDialog(roster,this,false);

        if (selectContactDialog.exec() == QDialog::Accepted)
        {
            Contact& contact = selectContactDialog.getSelectedContact();

            Client::mainWin->requestAddGroupParticipant(group->jid, contact.jid);
        }
    }
}

void GroupInfoWindow::groupParticipantRemoved(QString gjid, QString jid)
{
    if (gjid == group->jid)
    {
        removeParticipantItemFromModel(jid);
        setGroupName();
    }
}

void GroupInfoWindow::removeParticipantItemFromModel(QString jid)
{
    if (participants.contains(jid))
    {
        GroupParticipantItem *item = participants.value(jid);

        model->removeRow(item->row());
        participants.remove(jid);
    }
}

bool GroupInfoWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*> (event);
        QPoint p = mouseEvent->globalPos();

        QModelIndex index = ui->listView->indexAt(
                    ui->listView->viewport()->mapFromGlobal(p));

        if (index.isValid())
        {
            QString jid = index.data(Qt::UserRole + 1).toString();

            if (jid != Client::myJid)
            {
                QMenu *menu = new QMenu(this);
                QAction *removeContact = new QAction("Remove Contact",this);
                QAction *viewContact = new QAction("View Contact",this);

                menu->addAction(viewContact);
                if (group->author == Client::myJid)
                    menu->addAction(removeContact);

                QAction *action = menu->exec(p);

                if (action == removeContact)
                {
                    QMessageBox msg(this);

                    msg.setText("Are you sure you want to remove this contact from this group?");
                    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

                    if (msg.exec() == QMessageBox::Yes)
                        Client::mainWin->requestRemoveGroupParticipant(group->jid, jid);

                    return true;
                }

                else if (action == viewContact)
                {
                    Contact& c = roster->getContact(jid);

                    ContactInfoWindow *window = new ContactInfoWindow(&c,this);

                    Client::mainWin->requestContactStatus(c.jid);
                    Client::mainWin->requestPhotoRefresh(c.jid, c.photoId, false);
                    // emit queryLastOnline(c->jid);

                    window->setAttribute(Qt::WA_Maemo5StackedWindow);
                    window->setAttribute(Qt::WA_DeleteOnClose);
                    window->setWindowFlags(window->windowFlags() | Qt::Window);
                    window->show();
                }
            }

            ui->listView->clearSelection();
        }
    }

    return QMainWindow::eventFilter(obj, event);
}
