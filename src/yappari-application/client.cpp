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

#include <mce/dbus-names.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include <QDateTime>
#include <QStatusBar>
#include <QSystemInfo>
#include <QMessageBox>
#include <QMaemo5InformationBox>

#include "version.h"
#include "client.h"
#include "globalconstants.h"

#include "Gui/mainwindow.h"
#include "Gui/registrationwindow.h"
#include "Gui/changeusernamedialog.h"

#include "Whatsapp/fmessage.h"
#include "Whatsapp/ioexception.h"
#include "Whatsapp/protocolexception.h"
#include "Whatsapp/util/utilities.h"
#include "Whatsapp/util/qtmd5digest.h"
#include "Whatsapp/util/datetimeutilities.h"

/** ***********************************************************************
 ** Local definitions
 **/

// Maximum size of a profile picture
#define MAX_PROFILE_PICTURE_SIZE    0xfc00

/** ***********************************************************************
 ** Static global members
 **/

// Network data counters
DataCounters Client::dataCounters;

// Status of the connection
Client::ConnectionStatus Client::connectionStatus;

// Context to make calls to libosso functions
osso_context_t *Client::osso_context;

// Message number sequence
quint64 Client::seq;

// Global settings
QSettings *Client::settings;

// Background color of own self text
QString Client::mycolor;

// Nicknames color
QString Client::nickcolor;

// Text color
QString Client::textcolor;

// Own JID
QString Client::myJid;

// Country code
QString Client::cc;

// Phone number in local format (without the country code)
QString Client::number;

// Phone number in international format (with the country code)
QString Client::phoneNumber;

// User name or alias
QString Client::userName;

// User password
QString Client::password;

// IMEI
QString Client::imei;

// IMSI
QString Client::imsi;

// Account creation timestamp
QString Client::creation;

// Account expiration timestamp
QString Client::expiration;

// Account kind (free/paid)
QString Client::kind;

// Account status (active/expired)
QString Client::accountstatus;

// Port to connect to WhatsApp Servers (443/5222)
quint16 Client::port;

// User status
QString Client::myStatus;

// Show nicknames in conversations instead of their names from the address book
bool Client::showNicknames;

// Show users phone numbers in select contact dialog
bool Client::showNumbers;

// Pop up conversation windows when first message is received
bool Client::popupOnFirstMessage;

// Automatic download of media if less than this number of bytes
int Client::automaticDownloadBytes;

// Import media into gallery
bool Client::importMediaToGallery;

// Last time address book synchronizarion was performed
qint64 Client::lastSync;

// What's new window magic number
qint64 Client::whatsNew;

// Sync setting (on/intl/off)
QString Client::sync;

// Sync frequency
int Client::syncFreq;

// Start Yappari on boot
bool Client::startOnBoot;

// Android password encryption method enabled
bool Client::android;

// Is a synchronization active?
bool Client::isSynchronizing;

// Enter is Send
bool Client::enterIsSend;

// Voice codec (amr/aac)
QString Client::voiceCodec;

// Main GUI window
MainWindow *Client::mainWin;

// Roster
ContactRoster *Client::roster;

/**
    Constructs a Client object.

    @param minimized    Starts Yappari minimized if true.
    @param parent       QObject parent.
*/
Client::Client(bool minimized, QObject *parent) : QObject(parent)
{
    // Debug info start
    QString version = FULL_VERSION;
    Utilities::logData("Yappari " + version);
    Utilities::logData("Yappari console debug start");

    // Read Settings
    readSettings();

    // Initialization of sequence for messages IDs
    seq = 0;

    // Contacts roster
    osso_context = osso_initialize(YAPPARI_APPLICATION_NAME,
                                   VERSION, FALSE, NULL);
    int argc = 0;
    char **argv;
    osso_abook_init(&argc,&argv,osso_context);
    roster = new ContactRoster(this);

    // Create our own contact if it doesn't exist
    // or if it's invalid
    createMyJidContact();

    bool showWhatsNew = false;

    // Show Whatsnew window?
    if (whatsNew != MAGIC_NUMBER)
    {
        settings->setValue(SETTINGS_WHATSNEW,MAGIC_NUMBER);
        showWhatsNew = true;
    }

    // Create main window application
    mainWin = new MainWindow(roster, showWhatsNew);

    connect(mainWin,SIGNAL(settingsUpdated()),
            this,SLOT(updateSettings()));

    connect(mainWin,SIGNAL(sendMessage(FMessage)),
            this,SLOT(queueMessage(FMessage)));

    connect(mainWin,SIGNAL(sendSetGroupSubject(QString,QString)),
            this,SLOT(sendSetGroupSubject(QString,QString)));

    connect(mainWin,SIGNAL(requestLeaveGroup(QString)),
            this,SLOT(requestLeaveGroup(QString)));

    connect(mainWin,SIGNAL(changeStatus(QString)),
            this,SLOT(changeStatus(QString)));

    connect(mainWin,SIGNAL(changeUserName(QString)),
            this,SLOT(changeUserName(QString)));

    connect(mainWin,SIGNAL(sync()),
            this,SLOT(synchronizeContacts()));

    connect(mainWin,SIGNAL(queryLastOnline(QString)),
            this,SLOT(requestQueryLastOnline(QString)));

    connect(mainWin,SIGNAL(subscribe(QString)),
            this,SLOT(requestPresenceSubscription(QString)));

    connect(mainWin,SIGNAL(unsubscribe(QString)),
            this,SLOT(requestPresenceUnsubscription(QString)));

    connect(mainWin,SIGNAL(photoRequest(QString,QString,bool)),
            this,SLOT(photoRefresh(QString,QString,bool)));

    connect(mainWin,SIGNAL(requestStatus(QString)),
            this,SLOT(requestContactStatus(QString)));

    connect(mainWin,SIGNAL(setPhoto(QString,QImage)),
            this,SLOT(setPhoto(QString,QImage)));

    connect(mainWin,SIGNAL(createGroupChat(QImage,QString,QStringList)),
            this,SLOT(createGroupChat(QImage,QString,QStringList)));

    connect(mainWin,SIGNAL(getParticipants(QString)),
            this,SLOT(getParticipants(QString)));

    connect(mainWin, SIGNAL(addGroupParticipant(QString,QString)),
            this,SLOT(sendAddGroupParticipant(QString,QString)));

    connect(mainWin, SIGNAL(removeGroupParticipant(QString,QString)),
            this,SLOT(sendRemoveGroupParticipant(QString,QString)));

    connect(mainWin, SIGNAL(requestPrivacyList()),
            this, SLOT(requestPrivacyList()));

    connect(mainWin, SIGNAL(requestBlockOrUnblockContact(QString,bool)),
            this, SLOT(blockOrUnblockContact(QString,bool)));

    connect(mainWin,SIGNAL(voiceNotePlayed(FMessage)),
            this, SLOT(sendVoiceNotePlayed(FMessage)));

    if (!minimized)
        mainWin->show();

    // Status bar
    QStatusBar *bar = mainWin->statusBar();
    bar->setStyleSheet("QStatusBar { font-size:18px; }");
    bar->showMessage(FULL_VERSION);
    bar->show();

    // Create DBus server
    ifAdaptor = new DBusIfAdaptor(this);
    QDBusConnection sessionbus = QDBusConnection::sessionBus();
    bool ret = sessionbus.registerService(YAPPARI_SERVICE);
    ret = sessionbus.registerObject(YAPPARI_OBJECT, this);

    // Listen for screen lock changes
    QDBusConnection systembus = QDBusConnection::systemBus();
    mceAdaptor = new DBusNokiaMCESignalIf(MCE_SERVICE,
                                          MCE_SIGNAL_PATH,
                                          systembus,
                                          this);
    //connect(mce,SIGNAL(tklock_mode_ind(QString)),
    //        mainWin,SLOT(lockModeChanged(QString)));

    connect(mceAdaptor,SIGNAL(display_status_ind(QString)),
            mainWin,SLOT(lockModeChanged(QString)));

    // Show Status Menu Applet
    applet = new DBusAppletIf(APPLET_SERVICE,APPLET_OBJECT,sessionbus,this);
    applet->ShowApplet();

    // Timers
    keepAliveTimer = new QTimer(this);
    pendingMessagesTimer = new QTimer(this);
    retryLoginTimer = new QTimer(this);

    connect(keepAliveTimer, SIGNAL(timeout()), this, SLOT(keepAlive()));
    connect(pendingMessagesTimer, SIGNAL(timeout()), this, SLOT(sendMessagesInQueue()));
    connect(retryLoginTimer, SIGNAL(timeout()), this, SLOT(verifyAndConnect()));

    if (isRegistered)
    {
        // Not connected
        connectionStatus = WaitingForConnection;

        while (this->userName.isEmpty())
        {
            ChangeUserNameDialog dialog(mainWin);

            if (dialog.exec() == QDialog::Accepted)
            {
                this->userName = dialog.getUserName();
                settings->setValue(SETTINGS_USERNAME,userName);
            }
        }
    }
    else
        connectionStatus = Disconnected;

    isSynchronizing = false;
    updateStatus();

    // Network Manager Configutarion
    manager = new QNetworkConfigurationManager(this);
    connect(manager,SIGNAL(onlineStateChanged(bool)),
            this,SLOT(networkStatusChanged(bool)));

    connect(manager,SIGNAL(configurationChanged(QNetworkConfiguration)),
            this,SLOT(networkConfigurationChanged(QNetworkConfiguration)));

#ifdef Q_WS_SCRATCHBOX
    isOnline = true;
#else
    isOnline = manager->isOnline();
#endif
    networkStatusChanged(isOnline);
}

/**
    Destroys a Connection object
*/
Client::~Client()
{
    // Update data counters
    dataCounters.writeCounters();

    Utilities::logData("Application destroyed");
    applet->HideApplet();
}

/** ***********************************************************************
 ** Settings methods
 **/

/**
    Reads the global settings and store them in the public static members.
*/
void Client::readSettings()
{
    // Check the directories are created
    QDir home = QDir::home();
    if (!home.exists(home.path() + LOGS_DIR))
        home.mkpath(home.path() + LOGS_DIR);

    if (!home.exists(home.path() + CONF_DIR))
        home.mkpath(home.path() + CONF_DIR);

    this->settings = new QSettings(SETTINGS_ORGANIZATION,SETTINGS_APPLICATION,this);

    // Need registration verification
    isRegistered = ((settings->contains(SETTINGS_REGISTERED) &&
        settings->value(SETTINGS_REGISTERED).toBool())) ? true : false;

    // Colors
    this->mycolor = settings->value(SETTINGS_MYCOLOR).toString();
    this->textcolor = settings->value(SETTINGS_TEXTCOLOR).toString();
    this->nickcolor = settings->value(SETTINGS_NICKCOLOR).toString();

    // Status
    this->myStatus = settings->value(SETTINGS_STATUS).toString();

    // Sync
    this->sync = settings->value(SETTINGS_SYNC).toString();
    this->syncFreq = settings->value(SETTINGS_SYNC_FREQ,QVariant(DEFAULT_SYNC_FREQ)).toInt();

    // Android
    this->android = settings->value(SETTINGS_ANDROID).toBool();

    // Account
    this->cc = settings->value(SETTINGS_CC).toString();
    this->number = settings->value(SETTINGS_NUMBER).toString();
    this->phoneNumber = settings->value(SETTINGS_PHONENUMBER).toString();
    this->password = settings->value(SETTINGS_PASSWORD).toString();
    this->myJid = phoneNumber + "@s.whatsapp.net";
    this->userName = settings->value(SETTINGS_USERNAME).toString();
    this->imei = settings->value(SETTINGS_IMEI).toString();
    this->imsi = settings->value(SETTINGS_IMSI).toString();
    this->creation = settings->value(SETTINGS_CREATION).toString();
    this->kind = settings->value(SETTINGS_KIND).toString();
    this->expiration = settings->value(SETTINGS_EXPIRATION).toString();
    this->accountstatus = settings->value(SETTINGS_ACCOUNTSTATUS).toString();

    // Port
    this->port = settings->value(SETTINGS_PORT,QVariant(DEFAULT_PORT)).toInt();

    // Show nicknames in conversations
    this->showNicknames = settings->value(SETTINGS_SHOW_NICKNAMES,
                                          QVariant(DEFAULT_SHOW_NICKNAMES)).toBool();

    // Show phone numbers in select contact dialog
    this->showNumbers = settings->value(SETTINGS_SHOW_NUMBERS,
                                          QVariant(DEFAULT_SHOW_NUMBERS)).toBool();

    // Pop up conversation window when first message is received
    this->popupOnFirstMessage = settings->value(SETTINGS_POPUP_ON_FIRST_MESSAGE,
                                                  QVariant(DEFAULT_POPUP_ON_FIRST_MESSAGE)).toBool();

    // Automatic download bytes
    this->automaticDownloadBytes = settings->value(SETTINGS_AUTOMATIC_DOWNLOAD,
                                                   QVariant(DEFAULT_AUTOMATIC_DOWNLOAD)).toInt();

    // Automatic import of downloaded media into gallery
    this->importMediaToGallery = settings->value(SETTINGS_IMPORT_TO_GALLERY,
                                                  QVariant(DEFAULT_IMPORT_TO_GALLERY)).toBool();

    // Last Synchronization
    this->lastSync = settings->value(SETTINGS_LAST_SYNC).toLongLong();

    // Start on Boot
    this->startOnBoot = settings->value(SETTINGS_START_ON_BOOT,
                                        QVariant(DEFAULT_START_ON_BOOT)).toBool();

    // What's New Window
    this->whatsNew = settings->value(SETTINGS_WHATSNEW).toLongLong();

    // Enter is Send
    this->enterIsSend = settings->value(SETTINGS_ENTER_IS_SEND,
                                        QVariant(DEFAULT_ENTER_IS_SEND)).toBool();

    // Voice codec (amr/aac)
    this->voiceCodec = settings->value(SETTINGS_VOICE_CODEC,
                                       QVariant(DEFAULT_VOICE_CODEC)).toString();

    // Read counters
    dataCounters.readCounters();
    lastCountersWrite = QDateTime::currentMSecsSinceEpoch();
}

/**
    Update the global settings file with the current settings
*/
void Client::updateSettings()
{
    // We just update the settings that are configurable
    // through GUI

    settings->setValue(SETTINGS_SYNC,sync);
    settings->setValue(SETTINGS_PORT,port);
    settings->setValue(SETTINGS_SHOW_NICKNAMES,showNicknames);
    settings->setValue(SETTINGS_SHOW_NUMBERS,showNumbers);
    settings->setValue(SETTINGS_POPUP_ON_FIRST_MESSAGE,popupOnFirstMessage);
    settings->setValue(SETTINGS_AUTOMATIC_DOWNLOAD,automaticDownloadBytes);
    settings->setValue(SETTINGS_IMPORT_TO_GALLERY,importMediaToGallery);
    settings->setValue(SETTINGS_SYNC_FREQ,syncFreq);
    settings->setValue(SETTINGS_START_ON_BOOT,startOnBoot);
    settings->setValue(SETTINGS_ENTER_IS_SEND,enterIsSend);
    settings->setValue(SETTINGS_VOICE_CODEC,voiceCodec);

    QDir home = QDir::home();
    QString startFile = home.path() + START_FILE;

    if (!startOnBoot && home.exists(startFile))
        home.remove(startFile);
    else if (startOnBoot && !home.exists(startFile))
    {
        QFile file(startFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            file.close();
    }
}

/** ***********************************************************************
 ** Network detection methods
 **/

/**
    Handles a network change

    @param isOnline     True if an network connection is currently available
*/
void Client::networkStatusChanged(bool isOnline)
{
    QString status = (isOnline) ? "Online" : "Offline";
    Utilities::logData("Network connection changed: " + status);

    this->isOnline = isOnline;

    if (!isOnline)
    {
        activeNetworkID.clear();
        if (connectionStatus == Connected || connectionStatus == LoggedIn)
            connectionClosed();
    }
    else
    {
        updateActiveNetworkID();
        if (connectionStatus == Disconnected || connectionStatus == WaitingForConnection)
            verifyAndConnect();
    }
}

void Client::networkConfigurationChanged(QNetworkConfiguration conf)
{
    if (conf.state() == QNetworkConfiguration::Active)
    {
        Utilities::logData("Network activated: " + conf.identifier());
    }
    else
    {
        Utilities::logData("Network deactivated: " + conf.identifier());
    }

    if (isOnline && (connectionStatus == Connected || connectionStatus == LoggedIn) &&
            conf.state() == QNetworkConfiguration::Active &&
            activeNetworkID != conf.identifier())
    {
        isOnline = false;
        QTimer::singleShot(0,this,SLOT(connectionClosed()));
        QTimer::singleShot(3000,this,SLOT(connectionActivated()));
    }
}

void Client::connectionActivated()
{
    networkStatusChanged(true);
}

void Client::connectionDeactivated()
{
    networkStatusChanged(false);
}

bool Client::isNetworkAvailable()
{
    return isOnline;
}

void Client::updateActiveNetworkID()
{
#ifdef Q_WS_SCRATCHBOX
    activeNetworkID = "Scratchbox";
#else
    if (isOnline)
    {
        // Let's save the current configuration identifier
        QList<QNetworkConfiguration> activeConfigs = manager->allConfigurations(QNetworkConfiguration::Active);
        for (int i = 0; i < activeConfigs.size(); i++)
        {
            QNetworkConfiguration conf = activeConfigs.at(i);
            if (conf.state() == QNetworkConfiguration::Active)
            {
                activeNetworkID = conf.identifier();
                Utilities::logData("Current active connection: " + activeNetworkID);
                return;
            }
        }
    }
#endif
}

void Client::startRegistration()
{
    connectionMutex.lock();
    connectionStatus = Registering;
    updateStatus();

    RegistrationWindow *regWindow = new RegistrationWindow(mainWin);

    connect(regWindow,SIGNAL(accept(QVariantMap)),
            this,SLOT(registrationSuccessful(QVariantMap)));

    regWindow->setAttribute(Qt::WA_Maemo5StackedWindow);
    regWindow->setAttribute(Qt::WA_DeleteOnClose);
    regWindow->setWindowFlags(regWindow->windowFlags() | Qt::Window);
    regWindow->show();
}

void Client::registrationSuccessful(QVariantMap result)
{
    cc = result["cc"].toString();
    number = result["number"].toString();
    phoneNumber = result["login"].toString();
    password = result["pw"].toString();
    myJid = phoneNumber + "@s.whatsapp.net";
    isRegistered = true;

    settings->setValue(SETTINGS_REGISTERED,isRegistered);
    settings->setValue(SETTINGS_NUMBER,number);
    settings->setValue(SETTINGS_CC,cc);
    settings->setValue(SETTINGS_PHONENUMBER,phoneNumber);
    settings->setValue(SETTINGS_PASSWORD,password);

    connectionStatus = Disconnected;
    updateStatus();

    connectionMutex.unlock();

    while (this->userName.isEmpty())
    {
        ChangeUserNameDialog dialog(mainWin);

        if (dialog.exec() == QDialog::Accepted)
        {
            this->userName = dialog.getUserName();
            settings->setValue(SETTINGS_USERNAME,userName);
        }
    }

    networkStatusChanged(isNetworkAvailable());
}

void Client::verifyAndConnect()
{
    QSystemDeviceInfo deviceInfo(this);
#ifdef Q_WS_SCRATCHBOX
    QString imsi = "000000000000000";
#else
    QString imsi = deviceInfo.imsi();
#endif

    if (!imsi.isEmpty())
    {
        if (!this->imsi.isEmpty() && this->imsi != imsi)
        {
            // SIM Card changed
            QMessageBox msg(mainWin);

            msg.setText("A new SIM Card was detected.\n"
                        "Do you want to register the new number?");
            msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

            if (msg.exec() == QMessageBox::Yes)
            {
                isRegistered = false;
            }
        }

        if (this->imsi != imsi)
        {
            this->imsi = imsi;
            settings->setValue(SETTINGS_IMSI,imsi);
        }
    }

    // Verify if the user is registered
    if (!isRegistered)
        startRegistration();
    else
        connectToServer();

    connectionMutex.unlock();

}

void Client::connectToServer()
{
    connectionMutex.lock();

    if (connectionStatus == Connected || connectionStatus == Connecting ||
        connectionStatus == Registering)
    {
        connectionMutex.unlock();
        return;
    }

    // Verify there's an available connection

    if (!isNetworkAvailable())
    {
        Utilities::logData("No network available. Waiting for a connection...");
        connectionStatus = WaitingForConnection;
        updateStatus();
        connectionMutex.unlock();
        return;
    }

    connectionStatus = Connecting;
    updateStatus();

    // If there's a network mode change it will never reach this point
    // so it's safe to unlock de mutex here
    connectionMutex.unlock();

    // There's a connection available, now connect
    QString host = SERVER_DOMAIN;

    Utilities::logData("Connecting to " + host + ":" + QString::number(port) + "...");

    socket = new QTcpSocket(this);
    socket->connectToHost(host,port);

    // qRegisterMetaType<QAbstractSocket::SocketError>("SocketError");
    // connect(socket,SIGNAL(hostFound()),this,SLOT(desktopFound()));
    connect(socket,SIGNAL(connected()),this,SLOT(connected()));
    connect(socket,SIGNAL(error(QAbstractSocket::SocketError)),
            this,SLOT(error(QAbstractSocket::SocketError)));
    connect(socket,SIGNAL(disconnected()),this,SLOT(connectionClosed()));
}

void Client::connected()
{
    // Set SO_KEEPALIVE option
    socket->setSocketOption(QAbstractSocket::KeepAliveOption,1);
    int fd = socket->socketDescriptor();

    // Set TCP_KEEPIDLE option
    int optval = 300;
    socklen_t optlen = sizeof(optval);

    setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &optval, optlen);

    connectionStatus = Connected;
    updateStatus();
    Utilities::logData("Connected successfully");

    // QString password = Utilities::getChatPassword();

    QByteArray password = (this->password.isEmpty())
                                ? Utilities::getChatPassword().toUtf8()
                                : QByteArray::fromBase64(this->password.toUtf8());

    // Utilities::logData("Password: " + password);

    connection = new Connection(socket,JID_DOMAIN,RESOURCE,phoneNumber,
                                userName,password,&dataCounters,this);

    QByteArray nextChallenge = QByteArray::fromBase64(settings->value(SETTINGS_NEXTCHALLENGE).toByteArray());
    settings->remove(SETTINGS_NEXTCHALLENGE);

    // Utilities::logData("Challenge: " + QString::fromLatin1(nextChallenge.toHex()));

    try
    {
        // Never send previous challenge to avoid expiration of it
        nextChallenge.clear();

        connection->login(nextChallenge);

        settings->setValue(SETTINGS_NEXTCHALLENGE,
                                   QString::fromUtf8(connection->nextChallenge.toBase64()));

        settings->setValue(SETTINGS_CREATION, connection->creation);
        settings->setValue(SETTINGS_EXPIRATION, connection->expiration);
        settings->setValue(SETTINGS_KIND, connection->kind);
        settings->setValue(SETTINGS_ACCOUNTSTATUS, connection->accountstatus);
    }
    catch (IOException &e)
    {
        lastError = e.toString();
        Utilities::logData("connected(): There was an IO Exception: " + lastError);
        connectionClosed();
        showStatus(lastError + " Retrying in 10 seconds..");
        return;
    }
    catch (ProtocolException &e)
    {
        lastError = e.toString();
        Utilities::logData("connected(): There was a Protocol Exception: " + lastError);
        connectionClosed();
        showStatus("Protocol Exception");
        return;
    }
    catch (LoginException &e)
    {
        connectionStatus = LoginFailure;
        lastError = e.toString();
        Utilities::logData("connected(): There was a Login Exception: " + lastError);
        isRegistered = false;
        connectionClosed();
        return;
    }

    connectionStatus = LoggedIn;
    updateStatus();

    // Shouldn't these be in a function instead?

    connect(socket,SIGNAL(readyRead()),this,SLOT(read()));

    connect(connection,SIGNAL(timeout()),this,SLOT(connectionClosed()));

    connect(connection,SIGNAL(groupNewSubject(QString,QString,QString,QString,QString)),
            this,SLOT(groupNewSubject(QString,QString,QString,QString,QString)));

    connect(connection,SIGNAL(groupInfoFromList(QString,QString,QString,QString,
                                                QString,QString,QString)),
            this,SLOT(groupInfoFromList(QString,QString,QString,QString,
                                        QString,QString,QString)));

    connect(connection,SIGNAL(messageReceived(FMessage)),
            mainWin,SLOT(messageReceived(FMessage)));

    connect(connection,SIGNAL(messageStatusUpdate(FMessage)),
            mainWin,SLOT(messageStatusUpdate(FMessage)));

    connect(connection,SIGNAL(available(QString,bool)),
            mainWin,SLOT(available(QString,bool)));

    connect(connection,SIGNAL(composing(QString,QString)),
            mainWin,SLOT(composing(QString,QString)));

    connect(connection,SIGNAL(paused(QString)),
            mainWin,SLOT(paused(QString)));

    connect(connection,SIGNAL(groupLeft(QString)),
            mainWin,SLOT(groupLeft(QString)));

    connect(connection,SIGNAL(userStatusUpdated(FMessage)),
            this,SLOT(userStatusUpdated(FMessage)));

    connect(connection,SIGNAL(lastOnline(QString,qint64)),
            mainWin,SLOT(available(QString,qint64)));

    connect(connection,SIGNAL(mediaUploadAccepted(FMessage)),
            mainWin,SLOT(mediaUploadAccepted(FMessage)));

    connect(connection,SIGNAL(photoIdReceived(QString,QString,QString)),
            this,SLOT(photoIdReceived(QString,QString,QString)));

    connect(connection,SIGNAL(photoReceived(QString,QByteArray,QString,bool)),
            this,SLOT(photoReceived(QString,QByteArray,QString,bool)));

    connect(connection,SIGNAL(photoDeleted(QString,QString)),
            this,SLOT(photoDeleted(QString,QString)));

    connect(connection,SIGNAL(groupUser(QString,QString)),
            this,SLOT(groupUser(QString,QString)));

    connect(connection,SIGNAL(groupAddUser(QString,QString)),
            this,SLOT(groupAddUser(QString,QString)));

    connect(connection,SIGNAL(groupRemoveUser(QString,QString)),
            this,SLOT(groupRemoveUser(QString,QString)));

    connect(connection,SIGNAL(groupError(QString)),
            mainWin,SLOT(groupError(QString)));

    connect(connection,SIGNAL(privacyListReceived(QStringList)),
            this,SLOT(privacyListReceived(QStringList)));


    // Update participating groups
    connection->updateGroupChats();

    keepAliveTimer->start(MIN_INTERVAL);
    keepAliveTimer->setSingleShot(true);

    pendingMessagesTimer->start(CHECK_QUEUE_INTERVAL);
    pendingMessagesTimer->setSingleShot(true);

    // Set status if hasn't been set before
    if (this->myStatus.isEmpty())
        changeStatus(DEFAULT_STATUS);

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (syncFreq == onConnect ||
            (syncFreq == onceADay && (lastSync + 86400000U) < now) ||
            (syncFreq == onceAWeek && (lastSync + 604800000U) < now) ||
            (syncFreq == onceAMonth && (lastSync + 2419200000U) < now))
    {
        synchronizeContacts();
        lastSync = now;
        settings->setValue(SETTINGS_LAST_SYNC, lastSync);
    }
}

void Client::synchronizeContacts()
{
    // Contacts syncer
    syncer = new ContactSyncer(roster, this);

    connect(syncer,SIGNAL(syncFinished()),this,SLOT(syncFinished()));
    connect(syncer,SIGNAL(progress(int)),this,SLOT(syncProgress(int)));
    connect(syncer,SIGNAL(photoRefresh(QString,QString,bool)),
            this,SLOT(photoRefresh(QString,QString,bool)));

    connect(syncer,SIGNAL(httpError(int)),
            this,SLOT(syncHttpError(int)));

    connect(syncer,SIGNAL(sslError()),
            this,SLOT(synSslError()));

    isSynchronizing = true;
    QTimer::singleShot(0,syncer,SLOT(sync()));

}

void Client::syncHttpError(int error)
{
    QMaemo5InformationBox::information(mainWin,"There has been a fatal error synchronizing contacts.\nWhatsApp Servers down? Error = " +
                                       QString::number(error),
                                       QMaemo5InformationBox::NoTimeout);

    syncFinished();
}

void Client::syncSslError()
{
    QMaemo5InformationBox::information(mainWin,"SSL error while trying to synchronize contacts.\nMaybe bad time & date settings on the phone?",
                                       QMaemo5InformationBox::NoTimeout);

    syncFinished();
}


void Client::syncProgress(int progress)
{
    QString status = parseStatus() + " (Synchronizing contacts ... " +
                     QString::number(progress) + "%)";

    showStatus(status);
}

void Client::syncFinished()
{
    Utilities::logData("Synchronization finished");
    isSynchronizing = false;
    syncer->deleteLater();
    updateStatus();

    // Get all the photo ids
    //ContactList contactList = roster->getContactList();
    //connection->sendGetPhotoIds(contactList.toJidList());
}

void Client::changeStatus(QString newStatus)
{
    FMessage message("s.us",newStatus,"");
    message.type = FMessage::BodyMessage;
    queueMessage(message);
}

void Client::changeUserName(QString newUserName)
{
    userName = newUserName;

    settings->setValue(SETTINGS_USERNAME,userName);

    connection->setNewUserName(userName);
}

void Client::error(QAbstractSocket::SocketError socketError)
{
    IOException e(socketError);
    lastError = e.toString();

    QString s = "There was an IO error: " + lastError;
    Utilities::logData(s);

    connectionClosed();
    showStatus(lastError);
}

void Client::connectionClosed()
{
    connectionMutex.lock();
    Utilities::logData("Connection closed.");

    // Stop all timers
    Utilities::logData("Stopping timers.");
    keepAliveTimer->stop();
    pendingMessagesTimer->stop();

    Utilities::logData("Freeing up the connection.");
    if (connectionStatus == Connected || connectionStatus == LoggedIn ||
            connectionStatus == LoginFailure)
         connection->deleteLater();

    if (connectionStatus != Disconnected)
    {
        Utilities::logData("Freeing up the socket.");
        disconnect(socket,0,0,0);
            socket->disconnectFromHost();
        socket->deleteLater();
    }

    // Sometimes the network is available but there was an error
    // because a DNS problem or can't reach the server at the moment.
    // In these cases the client should retry
    if (isNetworkAvailable())
    {
        connectionStatus = Disconnected;

        if (!isRegistered)
        {
            // Immediately try to re-register
            QTimer::singleShot(0,this,SLOT(verifyAndConnect()));
        }
        else
        {
            Utilities::logData("Yappari will retry the connection in 10 seconds.");
            retryLoginTimer->start(RETRY_LOGIN_INTERVAL);
            retryLoginTimer->setSingleShot(true);
        }
    }
    else
    {
        Utilities::logData("Waiting for a connection.");
        connectionStatus = WaitingForConnection;
    }

    updateStatus();
    connectionMutex.unlock();
}

void Client::read()
{
    connectionMutex.lock();
    while (socket->bytesAvailable() > 0)
    {
        try {
            connection->read();
        }
        catch (IOException &e)
        {
            lastError = e.toString();
            Utilities::logData("read(): There was an IO Exception: " + lastError);
            connectionClosed();
            showStatus(lastError + " Retrying in 10 seconds..");
        }
        catch (ProtocolException &e)
        {
            lastError = e.toString();
            Utilities::logData("read(): There was a Protocol Exception: " + lastError);
            connectionClosed();
            showStatus("Protocol Exception. Retrying in 10 seconds...");
        }
    }

    connectionMutex.unlock();
}

void Client::keepAlive()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 timeElapsed = now - connection->getLastTreeReadTimestamp();

    if (timeElapsed >= MIN_INTERVAL)
    {
        // If we need a resync we'd do that instead of sending a ping
        if ((syncFreq == onceADay && (lastSync + 86400000U) < now) ||
            (syncFreq == onceAWeek && (lastSync + 604800000U) < now) ||
            (syncFreq == onceAMonth && (lastSync + 2419200000U) < now))
        {
            synchronizeContacts();
            lastSync = now;
            settings->setValue(SETTINGS_LAST_SYNC, lastSync);
        }
        else
            // connection->sendNop();
            connection->sendPing();

        keepAliveTimer->start(MIN_INTERVAL);
    }
    else
    {
        keepAliveTimer->start(MIN_INTERVAL - timeElapsed);
    }

    // Write counters to disk if more than 24 hours have passed
    if ((lastCountersWrite + 86400000U) < now)
    {
        dataCounters.writeCounters();
        lastCountersWrite = now;
    }
}

void Client::sendMessagesInQueue()
{
    if (connectionStatus == LoggedIn)
    {

        while (!pendingMessagesQueue.isEmpty())
        {
            pendingMessagesMutex.lock();
            FMessage message = pendingMessagesQueue.dequeue();
            pendingMessagesMutex.unlock();

            // What if connection is not "connected" ?
            // maybe a timer and recall this function?
            // or a sleep?

            connection->sendMessage(message);
        }

        // If we are not logged in the timer will be restarted
        // in the next login
        pendingMessagesTimer->start(CHECK_QUEUE_INTERVAL);
    }
}

void Client::sendSetGroupSubject(QString gjid, QString subject)
{
    if (connectionStatus == LoggedIn)
    {
        Group &g = roster->getGroup(gjid);
        roster->updateSubject(&g);

        connection->sendSetGroupSubject(gjid,subject);
    }

}

void Client::requestLeaveGroup(QString gjid)
{
    if (connectionStatus == LoggedIn)
        connection->sendLeaveGroup(gjid);
}

void Client::requestQueryLastOnline(QString jid)
{
    if (connectionStatus == LoggedIn)
        connection->sendQueryLastOnline(jid);
}

void Client::queueMessage(FMessage message)
{
    pendingMessagesMutex.lock();
    pendingMessagesQueue.enqueue(message);
    pendingMessagesMutex.unlock();

    // EMIT SIGNAL THAT THERE'S SOMETHING IN THE QUEUE HERE
    // OR CREATE A NEW QUEUE CLASS
    // DON'T USE TIMERS <-- good idea I think.
}

void Client::ShowWindow()
{
    mainWin->show();
    mainWin->activateWindow();
}

bool Client::isRunning()
{
    return true;
}

QString Client::parseStatus()
{
    QString status;

    switch(connectionStatus)
    {
        case WaitingForConnection:
            status = "Waiting for connection";
            break;

        case Connecting:
            status = "Connecting";
            break;

        case Connected:
            status = "Connected";
            break;

        case LoggedIn:
            status = "Logged In";
            break;

        case LoginFailure:
            status = "Login Failed";
            break;

        case Disconnected:
            status = "Disconnected";
            break;

        case RegistrationFailed:
            status = "Registration Failed";
            break;

        default:
            status = FULL_VERSION;
            break;
    }

    return status;
}

void Client::updateStatus()
{
    QString status = parseStatus();

    showStatus(status);
    applet->status(status);
}

void Client::showStatus(QString status)
{
    QStatusBar *bar = mainWin->statusBar();
    bar->showMessage(status);
}

void Client::userStatusUpdated(FMessage message)
{
    QString status = QString::fromUtf8(message.data.constData());


    if (message.key.remote_jid == "s.us")
    {
        Contact &c = roster->getContact(myJid);
        c.status = status;
        roster->updateStatus(&c);

        // Keep legacy status
        Utilities::logData("User status confirmed: " + status);
        settings->setValue(SETTINGS_STATUS,status);
        this->myStatus = status;
    }
    else
    {
        int index = message.key.remote_jid.indexOf('@');
        if (index > 0)
        {
            QString jid = message.key.remote_jid.left(index + 1) + "s.whatsapp.net";
            Contact &c = roster->getContact(jid);
            c.status = QString::fromUtf8(message.data);
            message.key.remote_jid = jid;
            mainWin->statusChanged(message);

            roster->updateStatus(&c);
        }
    }
}

void Client::photoIdReceived(QString jid, QString alias, QString photoId)
{
    Contact &c = roster->getContact(jid);

    if (c.photoId != photoId)
    {
        Utilities::logData("Contact " + jid + " has changed his profile photo");

        if (connectionStatus == LoggedIn)
            connection->sendGetPhoto(jid, QString(), false);
    }

    if (!c.alias.isEmpty() && c.alias != alias)
    {
        Utilities::logData("Contact " + jid + " has updated his alias to " + alias);

        c.alias = alias;
        roster->updateAlias(&c);
    }
}

void Client::photoReceived(QString from, QByteArray data,
                           QString photoId, bool largeFormat)
{
    Contact &c = roster->getContact(from);

    if (from == myJid)
    {
        c.photoId = photoId;
        roster->updatePhoto(&c);

        Utilities::logData("Updated my picture photoId: " + c.photoId);
    }
    else if (!data.isEmpty())
    {
        if (!largeFormat)
        {
            c.photo = QImage::fromData(data).scaled(64, 64, Qt::KeepAspectRatio,
                                                   Qt::SmoothTransformation);
            c.photoId = photoId;

            roster->updatePhoto(&c);

            mainWin->updatePhoto(c);

            Utilities::logData("Updated picture of " + from);
        }
        else
        {
            mainWin->photoReceived(c, QImage::fromData(data), photoId);
        }
    }
}

void Client::photoRefresh(QString jid, QString expectedPhotoId, bool largeFormat)
{
    if (connectionStatus == LoggedIn)
        connection->sendGetPhoto(jid, expectedPhotoId, largeFormat);
}

void Client::photoDeleted(QString jid, QString alias)
{
    Contact &c = roster->getContact(jid);

    if (c.photoId != "abook")
    {
        if (c.type == Contact::TypeContact && jid != myJid)
            roster->getPhotoFromAddressBook(&c);
        else
        {
            c.photoId.clear();
            c.photo = QImage();
        }

        roster->updatePhoto(&c);

        if (jid != myJid)
            mainWin->updatePhoto(c);

        Utilities::logData("Deleted photo of contact " + c.jid);
    }

    if (!c.alias.isEmpty() && c.alias != alias)
    {
        Utilities::logData("Contact " + jid + " has updated his alias to " + alias);

        c.alias = alias;
        roster->updateAlias(&c);
    }
}

void Client::requestContactStatus(QString jid)
{
    // This method is obsolete
    /*
    if (connectionStatus == LoggedIn)
        connection->sendGetStatus(jid);
    */

    if (connectionStatus == LoggedIn)
    {
        // Contact syncer
        syncer = new ContactSyncer(roster, this);

        connect(syncer,SIGNAL(syncFinished()),this,SLOT(syncFinished()));

        connect(syncer,SIGNAL(statusChanged(QString,QString)),
                mainWin,SLOT(statusChanged(QString,QString)));

        connect(syncer,SIGNAL(photoRefresh(QString,QString,bool)),
                this,SLOT(photoRefresh(QString,QString,bool)));

        connect(syncer,SIGNAL(httpError(int)),
                this,SLOT(syncHttpError(int)));

        connect(syncer,SIGNAL(sslError()),
                this,SLOT(synSslError()));

        isSynchronizing = true;
        syncer->syncContact(&(roster->getContact(jid)));
    }
}

void Client::setPhoto(QString jid, QImage image)
{
    QByteArray data;
    QByteArray thumbnail;

    if (!image.isNull())
    {
        int quality = 80;
        do
        {
            data.clear();
            QBuffer out(&data);
            out.open(QIODevice::WriteOnly);
            image.save(&out, "JPEG", quality);
            quality -= 10;
        } while ((quality > 10) && data.size() > MAX_PROFILE_PICTURE_SIZE);

        QImage thumb = image.scaled(100, 100, Qt::KeepAspectRatio,
                                    Qt::SmoothTransformation);
        quality = 80;
        do
        {
            thumbnail.clear();
            QBuffer out(&thumbnail);
            out.open(QIODevice::WriteOnly);
            thumb.save(&out, "JPEG", quality);
            quality -= 10;
        } while ((quality > 10) && thumbnail.size() > MAX_PROFILE_PICTURE_SIZE);
    }

    if (connectionStatus == LoggedIn)
        connection->sendSetPhoto(jid, data, thumbnail);
}

void Client::requestPresenceSubscription(QString jid)
{
    if (connectionStatus == LoggedIn)
        connection->sendPresenceSubscriptionRequest(jid);
}

void Client::requestPresenceUnsubscription(QString jid)
{
    if (connectionStatus == LoggedIn)
        connection->sendUnsubscribeHim(jid);
}

void Client::createGroupChat(QImage photo, QString subject, QStringList participants)
{
    qint64 creation = QDateTime::currentMSecsSinceEpoch() / 1000;

    QString id = "create_group_" + QString::number(seq++);

    Group *group = new Group();

    group->subjectOwner = group->author = myJid;
    group->creationTimestamp = group->subjectTimestamp = creation;
    group->name = subject;
    group->photo = photo;

    group->participants = participants;

    groups.insert(id,group);

    if (connectionStatus == LoggedIn)
        connection->sendCreateGroupChat(subject,id);
}

void Client::groupInfoFromList(QString id, QString from, QString author,
                               QString newSubject, QString creation,
                               QString subjectOwner, QString subjectTimestamp)
{
    Group *g;

    if (groups.contains(id))
    {
        Group *group = groups.value(id);
        QString creationStr = QString::number(group->creationTimestamp);

        g = &(roster->getGroup(from, group->author, group->name,
                              creationStr, group->subjectOwner, creationStr));

        if (connectionStatus == LoggedIn)
        {
            connection->sendAddParticipants(from, group->participants);
            if (!group->photo.isNull())
                setPhoto(from,group->photo);
        }

        groups.remove(id);
        delete group;
    }
    else
        g = &(roster->getGroup(from, author, newSubject,
                               creation, subjectOwner, subjectTimestamp));

    mainWin->updateGroup(*g, false);
}

void Client::groupNewSubject(QString from, QString author, QString authorName, QString newSubject, QString creation)
{
    Group &group = roster->getGroup(from);

    group.name = newSubject;
    group.subjectOwner = author;
    group.subjectOwnerName = authorName;
    group.subjectTimestamp = creation.toLongLong();

    roster->updateSubject(&group);

    mainWin->updateGroup(group, true);

    if (group.author.isEmpty())
        connection->updateGroupChats();
}

void Client::getParticipants(QString gjid)
{
    if (connectionStatus == LoggedIn)
        connection->sendGetParticipants(gjid);
}

void Client::groupUser(QString gjid, QString jid)
{
    Group &group = roster->getGroup(gjid);

    if (!group.containsParticipant(jid))
    {
        roster->addGroupParticipant(&group,jid);
        mainWin->groupParticipant(gjid, jid);
    }
}

void Client::createMyJidContact()
{
    Contact &contact = roster->getContact(myJid);

    if (contact.name != "You")
    {
        contact.name = "You";
        contact.alias = userName;
        contact.status = myStatus;

        QDir home = QDir::home();
        QString fileName = home.path() + CACHE_DIR"/profilephoto.png";

        if (home.exists(fileName))
        {
            QImage image(fileName);
            contact.photo = image.scaled(QSize(64,64), Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);

            // This will force a refresh in the next synchronization
            contact.photoId = "invalid";
            roster->updatePhoto(&contact);
        }

        roster->updateContact(&contact);
    }
}

void Client::sendAddGroupParticipant(QString gjid, QString jid)
{
    if (connectionStatus == LoggedIn)
    {
        QStringList participants;
        participants.append(jid);
        connection->sendAddParticipants(gjid, participants);
    }
}

void Client::groupAddUser(QString gjid, QString jid)
{
    Group &group = roster->getGroup(gjid);

    if (!group.containsParticipant(jid))
    {
        roster->addGroupParticipant(&group,jid);
        mainWin->groupParticipant(gjid, jid);
    }
}

void Client::groupRemoveUser(QString gjid, QString jid)
{
    Group &group = roster->getGroup(gjid);

    if (group.containsParticipant(jid))
    {
        roster->removeGroupParticipant(&group,jid);
        mainWin->removeParticipant(gjid, jid);
    }
}

void Client::sendRemoveGroupParticipant(QString gjid, QString jid)
{
    if (connectionStatus == LoggedIn)
    {
        QStringList participants;
        participants.append(jid);
        connection->sendRemoveParticipants(gjid, participants);
    }
}

void Client::requestPrivacyList()
{
    if (connectionStatus == LoggedIn)
        connection->sendGetPrivacyList();
}

void Client::setPrivacyList()
{
    QStringList list = roster->getBlockedJidsList().keys();
    Utilities::logData("Blocked people count: " + QString::number(list.size()));
    connection->sendSetPrivacyBlockedList(list);
}

void Client::blockOrUnblockContact(QString jid, bool blocked)
{
    Contact &c = roster->getContact(jid);
    c.blocked = blocked;
    roster->updateBlock(&c);
    setPrivacyList();
}

void Client::privacyListReceived(QStringList list)
{
    // Synchronize privacy list

    ContactList blockedList = roster->getBlockedJidsList();
    ContactList serverList;

    foreach(QString jid, list)
    {
        Contact &c = roster->getContact(jid);
        serverList.insert(jid,&c);
    }

    // Remove entries that are in both
    foreach (QString jid, serverList.keys())
        if (blockedList.contains(jid))
        {
            serverList.remove(jid);
            blockedList.remove(jid);
        }

    // Block people locally that are blocked in servers

    foreach (QString jid, serverList.keys())
    {
        Contact *c = serverList.value(jid);
        c->blocked = true;
        roster->updateBlock(c);
    }

    // Block people in servers that are blocked locally
    if (blockedList.size() > 0)
        setPrivacyList();

    mainWin->refreshPrivacyList();
}

void Client::sendVoiceNotePlayed(FMessage message)
{
    if (connectionStatus == LoggedIn)
        connection->sendVoiceNotePlayed(message);
}
