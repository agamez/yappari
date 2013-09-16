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

#include <QApplication>
#include <QMaemo5InformationBox>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCloseEvent>
#include <QUrl>

#include "Dbus/dbusappletif.h"
#include "Dbus/dbusnokiamcerequestif.h"
#include "Dbus/notifyobject.h"

#include "Contacts/contactitem.h"

#include "aboutdialog.h"
#include "accountinfowindow.h"
#include "blockedcontactswindow.h"
#include "statuswindow.h"
#include "chatdisplaydelegate.h"
#include "globalsettingsdialog.h"
#include "creategroupwindow.h"
#include "groupinfowindow.h"
#include "groupwindow.h"
#include "chatwindow.h"
#include "selectcontactdialog.h"
#include "profilewindow.h"
#include "networkusagewindow.h"
#include "whatsnewwindow.h"

#include "globalconstants.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Whatsapp/util/utilities.h"

#include "Sql/chatlogger.h"

#include "client.h"

MainWindow::MainWindow(ContactRoster *roster, bool showWhatsNew, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Configure the main listView

    model = new ContactSelectionModel(ui->listView);
    model->setSortRole(Qt::UserRole + 2);
    ui->listView->setModel(model);
    ui->listView->installEventFilter(this);

    ChatDisplayDelegate *delegate = new ChatDisplayDelegate(ui->listView);

    ui->listView->setItemDelegate(delegate);

    connect(ui->createChatButton,SIGNAL(pressed()),this,SLOT(createChatWindow()));
    connect(ui->actionSettings,SIGNAL(triggered()),this,SLOT(showGlobalSettingsDialog()));
    connect(ui->actionStatus,SIGNAL(triggered()),this,SLOT(showStatusWindow()));
    connect(ui->actionQuit,SIGNAL(triggered()),this,SLOT(quit()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(showAboutDialog()));
    connect(ui->actionDonate,SIGNAL(triggered()),this,SLOT(showDonate()));
    connect(ui->actionSync,SIGNAL(triggered()),this,SLOT(requestSync()));
    connect(ui->actionProfile,SIGNAL(triggered()),
            this,SLOT(showProfileWindow()));
    connect(ui->actionAccountInfo,SIGNAL(triggered()),
            this,SLOT(showAccountInfoWindow()));
    connect(ui->actionNetworkUsage,SIGNAL(triggered()),
            this,SLOT(showNetworkUsageWindow()));
    connect(ui->actionCreateGroup,SIGNAL(triggered()),
            this,SLOT(showCreateGroupWindow()));
    connect(ui->actionBlockedContacts,SIGNAL(triggered()),
            this,SLOT(showBlockedContactsWindow()));

    connect(ui->listView,SIGNAL(clicked(QModelIndex)),
            this,SLOT(contactSelected(QModelIndex)));

    connect(this,SIGNAL(sendRightButtonClicked(QPoint)),
            this,SLOT(contextMenu(QPoint)));

    this->roster = roster;

    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_DeleteOnClose,false);
    setAttribute(Qt::WA_QuitOnClose,false);

    isScreenLocked = false;

    notifyObject = new NotifyObject(this);
    connect(this,SIGNAL(sendNotification(QString,FMessage)),
            notifyObject,SLOT(sendNotify(QString,FMessage)));

    newDayTimer = new QTimer(this);
    connect(newDayTimer,SIGNAL(timeout()),this,SLOT(updateTimestamps()));
    resetNewDayTimer();

    loadOpenChats();

    // WhatsNew Window

    if (showWhatsNew)
    {
        WhatsNewWindow *window = new WhatsNewWindow(this);

        window->setAttribute(Qt::WA_Maemo5StackedWindow);
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->setWindowFlags(window->windowFlags() | Qt::Window);
        window->show();
    }

}

MainWindow::~MainWindow()
{
    delete ui;

    Utilities::logData("MainWindow destroyed");
}

void MainWindow::loadOpenChats()
{
    QList<ConversationsDBEntry> list = chatsDB.getAllChats();

    QListIterator<ConversationsDBEntry> i(list);

    while(i.hasNext())
    {
        ConversationsDBEntry entry = i.next();

        Utilities::logData("Entry retrieved: " + entry.jid);

        // Verify if the contact is still in roster
        // It should be unless the database has been corrupted somehow
        if (roster->isContactInRoster(entry.jid))
        {
            // Create the open chats item
            Contact& contact = roster->getContact(entry.jid);
            ChatDisplayItem *item = new ChatDisplayItem(&contact);
            lastContactsList.insert(entry.jid,item);
            item->muted = entry.muted;
            item->muteExpireTimestamp = entry.muteExpireTimestamp;
            model->appendRow(item);

            // Set the last line logged in the item
            FMessage msg = ChatLogger::lastMessage(entry.jid);
            item->updateData(msg);
        }
        else
        {
            // Database is corrupted.  Delete that chat

            Utilities::logData("ERROR: Jid " + entry.jid + " not found in roster");
            chatsDB.removeChat(entry.jid);
        }
    }
    model->sort(0,Qt::DescendingOrder);
}

void MainWindow::createChatWindow()
{
    if (!roster->size())
    {
        QMaemo5InformationBox::information(this,"Contacts haven't been synchronized yet.",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else
    {
        SelectContactDialog selectContactDialog(roster,this);

        connect(&selectContactDialog,SIGNAL(showContactInfo(Contact*)),
                this,SLOT(viewContact(Contact*)));

        if (selectContactDialog.exec() == QDialog::Accepted)
        {
            Contact& contact = selectContactDialog.getSelectedContact();

            createChatWindow(contact, true);
        }
    }
}

ChatWindow *MainWindow::createChatWindow(Contact& contact, bool show)
{
    // Just open one window at a time to avoid duplicates
    createWindowMutex.lock();

    ChatWindow *chat;

    QString jid = contact.jid;

    if (contact.type == Contact::TypeGroup)
        Utilities::logData("Group");
    else
        Utilities::logData("Contact");

    if (!chatWindowList.contains(jid))
    {
        Utilities::logData("There's no previous chat window");

        if (contact.type == Contact::TypeContact)
            chat = new ChatWindow(&contact,this);
        else
            chat = new GroupWindow((Group *)&contact,this);

        chatWindowList.insert(jid,chat);

        if (!lastContactsList.contains(jid))
        {
            // This is a new chat and it is not in the
            // open chats list

            // Create the open chats item
            ChatDisplayItem *item = new ChatDisplayItem(&contact);
            lastContactsList.insert(jid,item);
            model->appendRow(item);

            // Set the last line logged in the item
            FMessage msg = chat->lastMessage();
            item->updateData(msg);

            // Store this chat in the DB
            ConversationsDBEntry entry;
            entry.jid = jid;
            entry.muted = false;
            entry.muteExpireTimestamp = 0;
            chatsDB.createChat(entry);

            if (contact.type == Contact::TypeContact)
                emit subscribe(jid);
        }
        else
        {
            // This chat is in the open chats list

            // Configure mute settings
            ChatDisplayItem *item = lastContactsList.value(jid);
            if (item->muted)
                chat->setMute(item->muteExpireTimestamp);
        }

        chat->setAttribute(Qt::WA_DeleteOnClose);

        if (!show)
                Utilities::logData("Chat window will not be shown");
        if (show)
        {
            Utilities::logData("Showing chat window");
            qint64 startTime = QDateTime::currentMSecsSinceEpoch();
            chat->show();
            qint64 endTime = QDateTime::currentMSecsSinceEpoch() - startTime;
            Utilities::logData("Chat window showed " + QString::number(endTime) +
                               " milliseconds.");
        }

        connect(chat,SIGNAL(sendMessage(FMessage)),
                this,SLOT(sendMessageFromChat(FMessage)));

        connect(chat,SIGNAL(destroyed(QObject *)),
                this,SLOT(deleteChat(QObject *)));

        connect(chat,SIGNAL(mute(QString,bool,qint64)),
                this,SLOT(mute(QString,bool,qint64)));

        connect(chat,SIGNAL(blockOrUnblockContact(QString,bool)),
                this,SLOT(blockOrUnblockContact(QString,bool)));

        connect(chat,SIGNAL(photoRefresh(QString,QString,bool)),
                this,SLOT(requestPhotoRefresh(QString,QString,bool)));

        connect(chat,SIGNAL(requestStatus(QString)),
                this,SLOT(requestContactStatus(QString)));

        connect(chat,SIGNAL(voiceNotePlayed(FMessage)),
                this,SLOT(sendVoiceNotePlayed(FMessage)));

        if (contact.type == Contact::TypeGroup)
        {
            GroupWindow *groupChat = (GroupWindow *) chat;
            connect(groupChat,SIGNAL(changeSubject(QString,QString)),
                    this,SLOT(sendSetGroupSubjectFromChat(QString,QString)));

            connect(groupChat,SIGNAL(requestLeaveGroup(QString)),
                    this,SLOT(requestLeaveGroupFromChat(QString)));

            connect(groupChat,SIGNAL(getParticipants(QString)),
                    this,SLOT(requestGetParticipants(QString)));
        }
        else
            emit queryLastOnline(jid);
    }
    else
    {
        Utilities::logData("There's an open chat window for " + jid);

        chat = chatWindowList.value(jid);
        chat->show();
        chat->activateWindow();
        emit activeChatChanged(jid);
    }

    Utilities::logData("Setting chat window title");
    chat->setContact(&contact);
    Utilities::logData("Window title set");

    // Release the lock
    createWindowMutex.unlock();

    Utilities::logData("createChatWindow(): finished");
    return chat;
}

void MainWindow::deleteChat(QObject *obj)
{
    QString jid = ((ChatWindow *)obj)->getContact().jid;

    chatWindowList.remove(jid);
}

void MainWindow::setActiveChat(QString jid)
{
    if (chatWindowList.contains(jid))
    {
        ChatWindow *chat = chatWindowList.value(jid);
        chat->show();
        chat->activateWindow();
        emit activeChatChanged(jid);
    }
}

void MainWindow::updateGroup(Group &group, bool notify)
{
    ChatDisplayItem *item;

    if (!lastContactsList.contains(group.jid))
    {
        // New group
        item = new ChatDisplayItem(&group);
        lastContactsList.insert(group.jid,item);
        model->appendRow(item);

        // Refresh the last logged line
        ChatLogger logger;
        logger.init(group.jid);
        FMessage msg = logger.lastMessage();
        item->updateData(msg);

        // Store this chat in the DB
        ConversationsDBEntry entry;
        entry.jid = group.jid;
        entry.muted = false;
        entry.muteExpireTimestamp = 0;
        chatsDB.createChat(entry);

        QMaemo5InformationBox::information(this,
                                           (group.author == Client::myJid) ?
                                               "Group " + group.name + " created successfully" :
                                               "You've been added to group " + group.name);
    }
    else
    {
        // New subject.
        item = lastContactsList.value(group.jid);
        item->updateData();
    }

    /*

    // This section updates the contact name of the subject owner

    Contact& contact = roster->getContact(group.subjectOwner);

    if (contact.name.isEmpty())
    {
        if (!group.subjectOwnerName.isEmpty())
            contact.name = group.subjectOwnerName;
        else
            contact.name = contact.phone;
    }

    group.subjectOwnerName = contact.name;
    */


    if (chatWindowList.contains(group.jid))
    {
        // Chat is open, update subject
        GroupWindow *groupWindow = (GroupWindow *) chatWindowList.value(group.jid);
        groupWindow->setGroup(&group,notify);
    }

    emit groupSubjectUpdated(group.jid);
}

void MainWindow::messageReceived(FMessage message)
{
    ChatWindow *chat;

    // Contact who sent the message
    Contact& contact = roster->getContact(
                (!message.remote_resource.isEmpty()
                            ? message.remote_resource
                            : message.key.remote_jid
                ));

    // Update contact alias
    QString prevAlias = contact.alias;
    contact.alias = (!message.notify_name.isEmpty())
                        ? message.notify_name
                        : ((!contact.name.isEmpty())
                           ? contact.name
                           : contact.phone);

    // Update contact name if is empty with the alias
    if (contact.name.isEmpty() ||
            (!contact.fromAddressBook && contact.name != contact.alias))
    {
        contact.name = contact.alias;
        roster->updateName(&contact);
    }

    // This is to avoid constant DB access that
    // might be pretty slow in a phone
    if (contact.alias != prevAlias)
        roster->updateAlias(&contact);

    // If this is from a contact we don't have in the roster
    // or if the show nicknames settings is enabled then
    // let's use the alias
    message.notify_name = (Client::showNicknames || !contact.fromAddressBook)
                            ? contact.alias
                            : contact.name;

    // Try to retrieve a new photo for this contact only
    // if there's no entry in the last contact list
    if (!lastContactsList.contains(contact.jid))
        emit photoRequest(contact.jid, contact.photoId, false);

    // Check if a window for this chat is already open
    if (!chatWindowList.contains(message.key.remote_jid))
    {
        // Create a new window for this chat
        if (!message.remote_resource.isEmpty())
        {
            Group& group = roster->getGroup(message.key.remote_jid);
            chat = createChatWindow(group, Client::popupOnFirstMessage);
        }
        else
        {
            chat = createChatWindow(contact, Client::popupOnFirstMessage);
        }

        notify(contact,message);
    }
    else
    {
        chat = chatWindowList.value(message.key.remote_jid);
        if (isScreenLocked || !chat->isActiveWindow())
            notify(contact,message);
    }

    chat->messageReceived(message);
    ChatDisplayItem *item = lastContactsList.value(message.key.remote_jid);
    item->updateData(message);
    model->sort(0,Qt::DescendingOrder);
}

void MainWindow::mediaUploadAccepted(FMessage message)
{
    // Check if a window for this chat is already open
    if (chatWindowList.contains(message.remote_resource))
    {
        ChatWindow *chat = chatWindowList.value(message.remote_resource);
        chat->mediaUploadAccepted(message);
    }

}

void MainWindow::sendMessageFromChat(FMessage message)
{
    // Update the open conversations list only on text
    // and multimedia messages
    if (message.type == FMessage::BodyMessage ||
        message.type == FMessage::MediaMessage)
    {
        ChatDisplayItem *item = lastContactsList.value(message.key.remote_jid);
        item->updateData(message);
        model->sort(0,Qt::DescendingOrder);
    }
    emit sendMessage(message);
}

void MainWindow::sendSetGroupSubjectFromChat(QString gjid, QString newSubject)
{
    emit sendSetGroupSubject(gjid, newSubject);
}

void MainWindow::messageStatusUpdate(FMessage message)
{
    if (!chatWindowList.contains(message.key.remote_jid))
    {
        Utilities::logData("No chat for " + message.key.remote_jid + " found.");
        Contact& contact = roster->getContact(message.key.remote_jid);

        ChatLogger logger;
        logger.init(contact.jid);
        logger.updateLoggedMessage(message);
        //chat = createChat(contact);
    }
    else
    {
       Utilities::logData("Chat for " + message.key.remote_jid + " found.");
       ChatWindow *chat = chatWindowList.value(message.key.remote_jid);
       chat->messageStatusUpdate(message);
    }
}

void MainWindow::quit()
{
    Utilities::logData("User has quit the application");

    // TODO: Clean disconnect here and close all sockets

    qApp->quit();
}

void MainWindow::contactSelected(QModelIndex index)
{
    QString jid = index.data(Qt::UserRole + 1).toString();

    createChatWindow(roster->getContact(jid), true);

    ui->listView->clearSelection();
}

void MainWindow::notify(const Contact& contact, FMessage& message)
{
    if (lastContactsList.contains(message.key.remote_jid))
    {
        ChatDisplayItem *item = lastContactsList.value(message.key.remote_jid);
        if (item->muted && item->muteExpireTimestamp != 0 &&
                item->muteExpireTimestamp < QDateTime::currentMSecsSinceEpoch())
        {
            item->muted = false;
            if (chatWindowList.contains(message.key.remote_jid))
            {
                ChatWindow *chat = chatWindowList.value(message.key.remote_jid);
                chat->unmute();
            }
        }

        QString name;


        if (!item->muted)
        {
            name = (Client::showNicknames || !contact.fromAddressBook)
                    ? (contact.alias.isEmpty() ? contact.phone : contact.alias)
                    : contact.name;

            emit sendNotification(name,message);
        }
    }
}

void MainWindow::showAboutDialog()
{
    AboutDialog dialog(this);

    dialog.exec();
}

void MainWindow::lockModeChanged(QString lockMode)
{
    //isScreenLocked = (lockMode == "locked") ? true : false;
    isScreenLocked = (lockMode == "on") ? false : true;

    if (!isScreenLocked)
    {
        QList<ChatWindow*> values = chatWindowList.values();
        for (int i = 0; i < values.size(); i++)
            if (values.at(i)->isActiveWindow())
            {
                Utilities::logData("Closing outstanding notifications for " +
                                   values.at(i)->getContact().jid);
                notifyObject->closeNotification(values.at(i)->getContact().jid);
                return;
            }
    }
}

void MainWindow::available(QString jid, bool online)
{
    // This is a way to do it.  Another way is to connect the signal
    // available() in client.cpp to a method in ContactRoster.

    if (roster->isContactInRoster(jid))
    {
        Contact& c = roster->getContact(jid);
        c.setOnlineStatus(online);

        if (chatWindowList.contains(jid))
        {
            ChatWindow *chat = chatWindowList.value(jid);
            chat->available(online,c.lastSeen);
        }

        roster->updateLastSeen(&c);
    }

    emit onlineStatusChanged(jid);
}

void MainWindow::available(QString jid, qint64 lastSeen)
{
    if (roster->isContactInRoster(jid))
    {
        Contact& c = roster->getContact(jid);
        c.lastSeen = lastSeen;

        if (chatWindowList.contains(jid))
        {
            ChatWindow *chat = chatWindowList.value(jid);
            chat->available(c.isOnline,c.lastSeen);
        }

        roster->updateLastSeen(&c);
    }

    emit onlineStatusChanged(jid);
}


void MainWindow::composing(QString jid, QString media)
{
    if (chatWindowList.contains(jid))
    {
        ChatWindow *chat = chatWindowList.value(jid);
        chat->composing(media);
    }
}


void MainWindow::paused(QString jid)
{
    if (chatWindowList.contains(jid))
    {
        ChatWindow *chat = chatWindowList.value(jid);
        chat->paused();
    }
}

void MainWindow::showDonate()
{
    QDesktopServices::openUrl(QUrl(DONATE_URL));
}

void MainWindow::requestLeaveGroupFromChat(QString gjid)
{
    emit requestLeaveGroup(gjid);
}

void MainWindow::groupLeft(QString gjid)
{
    // Delete group from open chat windows
    if (chatWindowList.contains(gjid))
    {
        Utilities::logData("Removing open group window: " + gjid);
        GroupWindow *group = (GroupWindow *) chatWindowList.value(gjid);
        chatWindowList.remove(gjid);
        group->close();
    }

    // Delete group from last chats list
    if (lastContactsList.contains(gjid))
    {
        Utilities::logData("Removing group from conversations list: " + gjid);
        ChatDisplayItem *item = lastContactsList.value(gjid);
        lastContactsList.remove(gjid);
        model->removeRow(item->row());
        ui->listView->clearSelection();
    }

    // Delete group from open chats DB
    chatsDB.removeChat(gjid);

    roster->deleteGroup(gjid);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

void MainWindow::showGlobalSettingsDialog()
{
    GlobalSettingsDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted)
    {
        Client::port = dialog.getPort();
        Client::sync = dialog.getSync();

        Client::showNicknames = dialog.getShowNicknames();
        Client::showNumbers = dialog.getShowNumbers();
        Client::popupOnFirstMessage = dialog.getPopupOnFirstMessage();
        Client::automaticDownloadBytes = dialog.getAutomaticDownloadBytes();
        Client::importMediaToGallery = dialog.getImportMediaToGallery();
        Client::syncFreq = dialog.getSyncFrequency();
        Client::startOnBoot = dialog.getStartOnBoot();
        Client::enterIsSend = dialog.getEnterIsSend();
        Client::voiceCodec = dialog.getVoiceCodec();

        emit settingsUpdated();
    }
}

void MainWindow::requestSync()
{
    if (Client::connectionStatus == Client::LoggedIn)
    {
        if (Client::isSynchronizing)
            QMaemo5InformationBox::information(this,"Contacts are already being synchronized.");
        else
        {
            QMaemo5InformationBox::information(this,"Contacts will be synchronized in the background.");
            emit sync();
        }
    }
    else
    {
        QMaemo5InformationBox::information(this,"You need to be logged in to synchronize your contacts",
                                           QMaemo5InformationBox::NoTimeout);
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        // A little hack for a Qt bug workaround
        QString text = statusBar()->currentMessage();

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*> (event);
        sendRightButtonClicked(mouseEvent->globalPos());

        statusBar()->showMessage(text);
        return true;
    }
    else
        return QObject::eventFilter(obj, event);
}

void MainWindow::contextMenu(QPoint p)
{
    Utilities::logData("Context Menu requested");

    QModelIndex index = ui->listView->indexAt(ui->listView->viewport()->mapFromGlobal(p));

    if (index.isValid())
    {
        QString jid = index.data(Qt::UserRole + 1).toString();
        Utilities::logData("Index jid: " + jid);

        Contact& c = roster->getContact(jid);

        QMenu *menu = new QMenu(this);
        QString viewStr = (c.type == Contact::TypeContact)
                ? "View Contact" : "Group Info";
        QString deleteStr = (c.type == Contact::TypeContact)
                ? "Delete Chat" : "Leave Group";

        QAction *viewContactAction = new QAction(viewStr,this);
        menu->addAction(viewContactAction);
        QAction *deleteChat = new QAction(deleteStr,this);
        menu->addAction(deleteChat);

        QAction *action = menu->exec(p);
        if (action == viewContactAction)
        {
            if (c.type == Contact::TypeContact)
            {
                Contact& c = roster->getContact(jid);
                viewContact(&c);
            }
            else if (c.type == Contact::TypeGroup)
            {
                Group& g = roster->getGroup(jid);
                viewGroup(&g);
            }
        }
        else if (action == deleteChat)
        {
            QMessageBox msg(this);

            deleteStr = (c.type == Contact::TypeContact)
                    ? "delete this chat" : "leave this group";

            msg.setText("Are you sure you want to " + deleteStr + "?");
            msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

            if (msg.exec() == QMessageBox::Yes)
            {
                if (c.type == Contact::TypeGroup)
                    requestLeaveGroupFromChat(c.jid);
                else
                {
                    if (chatWindowList.contains(jid))
                    {
                        ChatWindow *chat = chatWindowList.value(jid);
                        chat->close();
                        chatWindowList.remove(jid);
                    }

                    model->removeRow(index.row());
                    lastContactsList.remove(jid);
                    chatsDB.removeChat(jid);

                    Contact& c = roster->getContact(jid);
                    if (c.type == Contact::TypeContact)
                        emit unsubscribe(jid);
                }
            }
        }
        Utilities::logData("Exit Menu");
        ui->listView->clearSelection();
    }
}

void MainWindow::mute(QString jid, bool muted, qint64 muteExpireTimestamp)
{
    if (lastContactsList.contains(jid))
    {
        ChatDisplayItem *item = lastContactsList.value(jid);
        item->muted = muted;
        item->muteExpireTimestamp = muteExpireTimestamp;

        ConversationsDBEntry entry;
        entry.jid = jid;
        entry.muted = muted;
        entry.muteExpireTimestamp = muteExpireTimestamp;
        chatsDB.updateChat(entry);
    }
}

bool MainWindow::hasChatOpen(Contact& c)
{
    return lastContactsList.contains(c.jid);
}

void MainWindow::updateTimestamps()
{
    Utilities::logData("updateTimeStamps()");
    newDayTimer->stop();

    // Update Timestamps in the Open Chats list.
    foreach (QString key, lastContactsList.keys())
    {
        ChatDisplayItem *item = lastContactsList.value(key);
        item->updateData();
    }

    // Update Timestamps in open conversation windows
    foreach (QString jid, chatWindowList.keys())
    {
        ChatWindow *chat = chatWindowList.value(jid);
        chat->updateTimestamps();
    }

    resetNewDayTimer();
}

void MainWindow::resetNewDayTimer()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime tomorrow = QDateTime(now.date().addDays(1));
    qint64 nextRunTime = tomorrow.toMSecsSinceEpoch() -
                         now.toMSecsSinceEpoch();

    newDayTimer->start(nextRunTime);

    Utilities::logData("Resetting timestamps at " + QString::number(now.toMSecsSinceEpoch()));
    Utilities::logData("Next run in " + QString::number(nextRunTime));
}

void MainWindow::updatePhoto(Contact& c)
{
    if (lastContactsList.contains(c.jid))
    {
        ChatDisplayItem *item = lastContactsList.value(c.jid);
        item->updatePhoto(&c);
    }

    emit previewPhotoReceived(c.jid);
}

void MainWindow::showAccountInfoWindow()
{
    QMaemo5InformationBox::information(this,
                                       "! ! ! ! A T T E N T I O N ! ! ! !\n\n"\
                                       "YAPPARI DOES NOT CHARGE YOU FOR THE USE OF THIS SOFTWARE\n"\
                                       "YAPPARI AND WHATSAPP INC. ARE NOT RELATED AT ALL!!!\n\n"\
                                       "But WhatsApp Inc. does and will charge you for the use of this service\n"
                                       "If your account is expired you have to pay THEM to renew your account\nThere's nothing Yappari can do about it\n",
                                       QMaemo5InformationBox::NoTimeout);

    AccountInfoWindow *window = new AccountInfoWindow(this);
    showWindow(window);
}

void MainWindow::showNetworkUsageWindow()
{
    NetworkUsageWindow *window = new NetworkUsageWindow(this);
    showWindow(window);
}

void MainWindow::showProfileWindow()
{
    ProfileWindow *window = new ProfileWindow(this);

    connect(window,SIGNAL(photoSelected(QString, QImage)),
            this,SLOT(requestSetPhoto(QString, QImage)));

    connect(window,SIGNAL(changeUserName(QString)),
            this,SLOT(requestChangeUserName(QString)));

    showWindow(window);
}

void MainWindow::showStatusWindow()
{
    StatusWindow *window = new StatusWindow(this);

    connect(window,SIGNAL(changeStatus(QString)),
            this,SLOT(requestChangeStatus(QString)));

    showWindow(window);
}


void MainWindow::showWindow(QWidget *window)
{
    window->setAttribute(Qt::WA_Maemo5StackedWindow);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowFlags(window->windowFlags() | Qt::Window);
    window->show();
}

void MainWindow::requestChangeUserName(QString newUserName)
{
    emit changeUserName(newUserName);
}

void MainWindow::requestChangeStatus(QString status)
{
    emit changeStatus(status);
}

void MainWindow::requestPhotoRefresh(QString jid, QString photoId, bool largeFormat)
{
    emit photoRequest(jid, photoId, largeFormat);
}

void MainWindow::requestContactStatus(QString jid)
{
    emit requestStatus(jid);
}

void MainWindow::sendVoiceNotePlayed(FMessage message)
{
    emit voiceNotePlayed(message);
}

void MainWindow::photoReceived(Contact &c, QImage photo, QString photoId)
{
    // I don't think this is needed
    if (chatWindowList.contains(c.jid))
    {
        ChatWindow *chat = (ChatWindow *) chatWindowList.value(c.jid);
        chat->photoReceivedHandler(photo, photoId);
    }

    // Emit the signal.  If there's a group/contact info window open it will catch it
    emit largePhotoReceived(c.jid, photo, photoId);

}

void MainWindow::statusChanged(FMessage message)
{
    statusChanged(message.key.remote_jid, QString::fromUtf8(message.data.constData()));
}

void MainWindow::statusChanged(QString jid, QString status)
{
    if (chatWindowList.contains(jid))
    {
        ChatWindow *chat = chatWindowList.value(jid);
        chat->statusChanged(status);
    }

    emit userStatusUpdated(jid);
}


void MainWindow::viewContact(Contact *c)
{
    ContactInfoWindow *contactInfoWindow = new ContactInfoWindow(c,this);

    emit requestStatus(c->jid);
    emit queryLastOnline(c->jid);

    showWindow(contactInfoWindow);

}

void MainWindow::requestSetPhoto(QString jid, QImage photo)
{
    emit setPhoto(jid, photo);
}

void MainWindow::showCreateGroupWindow()
{

    if (Client::connectionStatus != Client::LoggedIn)
        QMaemo5InformationBox::information(this,"You need to be logged in to create groups",
                                           QMaemo5InformationBox::NoTimeout);
    else
    {
        CreateGroupWindow *window = new CreateGroupWindow(roster,this);

        connect(window,SIGNAL(createGroupChat(QImage,QString,QStringList)),
                this,SLOT(requestCreateGroupChat(QImage,QString,QStringList)));

        showWindow(window);
    }
}

void MainWindow::requestCreateGroupChat(QImage photo, QString subject, QStringList participants)
{
    emit createGroupChat(photo, subject,participants);
}

void MainWindow::viewGroup(Group *g)
{
    GroupInfoWindow *groupInfoWindow = new GroupInfoWindow(g, roster, this);

    emit photoRequest(g->jid, g->photoId, false);
    emit getParticipants(g->jid);

    showWindow(groupInfoWindow);
}

void MainWindow::groupParticipant(QString gjid, QString participant)
{
    emit groupParticipantAdded(gjid, participant);
}

void MainWindow::requestGetParticipants(QString gjid)
{
    emit getParticipants(gjid);
}

void MainWindow::requestAddGroupParticipant(QString gjid, QString participant)
{
    emit addGroupParticipant(gjid, participant);
}

void MainWindow::removeParticipant(QString gjid, QString participant)
{
    emit groupParticipantRemoved(gjid, participant);
}

void MainWindow::requestRemoveGroupParticipant(QString gjid, QString participant)
{
    emit removeGroupParticipant(gjid, participant);
}

void MainWindow::groupError(QString gjid)
{
    if (chatWindowList.contains(gjid))
    {
        GroupWindow *chat = (GroupWindow *)chatWindowList.value(gjid);
        chat->groupError();
    }
}

void MainWindow::showBlockedContactsWindow()
{
    emit requestPrivacyList();

    BlockedContactsWindow *blockedContactsWindow =
            new BlockedContactsWindow(roster, this);

    connect(this,SIGNAL(privacyListRefreshed()),
            blockedContactsWindow,SLOT(showBlocked()));

    showWindow(blockedContactsWindow);
}

void MainWindow::blockOrUnblockContact(QString jid, bool blocked)
{
    emit requestBlockOrUnblockContact(jid, blocked);
}

void MainWindow::refreshPrivacyList()
{
    emit privacyListRefreshed();
}

