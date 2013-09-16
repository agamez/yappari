/**
 * Copyright (C) 2013 Naikel Aparicio. All rights reserved.
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

#include "Whatsapp/util/datacounters.h"

#include "Whatsapp/warequest.h"
#include "Whatsapp/connection.h"
#include "Whatsapp/phonereg.h"

#include "Contacts/contactroster.h"
#include "Contacts/contactsyncer.h"

#include "Gui/mainwindow.h"

/**
    @class      Client

    @brief      This is the main class of Yappari.

                It provides interaction between the GUI and the Connection object
                that keeps the connection to the WhatsApp servers.

                The object from this class is always running in background, and also
                interacts with the status menu applet.
*/

class Client : public QObject
{
    Q_OBJECT

public:

    /** ***********************************************************************
     ** Enumerations
     **/

    // Status of the connection to the WhatsApp servers
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

    // Frequency of the address book synchronization
    enum SyncFrequencies {
        onConnect,
        onceADay,
        onceAWeek,
        onceAMonth
    };


    /** ***********************************************************************
     ** Public members
     **/

    // Network data counters
    static DataCounters dataCounters;

    // Status of the connection
    static ConnectionStatus connectionStatus;

    // Context to make calls to libosso functions
    static osso_context_t *osso_context;

    // Message number sequence
    static quint64 seq;

    // Global settings
    static QSettings *settings;

    // Background color of own self text
    static QString mycolor;

    // Nicknames color
    static QString nickcolor;

    // Text color
    static QString textcolor;

    // Own JID
    static QString myJid;

    // Country code
    static QString cc;

    // Phone number in local format (without the country code)
    static QString number;

    // Phone number in international format (with the country code)
    static QString phoneNumber;

    // User name or alias
    static QString userName;

    // User password
    static QString password;

    // IMEI
    static QString imei;

    // IMSI
    static QString imsi;

    // Account creation timestamp
    static QString creation;

    // Account expiration timestamp
    static QString expiration;

    // Account kind (free/paid)
    static QString kind;

    // Account status (active/expired)
    static QString accountstatus;

    // Port to connect to WhatsApp Servers (443/5222)
    static quint16 port;

    // User Status
    static QString myStatus;

    // Show nicknames in conversations instead of their names from the address book
    static bool showNicknames;

    // Show users phone numbers in select contact dialog
    static bool showNumbers;

    // Pop up conversation windows when first message is received
    static bool popupOnFirstMessage;

    // Automatic download of media if less than this number of bytes
    static int automaticDownloadBytes;

    // Import media into gallery
    static bool importMediaToGallery;

    // Last time address book synchronizarion was performed
    static qint64 lastSync;

    // What's new window magic number
    static qint64 whatsNew;

    // Sync setting (on/intl/off)
    static QString sync;

    // Sync frequency (see SyncFrequencies enum above)
    static int syncFreq;

    // Start Yappari on boot
    static bool startOnBoot;

    // Android password encryption method enabled
    static bool android;

    // Is a synchronization active?
    static bool isSynchronizing;

    // Enter is Send
    static bool enterIsSend;

    // Voice codec (amr/aac)
    static QString voiceCodec;

    // Main GUI window
    static MainWindow *mainWin;

    // Roster
    static ContactRoster *roster;


    /** ***********************************************************************
     ** Constructors and destructors
     **/

    // Create a Client object
    explicit Client(bool minimized, QObject *parent = 0);

    // Destroy a Client object
    ~Client();


public slots:

    /** ***********************************************************************
     ** Public slots methods
     **/

    /** ***********************************************************************
     ** Settings methods
     **/

    // Update the global settings file with the current settings
    void updateSettings();


    /** ***********************************************************************
     ** Network Detection methods
     **/

    // Handles a network change
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
    void syncHttpError(int error);
    void syncSslError();
    void syncFinished();
    void syncProgress(int progress);
    void photoRefresh(QString jid, QString expectedPhotoId, bool largeFormat);
    void photoDeleted(QString jid, QString alias);
    void photoIdReceived(QString jid, QString name, QString pictureId);
    void photoReceived(QString from, QByteArray data,
                       QString photoId, bool largeFormat);
    void requestContactStatus(QString jid);
    void setPhoto(QString jid, QImage image);
    void requestPresenceSubscription(QString jid);
    void requestPresenceUnsubscription(QString jid);
    void createGroupChat(QImage photo, QString subject,QStringList participants);
    void groupInfoFromList(QString id, QString from, QString author,
                           QString newSubject, QString creation,
                           QString subjectOwner, QString subjectTimestamp);
    void groupNewSubject(QString from, QString author, QString authorName,
                         QString newSubject, QString creation);
    void getParticipants(QString gjid);
    void groupUser(QString gjid, QString jid);
    void sendAddGroupParticipant(QString gjid, QString jid);
    void sendRemoveGroupParticipant(QString gjid, QString jid);
    void groupAddUser(QString gjid ,QString jid);
    void groupRemoveUser(QString gjid ,QString jid);
    void requestPrivacyList();
    void setPrivacyList();
    void blockOrUnblockContact(QString jid, bool blocked);
    void privacyListReceived(QStringList list);
    void sendVoiceNotePlayed(FMessage message);


public Q_SLOTS:
    void ShowWindow();
    bool isRunning();

private:
    Connection *connection;
    QTcpSocket *socket;
    QNetworkConfigurationManager *manager;
    QString lastError;
    QString activeNetworkID;
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

    // Groups waiting to be created
    QHash<QString,Group *> groups;

    // Registration
    bool isRegistered;

    // Online
    bool isOnline;

    /** ***********************************************************************
     ** Private methods
     **/

    /** ***********************************************************************
     ** Settings
     **/

    // Reads the global settings and store them in the public static members
    void readSettings();

    bool isNetworkAvailable();
    void updateActiveNetworkID();
    void showStatus(QString status);
    void startRegistration();
    void connectToServer();
    QString parseStatus();
    void createMyJidContact();

signals:
Q_SIGNALS:
    void tklock_mode_ind(const QString &lock_mode);


};

#endif // CLIENTTHREAD_H
