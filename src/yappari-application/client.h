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
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Eeli Reilin.
 */

#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include "Dbus/gtkbindings-new.h"
#include <libosso.h>
#include <libosso-abook/osso-abook.h>

#include <QThread>
#include <QString>
#include <QTcpSocket>
#include <QQueue>
#include <QTimer>
#include <QMutex>
#include <QAbstractSocket>
#include <QMainWindow>
#include <QSettings>
#include <QNetworkConfigurationManager>

#include "Dbus/dbusifadaptor.h"
#include "Dbus/dbusappletif.h"
#include "Dbus/dbusnokiamcesignalif.h"

#include "Whatsapp/warequest.h"
#include "Whatsapp/connection.h"
#include "Whatsapp/phonereg.h"

#include "Whatsapp/util/datacounters.h"

#include "Contacts/contactroster.h"
#include "Contacts/contactsyncer.h"

#include "Gui/mainwindow.h"

// Q_DECLARE_METATYPE(QAbstractSocket::SocketError)

class Client : public QObject
{
    Q_OBJECT
public:

    enum ConnectionStatus {
        Unknown,
        WaitingForConnection,
        Connecting,
        Connected,
        LoggedIn,
        LoginFailure,
        Disconnected,
        Registering,
        RegistrationFailed
    };

    enum SyncFrequencies {
        onConnect,
        onceADay,
        onceAWeek,
        onceAMonth
    };

    static DataCounters dataCounters;

    static ConnectionStatus connectionStatus;

    static osso_context_t *osso_context;

    // Settings
    static QSettings *settings;

    // Sequence
    static quint64 seq;

    // Colors
    static QString mycolor;
    static QString nickcolor;
    static QString textcolor;

    // Account information
    static QString myJid;
    static QString cc;
    static QString number;
    static QString phoneNumber;
    static QString password;
    static QString userName;
    static QString imei;

    static QString creation;
    static QString expiration;
    static QString kind;
    static QString accountstatus;

    // Port
    static quint16 port;

    // Status
    static QString myStatus;

    // Nicknames in conversations
    static bool showNicknames;

    // Phone numbers in select contact dialog
    static bool showNumbers;

    // Pop up conversation windows when first message is received
    static bool popupOnFirstMessage;

    // Automatic download of media if less than this number of bytes
    static int automaticDownloadBytes;

    // Import media into gallery
    static bool importMediaToGallery;

    // Last time photo refresh was performed
    static qint64 lastSync;

    // What's new window
    static qint64 whatsNew;

    // Sync
    static QString sync;

    // Sync frequency
    static int syncFreq;

    // Start on boot
    static bool startOnBoot;

    // Android password encryption method
    static bool android;

    static bool isSynchronizing;

    explicit Client(bool minimized, QObject *parent = 0);
    ~Client();

private:
    MainWindow *mainWin;
    Connection *connection;
    QTcpSocket *socket;
    QNetworkConfigurationManager *manager;
    QString lastError;
    QString activeNetworkID;
    ContactRoster *roster;
    DBusIfAdaptor *ifAdaptor;
    DBusNokiaMCESignalIf *mceAdaptor;
    DBusAppletIf *applet;
    ContactSyncer *syncer;
    qint64 lastCountersWrite;

    // Timers
    QTimer *keepAliveTimer;
    QTimer *pendingMessagesTimer;
    QTimer *retryLoginTimer;

    // Queues
    QQueue<FMessage> pendingMessagesQueue;

    // Mutex
    QMutex connectionMutex;
    QMutex pendingMessagesMutex;

    // Registration
    bool isRegistered;

    // Online
    bool isOnline;

    void readSettings();
    bool isNetworkAvailable();
    void updateActiveNetworkID();
    void showStatus(QString status);
    void startRegistration();
    void connectToServer();
    QString parseStatus();

signals:

public slots:
    void updateSettings();
    void networkStatusChanged(bool isOnline);
    void networkConfigurationChanged(QNetworkConfiguration);
    void verifyAndConnect();
    void connected();
    void error(QAbstractSocket::SocketError socketError);
    void connectionActivated();
    void connectionDeactivated();
    void connectionClosed();
    void read();
    void keepAlive();
    void queueMessage(FMessage message);
    void sendMessagesInQueue();
    void updateStatus();
    void registrationSuccessful(QVariantMap result);
    void sendSetGroupSubject(QString gjid, QString subject);
    void requestLeaveGroup(QString jid);
    void requestQueryLastOnline(QString jid);
    void userStatusUpdated(FMessage message);
    void changeStatus(QString newStatus);
    void changeUserName(QString newUserName);
    void synchronizeContacts();
    void syncFinished();
    void syncProgress(int progress);
    void photoRefresh(QString jid, QString expectedPhotoId, bool largeFormat);
    void photoDeleted(QString jid);
    void photoIdReceived(QString jid, QString pictureId);
    void photoReceived(QString from, QByteArray data,
                       QString photoId, bool largeFormat);
    void requestContactStatus(QString jid);
    void setPhoto(QImage image);


public Q_SLOTS:
    void ShowWindow();
    bool isRunning();

Q_SIGNALS:
    void tklock_mode_ind(const QString &lock_mode);
};

#endif // CLIENTTHREAD_H
