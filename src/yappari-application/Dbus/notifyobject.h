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

#ifndef NOTIFYTHREAD_H
#define NOTIFYTHREAD_H

#include <QThread>
#include <QHash>

#include "Whatsapp/fmessage.h"

#include "Dbus/dbusnotificationsif.h"
#include "Dbus/dbusnokiahildonsvndif.h"

class MainWindow;

class NotifyObject : public QObject
{
    Q_OBJECT
public:
    explicit NotifyObject(MainWindow *mainWin);

signals:

public slots:
    void sendNotify(QString contact, FMessage message);
    void closeNotification(QString jid);
    void notifyCallback(uint id, QString action);
    void deleteNotify(uint id,uint reason);


private:
    DBusNotificationsIf *notifierBus;
    DBusNokiaHildonSVNDIf *sndVibBus;
    MainWindow *mainWin;
    QHash<uint,QString> notificationList;
};

class NotifyData
{
public:
    MainWindow *mainWindow;
    QString jid;
};


#endif // NOTIFYTHREAD_H
