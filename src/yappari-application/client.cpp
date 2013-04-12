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

#include <mce/dbus-names.h>

#include <execinfo.h>

#include <QDateTime>
#include <QStatusBar>
#include <QSystemInfo>
#include <QMessageBox>

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

Client::ConnectionStatus Client::connectionStatus;

QSettings *Client::settings;

quint64 Client::seq;
QString Client::mycolor;
QString Client::nickcolor;
QString Client::textcolor;
QString Client::myStatus;

QString Client::myJid;
QString Client::cc;
QString Client::phoneNumber;
QString Client::password;
QString Client::userName;
QString Client::number;
QString Client::imei;

quint16 Client::port;
qint64 Client::whatsnew;

bool Client::showNicknames;
bool Client::showNumbers;
bool Client::popupOnFirstMessage;

QString Client::sync;

bool Client::android;

bool Client::isSynchronizing;

Client::Client(QObject *parent) : QObject(parent)
{
    // Debug info start
    QString version = FULL_VERSION;
    Utilities::logData("Yappari " + version);
    Utilities::logData("Yappari console test client start");

    // Read Settings
    readSettings();

    // Initialization of sequence for messages IDs
    seq = 0;

    // Contacts roster
    roster = new ContactRoster(this);

    bool showWhatsNew = false;

    // Show Whatsnew window?
    if (whatsnew != MAGIC_NUMBER)
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

Client::~Client()
{
    /*
    void *array[10];
    backtrace(array,10);
    char **strings;
    size_t size;

    size = backtrace (array, 10);
    strings = backtrace_symbols (array, size);

    Utilities::logData("Obtained " + QString::number(size) + " stack frames");

    for (size_t i = 0; i < size; i++)
       Utilities::logData(strings[i]);

    free (strings);

    */
    Utilities::logData("Application destroyed");
    applet->HideApplet();
}

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

    // What's New Window
    this->whatsnew = settings->value(SETTINGS_WHATSNEW).toLongLong();
}

void Client::updateSettings()
{
    // We just update the settings that are configurable
    // through GUI

    settings->setValue(SETTINGS_SYNC,sync);
    settings->setValue(SETTINGS_PORT,port);
    settings->setValue(SETTINGS_SHOW_NICKNAMES,showNicknames);
    settings->setValue(SETTINGS_SHOW_NUMBERS,showNumbers);
    settings->setValue(SETTINGS_POPUP_ON_FIRST_MESSAGE,popupOnFirstMessage);
}

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
    networkStatusChanged(isNetworkAvailable());
}

void Client::verifyAndConnect()
{
    // ToDo: SIM Changed? verify IMSI and force re-registration

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

    connectionStatus = Connected;
    updateStatus();
    Utilities::logData("Connected successfully");

    // QString password = Utilities::getChatPassword();

    QByteArray password = (this->password.isEmpty())
                                ? Utilities::getChatPassword().toUtf8()
                                : QByteArray::fromBase64(this->password.toUtf8());

    Utilities::logData("Password: " + password);

    connection = new Connection(socket,JID_DOMAIN,RESOURCE,phoneNumber,
                                userName,password);

    QByteArray nextChallenge = QByteArray::fromBase64(settings->value(SETTINGS_NEXTCHALLENGE).toByteArray());
    settings->remove(SETTINGS_NEXTCHALLENGE);

    Utilities::logData("Challenge: " + QString::fromLatin1(nextChallenge.toHex()));

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

    connect(connection,SIGNAL(groupNewSubject(QString,QString,QString,QString,QString)),
            mainWin,SLOT(groupNewSubject(QString,QString,QString,QString,QString)));

    connect(connection,SIGNAL(groupInfoFromList(QString,QString,QString,
                                                QString,QString,QString)),
            mainWin,SLOT(groupInfoFromList(QString,QString,QString,
                                           QString,QString,QString)));

    connect(connection,SIGNAL(messageReceived(FMessage)),
            mainWin,SLOT(messageReceived(FMessage)));

    connect(connection,SIGNAL(messageStatusUpdate(FMessage)),
            mainWin,SLOT(messageStatusUpdate(FMessage)));

    connect(connection,SIGNAL(available(QString,bool)),
            mainWin,SLOT(available(QString,bool)));

    connect(connection,SIGNAL(composing(QString)),
            mainWin,SLOT(composing(QString)));

    connect(connection,SIGNAL(paused(QString)),
            mainWin,SLOT(paused(QString)));

    connect(connection,SIGNAL(leaveGroup(QString)),
            mainWin,SLOT(leaveGroup(QString)));

    connect(connection,SIGNAL(userStatusUpdated(QString)),
            this,SLOT(userStatusUpdated(QString)));

    connect(connection,SIGNAL(lastOnline(QString,qint64)),
            mainWin,SLOT(available(QString,qint64)));

    connect(connection,SIGNAL(mediaUploadAccepted(FMessage)),
            mainWin,SLOT(mediaUploadAccepted(FMessage)));

    // Update participating groups
    connection->updateGroupChats();

    keepAliveTimer->start(MIN_INTERVAL);
    keepAliveTimer->setSingleShot(true);

    pendingMessagesTimer->start(CHECK_QUEUE_INTERVAL);
    pendingMessagesTimer->setSingleShot(true);

    // Set status if hasn't been set before
    if (this->myStatus.isEmpty())
        changeStatus(DEFAULT_STATUS);

    synchronizeContacts();
}

void Client::synchronizeContacts()
{
    // Contacts syncer
    syncer = new ContactSyncer(roster);

    connect(syncer,SIGNAL(syncFinished()),this,SLOT(syncFinished()));
    connect(syncer,SIGNAL(progress(int)),this,SLOT(syncProgress(int)));

    isSynchronizing = true;
    QTimer::singleShot(0,syncer,SLOT(sync()));

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
    qint64 timeElapsed = QDateTime::currentMSecsSinceEpoch() -
                         connection->getLastTreeReadTimestamp();

    if (timeElapsed >= MIN_INTERVAL)
    {
        connection->sendNop();
        keepAliveTimer->start(MIN_INTERVAL);
    }
    else
    {
        keepAliveTimer->start(MIN_INTERVAL - timeElapsed);
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
        connection->sendSetGroupSubject(gjid,subject);
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

void Client::userStatusUpdated(QString status)
{
    Utilities::logData("User status confirmed: " + status);
    settings->setValue(SETTINGS_STATUS,status);
    this->myStatus = status;
}
