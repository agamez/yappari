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

#include <QDateTime>
#include <QString>

#include "Gui/mainwindow.h"

#include "Whatsapp/util/utilities.h"

#include "notifyobject.h"
#include "globalconstants.h"

NotifyObject::NotifyObject(MainWindow *mainWin) : QObject(mainWin)
{
    this->mainWin = mainWin;

    // DBus interfaces

    QDBusConnection dbus = QDBusConnection::sessionBus();

    notifierBus = new DBusNotificationsIf(HD_NOTIFICATION_MANAGER_DBUS_NAME,
                                          HD_NOTIFICATION_MANAGER_DBUS_PATH,
                                          dbus,this);

    sndVibBus = new DBusNokiaHildonSVNDIf(HD_SV_NOTIFICATION_DAEMON_DBUS_NAME,
                                          HD_SV_NOTIFICATION_DAEMON_DBUS_PATH,
                                          dbus,this);

    connect(notifierBus,SIGNAL(ActionInvoked(uint,QString)),
            this,SLOT(notifyCallback(uint,QString)));

    connect(notifierBus,SIGNAL(NotificationClosed(uint,uint)),
            this,SLOT(deleteNotify(uint,uint)));
}


void NotifyObject::notifyCallback(uint id, QString action)
{
    Q_UNUSED(action);
    if (notificationList.contains(id))
        mainWin->setActiveChat(notificationList.value(id));

    // mainWin->setActiveChat(action);
}

void NotifyObject::deleteNotify(uint id, uint)
{
    if (notificationList.contains(id))
        notificationList.remove(id);
}


void NotifyObject::sendNotify(QString contact, FMessage message)
{
    qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    Utilities::logData("Starting sending notification");

    QVariantMap notifyHints;
    notifyHints.insert("category","yappari-message");
    notifyHints.insert("message-thread",message.key.remote_jid);
    notifyHints.insert("presistent",true);

    QString text;
    if (message.type == FMessage::MediaMessage)
    {
        if (message.live)
            text = "[voice note]";
        else
            text = "[" + message.getMediaWAType() + "]";
    }
    else
        text = QString::fromUtf8(message.data.constData());

    uint reply = notifierBus->Notify(YAPPARI_APPLICATION_NAME,0,YAPPARI_NOTIFICATION_ICON,
                                     Utilities::removeEmoji(contact),
                                     Utilities::removeEmoji(text),
                                     QStringList(),notifyHints,-1);

    Utilities::logData("New notification created with id " + QString::number(reply));

    notificationList.insert(reply,message.key.remote_jid);

    QVariantMap eventHints;
    eventHints.insert("category","chat-message");
    eventHints.insert("vibra","PatternChatAndEmail");

    sndVibBus->PlayEvent(eventHints,YAPPARI_APPLICATION_NAME);

    qint64 endTime = QDateTime::currentDateTime().toMSecsSinceEpoch() - startTime;
    Utilities::logData("Finished sending notifications in " + QString::number(endTime) +
                       " milliseconds.");
}

void NotifyObject::closeNotification(QString jid)
{
    QList<uint> keys = notificationList.keys();

    for (int i = 0; i < keys.size(); i++)
    {
        uint id = keys.at(i);
        if (notificationList[id] == jid)
            notifierBus->CloseNotification(id);
    }

}


