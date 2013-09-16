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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMap>
#include <QTimer>
#include <QString>
#include <QMainWindow>

#include "Whatsapp/fmessage.h"

#include "Contacts/contact.h"
#include "Contacts/contactitem.h"
#include "Contacts/contactroster.h"

#include "Dbus/notifyobject.h"

#include "Sql/conversationsdb.h"

#include "Gui/chatdisplayitem.h"
#include "Gui/contactinfowindow.h"
#include "Gui/contactselectionmodel.h"
#include "Gui/chatwindow.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ContactRoster *roster, bool showWhatsNew, QWidget *parent = 0);
    ~MainWindow();

    void setActiveChat(QString jid);
    bool hasChatOpen(Contact& c);
    void statusChanged(FMessage message);

protected:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void statusChanged(QString jid, QString status);
    void createChatWindow();
    void messageReceived(FMessage message);
    void mediaUploadAccepted(FMessage message);
    void updateGroup(Group& group, bool notify);
    void sendMessageFromChat(FMessage message);
    void sendSetGroupSubjectFromChat(QString gjid,QString newSubject);
    void requestLeaveGroupFromChat(QString gjid);
    void deleteChat(QObject *obj);
    void messageStatusUpdate(FMessage message);
    void quit();
    void contactSelected(QModelIndex index);
    void showAboutDialog();
    void showGlobalSettingsDialog();
    void showAccountInfoWindow();
    void showProfileWindow();
    void showNetworkUsageWindow();
    void showCreateGroupWindow();
    void lockModeChanged(QString lockMode);
    void available(QString jid, bool online);
    void available(QString jid, qint64 lastSeen);
    void composing(QString jid, QString media);
    void paused(QString jid);
    void showDonate();
    void groupLeft(QString jid);
    void requestSync();
    void contextMenu(QPoint p);
    void mute(QString jid, bool muted, qint64 muteExpireTimestamp);
    void updateTimestamps();
    void updatePhoto(Contact& c);
    void requestChangeUserName(QString newUserName);
    void requestSetPhoto(QString jid, QImage photo);
    void requestChangeStatus(QString status);
    void requestPhotoRefresh(QString jid, QString photoId, bool largeFormat);
    void requestContactStatus(QString jid);
    void sendVoiceNotePlayed(FMessage message);
    void showStatusWindow();
    void photoReceived(Contact& c, QImage photo, QString photoId);
    void viewContact(Contact *c);
    void requestCreateGroupChat(QImage photo, QString subject,QStringList participants);
    void viewGroup(Group *g);
    void groupParticipant(QString gjid, QString participant);
    void removeParticipant(QString gjid, QString participant);
    void requestGetParticipants(QString gjid);
    void requestAddGroupParticipant(QString gjid, QString participant);
    void requestRemoveGroupParticipant(QString gjid, QString participant);
    void groupError(QString gjid);
    void showBlockedContactsWindow();
    void blockOrUnblockContact(QString jid, bool blocked);
    void refreshPrivacyList();

private:
    Ui::MainWindow *ui;
    QMap<QString,ChatWindow *> chatWindowList;
    QMap<QString,ChatDisplayItem *> lastContactsList;
    ContactRoster *roster;
    bool isScreenLocked;
    NotifyObject *notifyObject;
    QMutex createWindowMutex;
    ConversationsDB chatsDB;
    ContactSelectionModel *model;
    QTimer *newDayTimer;

    void loadOpenChats();
    ChatWindow *createChatWindow(Contact& contact, bool show);
    void notify(const Contact& contact,FMessage& message);
    void showWindow(QWidget *window);
    void resetNewDayTimer();

signals:
    void sendMessage(FMessage message);
    void sendNotification(QString name, FMessage message);
    void activeChatChanged(QString jid);
    void sendSetGroupSubject(QString gjid, QString newSubject);
    void requestLeaveGroup(QString jid);
    void changeStatus(QString newStatus);
    void changeUserName(QString newUserName);
    void sync();
    void sendRightButtonClicked(const QPoint& p);
    void settingsUpdated();
    void queryLastOnline(QString jid);
    void subscribe(QString jid);
    void unsubscribe(QString jid);
    void photoRequest(QString jid, QString expectedPhotoId, bool largeFormat);
    void requestStatus(QString jid);
    void setPhoto(QString jid, QImage photo);
    void createGroupChat(QImage photo, QString subject, QStringList participants);
    void getParticipants(QString gjid);
    void groupParticipantAdded(QString gjid, QString jid);
    void previewPhotoReceived(QString jid);
    void largePhotoReceived(QString jid, QImage photo, QString photoId);
    void onlineStatusChanged(QString jid);
    void userStatusUpdated(QString jid);
    void groupSubjectUpdated(QString gjid);
    void addGroupParticipant(QString gjid, QString participant);
    void removeGroupParticipant(QString gjid, QString participant);
    void groupParticipantRemoved(QString gjid, QString participant);
    void requestPrivacyList();
    void requestBlockOrUnblockContact(QString jid, bool blocked);
    void privacyListRefreshed();
    void voiceNotePlayed(FMessage message);
};

#endif // MAINWINDOW_H
