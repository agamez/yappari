#include "mainconnection.h"
#include "src/settings/accountsettings.h"
#include "src/settings/serverproperties.h"
#include "src/constants.h"
#include "regtools.h"
#include "src/models/contactsbasemodel.h"

#include <QThread>
#include <QDateTime>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QProcess>
#include <QBuffer>
#include <QImage>
#include <QImageReader>
#include <QDebug>

#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/aes.h>

#include <QXmlQuery>

MainConnection::MainConnection(QObject *parent) :
    QObject(parent)
{
    qDebug() << "Constructing MainConnection";

    m_connectionStatus = WAConnection::Disconnected;
    Q_EMIT connectionStatusChanged();

    connection = WAConnection::GetInstance(0);

    QThread *thread = new QThread(this);
    thread->setObjectName(QString("connectionThread-%1").arg(QDateTime::currentMSecsSinceEpoch()));
    connection->moveToThread(thread);
    QObject::connect(thread, SIGNAL(started()), connection, SLOT(init()));
    QObject::connect(connection, SIGNAL(destroyed()), thread, SLOT(quit()));

    QObject::connect(connection, SIGNAL(authSuccess(AttributeList)), this, SLOT(onAuthSuccess(AttributeList)));
    QObject::connect(connection, SIGNAL(authFailed()), this, SLOT(onAuthFailed()));
    QObject::connect(connection, SIGNAL(connectionStatusChanged(int)), this, SLOT(onConnectionStatusChanged(int)));
    QObject::connect(connection, SIGNAL(serverProperties(QVariantMap)), this, SLOT(onServerProperties(QVariantMap)));

    QObject::connect(connection, SIGNAL(contactsSynced(QVariantMap)), this, SIGNAL(contactsSynced(QVariantMap)));
    QObject::connect(connection, SIGNAL(contactsStatuses(QVariantMap)), this, SIGNAL(contactsStatuses(QVariantMap)));
    QObject::connect(connection, SIGNAL(contactsPictureIds(QVariantMap)), this, SIGNAL(contactsPictureIds(QVariantMap)));
    QObject::connect(connection, SIGNAL(contactPictureHidden(QString,QString)), this, SIGNAL(contactPictureHidden(QString,QString)));
    QObject::connect(connection, SIGNAL(contactPicture(QString,QByteArray,QString)), this, SIGNAL(contactPicture(QString,QByteArray,QString)));
    QObject::connect(connection, SIGNAL(contactPictureId(QString,QString,QString)), this, SIGNAL(contactPictureId(QString,QString,QString)));
    QObject::connect(connection, SIGNAL(contactStatus(QString,QString,QString)), this, SIGNAL(contactStatus(QString,QString,QString)));
    QObject::connect(connection, SIGNAL(contactLastSeen(QString,uint)), this, SIGNAL(contactLastSeen(QString,uint)));
    QObject::connect(connection, SIGNAL(contactLastSeenHidden(QString,QString)), this, SIGNAL(contactLastSeenHidden(QString,QString)));
    QObject::connect(connection, SIGNAL(contactAvailable(QString)), this, SIGNAL(contactAvailable(QString)));
    QObject::connect(connection, SIGNAL(contactUnavailable(QString,QString)), this, SIGNAL(contactUnavailable(QString,QString)));
    QObject::connect(connection, SIGNAL(groupsReceived(QVariantMap)), this, SIGNAL(groupsReceived(QVariantMap)));
    QObject::connect(connection, SIGNAL(blacklistReceived(QStringList)), this, SIGNAL(blacklistReceived(QStringList)));
    QObject::connect(connection, SIGNAL(broadcastsReceived(QVariantMap)), this, SIGNAL(broadcastsReceived(QVariantMap)));

    QObject::connect(connection, SIGNAL(groupParticipantAdded(QString,QString)), this, SIGNAL(groupParticipantAdded(QString,QString)));
    QObject::connect(connection, SIGNAL(groupParticipantDemoted(QString,QString)), this, SIGNAL(groupParticipantDemoted(QString,QString)));
    QObject::connect(connection, SIGNAL(groupParticipantPromoted(QString,QString)), this, SIGNAL(groupParticipantPromoted(QString,QString)));
    QObject::connect(connection, SIGNAL(groupParticipantRemoved(QString,QString)), this, SIGNAL(groupParticipantRemoved(QString,QString)));
    QObject::connect(connection, SIGNAL(groupSubjectChanged(QString,QString,QString,QString)), this, SIGNAL(groupSubjectChanged(QString,QString,QString,QString)));
    QObject::connect(connection, SIGNAL(groupCreated(QString,QString,QString,QString,QString,QString,QStringList,QStringList)), this, SIGNAL(groupCreated(QString,QString,QString,QString,QString,QString,QStringList,QStringList)));
    QObject::connect(connection, SIGNAL(groupInfo(QString,QVariantMap)), this, SIGNAL(groupInfo(QString,QVariantMap)));

    QObject::connect(connection, SIGNAL(textMessageReceived(QString,QString,QString,QString,bool,QString)), this, SIGNAL(textMessageReceived(QString,QString,QString,QString,bool,QString)));
    QObject::connect(connection, SIGNAL(mediaMessageReceived(QString,QString,QString,QString,bool,AttributeList,QByteArray)), this, SIGNAL(mediaMessageReceived(QString,QString,QString,QString,bool,AttributeList,QByteArray)));
    QObject::connect(connection, SIGNAL(textMessageSent(QString,QString,QString,QString)), this, SIGNAL(textMessageSent(QString,QString,QString,QString)));
    QObject::connect(connection, SIGNAL(messageSent(QString,QString,QString)), this, SIGNAL(messageSent(QString,QString,QString)));

    QObject::connect(connection, SIGNAL(messageReceipt(QString,QString,QString,QString,QString)), this, SIGNAL(messageReceipt(QString,QString,QString,QString,QString)));
    QObject::connect(connection, SIGNAL(retryMessage(QString,QString)), this, SIGNAL(retryMessage(QString,QString)));

    QObject::connect(connection, SIGNAL(encryptionStatus(QString,bool)), this, SIGNAL(encryptionStatus(QString,bool)));

    QObject::connect(this, SIGNAL(connectionLogin(QVariantMap)), connection, SLOT(login(QVariantMap)));
    QObject::connect(this, SIGNAL(connectionLogout()), connection, SLOT(logout()));
    QObject::connect(this, SIGNAL(connectionReconnect()), connection, SLOT(reconnect()));
    QObject::connect(this, SIGNAL(connectionSendGetProperties()), connection, SLOT(sendGetProperties()));
    QObject::connect(this, SIGNAL(connectionSendText(QString,QString)), connection, SLOT(sendText(QString,QString)));
    QObject::connect(this, SIGNAL(connectionSyncContacts(QVariantMap)), connection, SLOT(syncContacts(QVariantMap)));
    QObject::connect(this, SIGNAL(connectionSendGetPictureIds(QStringList)), connection, SLOT(sendGetPictureIds(QStringList)));
    QObject::connect(this, SIGNAL(connectionSendGetStatuses(QStringList)), connection, SLOT(sendGetStatuses(QStringList)));
    QObject::connect(this, SIGNAL(connectionSendGetPicture(QString)), connection, SLOT(sendGetPicture(QString)));
    QObject::connect(this, SIGNAL(connectionSendGetLastSeen(QString)), connection, SLOT(sendGetLastSeen(QString)));
    QObject::connect(this, SIGNAL(connectionSendSubscribe(QString)), connection, SLOT(sendSubscribe(QString)));
    QObject::connect(this, SIGNAL(connectionSendUnsubscribe(QString)), connection, SLOT(sendUnsubscribe(QString)));
    QObject::connect(this, SIGNAL(connectionSendAvailable(QString)), connection, SLOT(sendAvailable(QString)));
    QObject::connect(this, SIGNAL(connectionSendUnavailable(QString)), connection, SLOT(sendUnavailable(QString)));
    QObject::connect(this, SIGNAL(connectionGetGroups(QString)), connection, SLOT(sendGetGroups(QString)));
    QObject::connect(this, SIGNAL(connectionGetBroadcasts()), connection, SLOT(sendGetBroadcasts()));
    QObject::connect(this, SIGNAL(connectionSendRetryMessage(QString,QString,QString)), connection, SLOT(sendRetryMessage(QString,QString,QString)));
    QObject::connect(this, SIGNAL(connectionSendTyping(QString,bool)), connection, SLOT(sendTyping(QString,bool)));
    QObject::connect(this, SIGNAL(connectionGetEncryptionStatus(QString)), connection, SLOT(getEncryptionStatus(QString)));
    QObject::connect(this, SIGNAL(connectionSendSetStatusMessage(QString)), connection, SLOT(sendSetStatusMessage(QString)));
    QObject::connect(this, SIGNAL(connectionSendSetAvatar(QString,QByteArray,QByteArray)), connection, SLOT(sendSetPicture(QString,QByteArray,QByteArray)));
    QObject::connect(this, SIGNAL(connectionSendSetGroupSubject(QString,QString)), connection, SLOT(sendSetGroupSubject(QString,QString)));
    QObject::connect(this, SIGNAL(connectionSendGetGroupInfo(QString)), connection, SLOT(sendGetGroupInfo(QString)));

    thread->start();

    keepalive = new BackgroundActivity(this);
    connect(keepalive, SIGNAL(running()), this, SLOT(checkActivity()));
    connect(keepalive, SIGNAL(stopped()), this, SLOT(wakeupStopped()));

    m_needReconnect = false;

    nconf = new QNetworkConfigurationManager(this);
    QObject::connect(nconf, SIGNAL(onlineStateChanged(bool)), this, SLOT(onlineStateChanged(bool)));
    QObject::connect(nconf, SIGNAL(configurationAdded(QNetworkConfiguration)), this, SLOT(configurationAdded(QNetworkConfiguration)));
    QObject::connect(nconf, SIGNAL(configurationRemoved(QNetworkConfiguration)), this, SLOT(configurationRemoved(QNetworkConfiguration)));
    QObject::connect(nconf, SIGNAL(configurationChanged(QNetworkConfiguration)), this, SLOT(configurationChanged(QNetworkConfiguration)));
    onlineStateChanged(nconf->isOnline());

    m_myJid = QString("%1@s.whatsapp.net").arg(AccountSettings::GetInstance()->value("login").toString());
    Q_EMIT myJidChanged();
}

MainConnection *MainConnection::GetInstance(QObject *parent)
{
    static MainConnection* lsSingleton = NULL;
    if (!lsSingleton) {
        lsSingleton = new MainConnection(parent);
    }
    return lsSingleton;
}

void MainConnection::login(bool force)
{
    if (ContactsBaseModel::GetInstance()->isBusy()) {
        return;
    }

    if (!nconf->isOnline() && force) {
        QDBusInterface connSelectorInterface(QStringLiteral("com.jolla.lipstick.ConnectionSelector"),
                                             QStringLiteral("/"),
                                             QStringLiteral("com.jolla.lipstick.ConnectionSelectorIf"));

        connSelectorInterface.call(QDBus::NoBlock, "openConnection", "wlan");

        m_needReconnect = true;
        return;
    }

    QVariantMap loginData;
    loginData["login"] = AccountSettings::GetInstance()->value("login").toString();
    loginData["password"] = AccountSettings::GetInstance()->value("password").toString();
    QString platform = AccountSettings::GetInstance()->value("platform", "Android").toString();
    loginData["resource"] = RegTools::getResource(platform);
    loginData["encryptionav"] = RegTools::getEncryptionAV(platform);
    QString device = platform == "Android" ? RegTools::getDevice(platform) : AccountSettings::GetInstance()->value("device", RegTools::getDevice(platform)).toString();
    useragent = RegTools::getUseragent(device, platform);
    loginData["userAgent"] = useragent;
    QString mcc = AccountSettings::GetInstance()->value("mcc").toString();
    loginData["mcc"] = mcc.rightJustified(3, '0');
    QString mnc = AccountSettings::GetInstance()->value("mnc").toString();
    loginData["mnc"] = mnc.rightJustified(3, '0');
    loginData["nextChallenge"] = AccountSettings::GetInstance()->value("nextChallenge").toString();
    loginData["servers"] = RegTools::getServers();
    AccountSettings::GetInstance()->setValue("nextChallenge", "");
    QString dataFile = QString("%1/axolotl.db").arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    loginData["database"] = dataFile;
    loginData["passive"] = ContactsBaseModel::GetInstance()->count() == 0;
    if (loginData["passive"].toBool()) {
        qDebug() << "PASSIVE LOGIN!";
    }
    else {
        qDebug() << "CONTACTS COUNT:" << ContactsBaseModel::GetInstance()->count();
    }

    Q_EMIT connectionLogin(loginData);
}

void MainConnection::logout()
{
    m_needReconnect = false;
    Q_EMIT connectionLogout();
}

void MainConnection::getEncryptionStatus(const QString &jid)
{
    Q_EMIT connectionGetEncryptionStatus(jid);
}

void MainConnection::checkImports()
{
    QProcess *proc1 = new QProcess(this);
    proc1->start("/usr/bin/mitakuuluu3-helper", QStringList() << "check");
    proc1->waitForFinished(10000);
    if (proc1->exitCode() == 0) {
        QByteArray medata;
        QFile mefile("/data/data/com.whatsapp/files/me");
        if (mefile.open(QFile::ReadOnly)) {
            medata = mefile.readAll();
            mefile.close();
        }
        else {
            qWarning() << "medata not found";
            return;
        }
        QByteArray pwdata;
        QFile pwfile("/data/data/com.whatsapp/files/pw");
        if (pwfile.open(QFile::ReadOnly)) {
            pwdata = pwfile.readAll();
            pwfile.close();
        }
        else {
            qWarning() << "pwdata not found";
            return;
        }

        int base = 99;
        int cclen = medata.at(base);
        QByteArray cc = medata.mid(base + 1, cclen);
        int jidlen = medata.at(base + cclen + 3);
        QByteArray jid = medata.mid(base + cclen + 4, jidlen);
        int numlen = medata.at(base + cclen + jidlen + 6);
        QByteArray num = medata.mid(base + cclen + jidlen + 7, numlen);

        int pbase = 26;
        int datalen = pwdata.at(pbase);
        QByteArray data = pwdata.mid(pbase + 1, datalen);

        QByteArray salt = data.mid(2, 4);
        QByteArray iv = data.mid(6, 16);
        QByteArray enc_key = data.mid(22, datalen - 22);
        enc_key.append(QByteArray("111111111111"));
        QByteArray edata = QByteArray::fromHex("c2991ec29b1d0cc2b8c3b7556458c298c29203c28b45c2973e78c386c395");
        edata.append(jid);

        int ic = 16;
        unsigned char buf[16];

        QByteArray saltTemp = salt;
        PKCS5_PBKDF2_HMAC_SHA1(edata.constData(), edata.size(), (unsigned char*)saltTemp.data(), saltTemp.size(), ic, 16, buf);

        AES_KEY aes_key;
        AES_set_encrypt_key(buf, 128, &aes_key);
        int iter = 0;
        unsigned char pass[enc_key.size()];
        AES_ofb128_encrypt ((const unsigned char*)enc_key.constData(), pass, enc_key.size(), &aes_key, (unsigned char*)iv.constData(), &iter);
        QByteArray pass_bytes = QByteArray::fromRawData((char*)pass, 20);

        QXmlQuery query;
        QString next_ch;
        QString push_name;
        QFile prefs("/data/data/com.whatsapp/shared_prefs/com.whatsapp_preferences.xml");
        if (prefs.open(QFile::ReadOnly)) {
            query.setFocus(&prefs);
            query.setQuery("/map/string[@name=\"next_ch_data\"]/string()");
            query.evaluateTo(&next_ch);
            next_ch = next_ch.trimmed();
            query.setQuery("/map/string[@name=\"push_name\"]/string()");
            query.evaluateTo(&push_name);
            push_name = push_name.trimmed();
            prefs.close();
        }

        QProcess proc;
        proc.startDetached("/usr/bin/mitakuuluu3-helper", QStringList() << "remove_temp");

        QVariantMap importData;
        importData["name"] = "Android";
        importData["type"] = "android";
        importData["phone"] = QString(cc + num);
        importData["pass"] = QString(pass_bytes.toBase64());
        importData["nextch"] = next_ch;
        importData["pushname"] = push_name;

        Q_EMIT importFound(importData);
    }
    else {
        qDebug() << "proc1:" << proc1->exitCode();
    }
    proc1->deleteLater();

    QString whatsupConfPath = QString("%1/.config/cepiperez/whatsup.conf").arg(QDir::homePath());
    if (QFile(whatsupConfPath).exists()) {
        QSettings whatsupConf(whatsupConfPath, QSettings::IniFormat);

        QVariantMap importData;
        importData["name"] = "Whatsup";
        importData["type"] = "whatsup";
        importData["phone"] = whatsupConf.value("username").toString();
        importData["pass"] = whatsupConf.value("password").toString();
        importData["nextch"] = whatsupConf.value("nextchallenge").toString();
        importData["pushname"] = whatsupConf.value("pushName").toString();

        Q_EMIT importFound(importData);
    }

    MDConfAgent mitakuuluu("/apps/harbour-mitakuuluu2/account/");
    if (!mitakuuluu.value("password").toString().isEmpty()) {
        QVariantMap importData;
        importData["name"] = "Mitakuuluu2";
        importData["type"] = "mitakuuluu";
        importData["phone"] = mitakuuluu.value("phoneNumber").toString();
        importData["pass"] = mitakuuluu.value("password").toString();
        importData["nextch"] = QString();
        importData["pushname"] = mitakuuluu.value("pushname").toString();

        Q_EMIT importFound(importData);
    }
}

void MainConnection::deactivateAccount(const QString &type)
{
    if (type == "android") {
        QProcess proc;
        proc.startDetached("/usr/bin/mitakuuluu3-helper", QStringList() << "remove_android");
    }
    else if (type == "whatsup") {
        QString whatsupConfPath = QString("%1/.config/cepiperez/whatsup.conf").arg(QDir::homePath());
        QSettings whatsupConf(whatsupConfPath, QSettings::IniFormat);
        whatsupConf.remove("password");
        whatsupConf.remove("nextchallenge");
        whatsupConf.remove("username");
    }
    else if (type == "mitakuuluu") {
        MDConfAgent mitakuuluu("/apps/harbour-mitakuuluu2/account/");
        mitakuuluu.unsetValue("phoneNumber");
        mitakuuluu.unsetValue("password");
    }
}

void MainConnection::downloadMedia(const QString &jid, const QString &from, const QString &fileName, const QString &msgid)
{
    QString destDir = QString("%1/media").arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    if (!QDir(destDir).exists()) {
        QDir().mkpath(destDir);
    }
    QString destination = QString("%1/%2").arg(destDir).arg(fileName);

    qDebug() << from << destination << msgid;

    MediaDownloader *downloader = new MediaDownloader(useragent, 0);
    downloader->setData(jid, from, destination, msgid);
    QThread *thread = new QThread(this);
    thread->setObjectName(QString("mediaDownloader-%1").arg(QDateTime::currentMSecsSinceEpoch()));
    downloader->moveToThread(thread);
    QObject::connect(thread, SIGNAL(started()), downloader, SLOT(download()));
    QObject::connect(downloader, SIGNAL(destroyed()), thread, SLOT(quit()));

    QObject::connect(downloader, SIGNAL(completed(QString,QString,QString,MediaDownloader*)), this, SLOT(onDownloadCompleted(QString,QString,QString,MediaDownloader*)));
    QObject::connect(downloader, SIGNAL(failed(QString,QString,MediaDownloader*)), this, SLOT(onDownloadFailed(QString,QString,MediaDownloader*)));
    QObject::connect(downloader, SIGNAL(progress(QString,float,QString)), this, SLOT(onDownloadProgress(QString,float,QString)));

    thread->start();
}

void MainConnection::openVCardData(const QString &name, const QString &data)
{
    QString dirPath = QString("%1/vcard").arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    if (!QDir(dirPath).exists()) {
        QDir().mkpath(dirPath);
    }

    QString path = QString("%1/%2.vcf").arg(dirPath).arg(name);
    QFile file(path);
    if (file.exists())
        file.remove();
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        file.write(QByteArray::fromBase64(data.toLatin1()));
        file.close();
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

bool MainConnection::compressLogs(const QString &since)
{
    QProcess helper;
    QStringList args;
    args << "get_logs";
    if (!since.isEmpty()) {
        args << since;
    }
    helper.start("/usr/bin/mitakuuluu3-helper", args);
    helper.waitForFinished();
    return helper.exitCode() == 0;
}

void MainConnection::presenceAvailable(const QString &pushname)
{
    Q_EMIT connectionSendAvailable(pushname);
}

void MainConnection::presenceUnavailable(const QString &pushname)
{
    Q_EMIT connectionSendUnavailable(pushname);
}

void MainConnection::setStatusMessage(const QString &message)
{
    Q_EMIT connectionSendSetStatusMessage(message);
}

void MainConnection::sendSetAvatar(const QString &jid, const QString &file)
{
    qDebug() << jid << file;
    QByteArray imgdata;
    QBuffer dataBuffer(&imgdata);
    QByteArray thumbdata;
    QBuffer thumbBuffer(&thumbdata);
    QString path = file;
    path.replace("file://", "");
    QFile ava(path);
    if (ava.exists()) {
        QImageReader reader(path);
        qDebug() << reader.size().width() << reader.size().height();
        int scaled0 = qMin(reader.size().width(), reader.size().height());
        if (scaled0 < 128) {
            reader.setScaledSize(QSize(128, 128));
        }
        else if (scaled0 > 512) {
            reader.setScaledSize(QSize(512, 512));
        }
        else {
            reader.setScaledSize(QSize(scaled0, scaled0));
        }
        QImage img;
        if (reader.read(&img)) {
            img.save(&dataBuffer, "JPEG", 100);
        }

        reader.setFileName(path);
        reader.setScaledSize(QSize(128, 128));
        QImage thumb;
        if (reader.read(&thumb)) {
            thumb.save(&thumbBuffer, "JPEG", 90);
        }
    }
    qDebug() << jid << imgdata.size() << thumbdata.size();
    Q_EMIT connectionSendSetAvatar(jid, imgdata, thumbdata);
}

void MainConnection::sendSetGroupSubject(const QString &jid, const QString &subject)
{
    Q_EMIT connectionSendSetGroupSubject(jid, subject);
}

void MainConnection::sendGetGroupInfo(const QString &jid)
{
    Q_EMIT connectionSendGetGroupInfo(jid);
}

QString MainConnection::getPathForAvatar(const QString &jid)
{
    return QString("%1/avatars/j%2.jpg").arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation))
                                   .arg(jid.split("@").first());
}

void MainConnection::sendTyping(const QString &jid, bool typing)
{
    Q_EMIT connectionSendTyping(jid, typing);
}

void MainConnection::sendRetryMessage(const QString &jid, const QString &msgId, const QString &data)
{
    Q_EMIT connectionSendRetryMessage(jid, msgId, data);
}

void MainConnection::getGroups(const QString &type)
{
    Q_EMIT connectionGetGroups(type);
}

void MainConnection::getBroadcasts()
{
    Q_EMIT connectionGetBroadcasts();
}

void MainConnection::sendSubscribe(const QString &jid)
{
    Q_EMIT connectionSendSubscribe(jid);
}

void MainConnection::sendUnsubscribe(const QString &jid)
{
    Q_EMIT connectionSendUnsubscribe(jid);
}

void MainConnection::getContactLastSeen(const QString &jid)
{
    Q_EMIT connectionSendGetLastSeen(jid);
}

void MainConnection::getContactPicture(const QString &jid)
{
    Q_EMIT connectionSendGetPicture(jid);
}

void MainConnection::getContactsPictureIds(const QStringList &jids)
{
    Q_EMIT connectionSendGetPictureIds(jids);
}

void MainConnection::getContactsStatuses(const QStringList &jids)
{
    Q_EMIT connectionSendGetStatuses(jids);
}

void MainConnection::syncContacts(const QVariantMap &contacts)
{
    Q_EMIT connectionSyncContacts(contacts);
}

void MainConnection::sendText(const QString &jid, const QString &text)
{
    qDebug() << jid << text;
    Q_EMIT connectionSendText(jid, text);
}

int MainConnection::connectionStatus()
{
    return m_connectionStatus;
}

QString MainConnection::myJid()
{
    return m_myJid;
}

int MainConnection::available()
{
    return AccountSettings::GetInstance()->value("available", true).toBool() ? 0 : 1;
}

void MainConnection::setAvailable(int value)
{
    if (value != available()) {
        AccountSettings::GetInstance()->setValue("available", value == Available);
        Q_EMIT availableChanged();

        if (m_connectionStatus == LoggedIn) {
            QString pushname = AccountSettings::GetInstance()->value("pushname").toString();
            if (value == Available) {
                Q_EMIT connectionSendAvailable(pushname);
            }
            else {
                Q_EMIT connectionSendUnavailable(pushname);
            }
        }
    }
}

void MainConnection::onServerProperties(const QVariantMap &props)
{
    foreach (const QString &key, props.keys()) {
        ServerProperties::GetInstance()->setValue(key, props.value(key));
    }
}

void MainConnection::onConnectionStatusChanged(int newConnectionStatus)
{
    qDebug() << newConnectionStatus;
    m_connectionStatus = newConnectionStatus;
    Q_EMIT connectionStatusChanged();

    if (m_connectionStatus == WAConnection::Disconnected) {
        if (keepalive->isWaiting()) {
            keepalive->stop();
        }
    }
}

void MainConnection::checkActivity()
{
    keepalive->wait(BackgroundActivity::TenMinutes);
}

void MainConnection::wakeupStopped()
{
    qDebug() << "WAKEUP STOPPED! WHAT SHOULD I DO NOW!?";
}

void MainConnection::onlineStateChanged(bool isOnline)
{
    qDebug() << isOnline;

    if (!isOnline) {
        if (m_connectionStatus >= WAConnection::Connecting) {
            Q_EMIT connectionLogout();
            m_needReconnect = true;
        }
    }
    else {
        if (m_needReconnect) {
            Q_EMIT connectionReconnect();
        }
    }
}

void MainConnection::configurationAdded(const QNetworkConfiguration &config)
{
    qDebug() << config.identifier() << config.name() << config.state();
}

void MainConnection::configurationRemoved(const QNetworkConfiguration &config)
{
    qDebug() << config.identifier() << config.name() << config.state();
}

void MainConnection::configurationChanged(const QNetworkConfiguration &config)
{
    qDebug() << config.identifier() << config.name() << config.state();
    if (config.state() == QNetworkConfiguration::Active) {
        if (m_connectionStatus >= Connecting) {
            m_needReconnect = true;
            Q_EMIT connectionLogout();
            onlineStateChanged(nconf->isOnline());
        }
    }
}

void MainConnection::onDownloadCompleted(const QString &jid, const QString &dest, const QString &msgid, MediaDownloader *object)
{
    //qDebug() << jid << dest << msgid;

    object->deleteLater();

    Q_EMIT downloadCompleted(jid, dest, msgid);
}

void MainConnection::onDownloadFailed(const QString &jid, const QString &msgid, MediaDownloader *object)
{
    //qDebug() << jid << msgid;

    object->deleteLater();

    Q_EMIT downloadFailed(jid, msgid);
}

void MainConnection::onDownloadProgress(const QString &jid, float progress, const QString &msgid)
{
    //qDebug() << jid << progress << msgid;

    Q_EMIT downloadProgress(jid, progress, msgid);
}

void MainConnection::onAuthSuccess(const AttributeList &accountData)
{
    m_myJid = QString("%1@s.whatsapp.net").arg(AccountSettings::GetInstance()->value("login").toString());
    Q_EMIT myJidChanged();

    AccountSettings::GetInstance()->setValue("creation", accountData["creation"].toString());
    AccountSettings::GetInstance()->setValue("expiration", accountData["expiration"].toString());
    AccountSettings::GetInstance()->setValue("kind", accountData["kind"].toString());
    AccountSettings::GetInstance()->setValue("status", accountData["status"].toString());
    AccountSettings::GetInstance()->setValue("lastLogin", accountData["t"].toString());
    AccountSettings::GetInstance()->setValue("nextChallenge", accountData["nextChallenge"].toString());
    QString props = AccountSettings::GetInstance()->value("props", QString("1")).toString();
    if (props != accountData["props"].toString()) {
        AccountSettings::GetInstance()->setValue("props", accountData["props"].toString());
        Q_EMIT connectionSendGetProperties();
    }
    QString login = AccountSettings::GetInstance()->value("login").toString();

    bool available = AccountSettings::GetInstance()->value("available", true).toBool();
    QString pushname = AccountSettings::GetInstance()->value("pushname", login).toString();
    if (available) {
        Q_EMIT connectionSendAvailable(pushname);
    }
    else {
        Q_EMIT connectionSendUnavailable(pushname);
    }

    if (AccountSettings::GetInstance()->value("message").isNull()) {
        Q_EMIT connectionSendGetStatuses(QStringList() << m_myJid);
    }

    if (AccountSettings::GetInstance()->value("avatar").isNull()) {
        Q_EMIT connectionSendGetPicture(m_myJid);
    }

    keepalive->wait(BackgroundActivity::TenMinutes);

    Q_EMIT authSuccess();
}

void MainConnection::onAuthFailed()
{
    qDebug() << "Auth failed";
    AccountSettings::GetInstance()->setValue("login", QString());
    AccountSettings::GetInstance()->setValue("password", QString());
    AccountSettings::GetInstance()->setValue("nextChallenge", QString());
    Q_EMIT authFailed();
}
