/* Copyright 2012 Naikel Aparicio. All rights reserved.
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
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Eeli Reilin.
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

protected:
    void closeEvent(QCloseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

public slots:
    void createChatWindow();
    void messageReceived(FMessage message);
    void mediaUploadAccepted(FMessage message);
    void groupNewSubject(QString from, QString author, QString authorName,
                         QString newSubject, QString creation);
    void groupInfoFromList(QString from, QString author, QString newSubject, QString creation,
                           QString subjectOwner, QString subjectTimestamp);
    void sendMessageFromChat(FMessage message);
    void sendSetGroupSubjectFromChat(QString gjid,QString newSubject);
    void requestLeaveGroupFromChat(QString gjid);
    void deleteChat(QObject *obj);
    void messageStatusUpdate(FMessage message);
    void quit();
    void contactSelected(QModelIndex index);
    void showAboutDialog();
    void showGlobalSettingsDialog();
    void showChangeStatusDialog();
    void showChangeUserNameDialog();
    void lockModeChanged(QString lockMode);
    void available(QString jid, bool online);
    void available(QString jid, qint64 lastSeen);
    void composing(QString jid);
    void paused(QString jid);
    void showDonate();
    void leaveGroup(QString jid);
    void requestSync();
    void contextMenu(QPoint p);
    void mute(QString jid, bool muted, qint64 muteExpireTimestamp);
    void updateTimestamps();

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
    void updateGroup(Group& group,bool notify);
    void notify(const Contact& contact,FMessage& message);
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

};

#endif // MAINWINDOW_H
