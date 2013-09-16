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

#ifndef GROUPINFOWINDOW_H
#define GROUPINFOWINDOW_H

#include <QMainWindow>

#include "groupparticipantitem.h"
#include "contactselectionmodel.h"

#include "Contacts/contactroster.h"
#include "Contacts/group.h"

namespace Ui {
    class GroupInfoWindow;
}

class GroupInfoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GroupInfoWindow(Group *group, ContactRoster *roster, QWidget *parent = 0);
    ~GroupInfoWindow();

public slots:
    void groupParticipantAdded(QString gjid, QString jid);
    void groupParticipantRemoved(QString gjid, QString jid);
    void groupIconReceived(QString gjid, QImage photo, QString photoId);
    void showPhoto();
    void previewPhotoReceived(QString gjid);
    void groupSubjectUpdated(QString gjid);
    void leaveGroup();
    void openChangeSubjectWindow();
    void selectPicture();
    void removeGroupIcon();
    void createProfilePictureWindow();
    void finishedPhotoSelection(QImage image);
    void addParticipant();

signals:
    void photoRefresh(QString jid, QString expectedPhotoId, bool largeFormat);
    void requestLeaveGroup(QString gjid);

private:
    Ui::GroupInfoWindow *ui;

    Group *group;
    ContactRoster *roster;
    ContactSelectionModel *model;
    QHash<QString,GroupParticipantItem *> participants;
    QString photoFileName;

    bool photoDownloaded;
    bool isDownloading;

    void setGroupIcon();
    void setGroupName();
    void showParticipants();
    void addParticipantItemToModel(QString jid);
    void removeParticipantItemFromModel(QString jid);
    void showPhotoInImageViewer();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // GROUPINFOWINDOW_H
