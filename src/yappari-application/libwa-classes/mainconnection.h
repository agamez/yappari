#ifndef MAINCONNECTION_H
#define MAINCONNECTION_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkConfigurationManager>
#include <keepalive/backgroundactivity.h>

#include <libwa.h>

class MainConnection : public QObject
{
    Q_OBJECT
    Q_ENUMS(ConnectionStatus)
    Q_ENUMS(AvailableStatus)
    Q_PROPERTY(int connectionStatus READ connectionStatus NOTIFY connectionStatusChanged)
    Q_PROPERTY(int available READ available WRITE setAvailable NOTIFY availableChanged)
    Q_PROPERTY(QString myJid READ myJid NOTIFY myJidChanged)
public:
    explicit MainConnection(QObject *parent = 0);
    static MainConnection *GetInstance(QObject *parent = 0);

    enum ConnectionStatus {
        Disconnected,
        Connecting,
        Connected,
        Initiaization,
        LoggedIn
    };

    enum AvailableStatus {
        Available,
        Unavailable
    };

    Q_INVOKABLE void login(bool force = false);
    Q_INVOKABLE void logout();

    Q_INVOKABLE void sendText(const QString &jid, const QString &text);
    Q_INVOKABLE void syncContacts(const QVariantMap &contacts);
    Q_INVOKABLE void getContactsStatuses(const QStringList &jids);
    Q_INVOKABLE void getContactsPictureIds(const QStringList &jids);
    Q_INVOKABLE void getContactPicture(const QString &jid);
    Q_INVOKABLE void getContactLastSeen(const QString &jid);
    Q_INVOKABLE void sendSubscribe(const QString &jid);
    Q_INVOKABLE void sendUnsubscribe(const QString &jid);
    Q_INVOKABLE void getGroups(const QString &type);
    Q_INVOKABLE void getBroadcasts();
    Q_INVOKABLE void sendTyping(const QString &jid, bool typing);
    Q_INVOKABLE void presenceAvailable(const QString &pushname = QString());
    Q_INVOKABLE void presenceUnavailable(const QString &pushname = QString());
    Q_INVOKABLE void setStatusMessage(const QString &message);
    Q_INVOKABLE void sendSetAvatar(const QString &jid, const QString &file);
    Q_INVOKABLE void sendSetGroupSubject(const QString &jid, const QString &subject);
    Q_INVOKABLE void sendGetGroupInfo(const QString &jid);

    Q_INVOKABLE void getEncryptionStatus(const QString &jid);

    Q_INVOKABLE void checkImports();
    Q_INVOKABLE void deactivateAccount(const QString &type);

    Q_INVOKABLE void downloadMedia(const QString &jid, const QString &from, const QString &fileName, const QString &msgid);

    Q_INVOKABLE void openVCardData(const QString &name, const QString &data);
    Q_INVOKABLE bool compressLogs(const QString &since = QString());

    Q_INVOKABLE QString getPathForAvatar(const QString &jid);

    void sendRetryMessage(const QString &jid, const QString &msgId, const QString &data);

    int connectionStatus();
    QString myJid();

    int available();
    void setAvailable(int value);

private:
    WAConnection *connection;
    int m_connectionStatus;
    bool m_needReconnect;
    QString m_myJid;
    QNetworkConfigurationManager *nconf;
    BackgroundActivity *keepalive;

    QString useragent;

private slots:
    void onAuthSuccess(const AttributeList &authData);
    void onAuthFailed();
    void onServerProperties(const QVariantMap &props);

    void onConnectionStatusChanged(int newConnectionStatus);

    void checkActivity();
    void wakeupStopped();

    void onlineStateChanged(bool isOnline);
    void configurationAdded(const QNetworkConfiguration &config);
    void configurationRemoved(const QNetworkConfiguration &config);
    void configurationChanged(const QNetworkConfiguration &config);

    void onDownloadCompleted(const QString &jid, const QString &from, const QString &msgid, MediaDownloader *object);
    void onDownloadFailed(const QString &jid, const QString &msgid, MediaDownloader *object);
    void onDownloadProgress(const QString &jid, float progress, const QString &msgid);

signals:
    void connectionStatusChanged();
    void availableChanged();
    void myJidChanged();

    void authSuccess();
    void authFailed();

    void connectionLogin(const QVariantMap &loginData);
    void connectionLogout();
    void connectionReconnect();
    void connectionSendGetProperties();

    void connectionSendText(const QString &jid, const QString &text);
    void connectionSendRetryMessage(const QString &jid, const QString &msgId, const QString &data);

    void connectionSyncContacts(const QVariantMap &contacts);
    void connectionSendGetStatuses(const QStringList &jids);
    void connectionGetGroups(const QString &type);
    void connectionGetBroadcasts();
    void connectionSendGetPictureIds(const QStringList &jids);
    void connectionSendGetPicture(const QString &jid);
    void connectionSendGetLastSeen(const QString &jid);
    void connectionSendSubscribe(const QString &jid);
    void connectionSendUnsubscribe(const QString &jid);
    void connectionSendAvailable(const QString &pushname);
    void connectionSendUnavailable(const QString &pushname = QString());
    void connectionSendTyping(const QString &jid, bool typing);
    void connectionSendSetStatusMessage(const QString &message);
    void connectionSendSetAvatar(const QString &jid, const QByteArray &data, const QByteArray &thumb = QByteArray());
    void connectionSendSetGroupSubject(const QString &jid, const QString &subject);
    void connectionSendGetGroupInfo(const QString &jid);

    void connectionGetEncryptionStatus(const QString &jid);

    void contactsSynced(const QVariantMap &contacts);
    void contactsStatuses(const QVariantMap &contacts);
    void contactsPictureIds(const QVariantMap &contacts);
    void contactPictureHidden(const QString &jid, const QString &code);
    void contactPicture(const QString &jid, const QByteArray &data, const QString &id);
    void contactPictureId(const QString &jid, const QString &id, const QString &author);
    void contactStatus(const QString &jid, const QString &status, const QString &t);
    void contactLastSeen(const QString &jid, uint timestamp);
    void contactLastSeenHidden(const QString &jid, const QString &code);
    void contactAvailable(const QString &jid);
    void contactUnavailable(const QString &jid, const QString &last);
    void groupsReceived(const QVariantMap &groups);
    void blacklistReceived(const QStringList &list);
    void broadcastsReceived(const QVariantMap &broadcasts);
    void groupInfo(const QString &jid, const QVariantMap &attributes);

    void textMessageReceived(const QString &jid, const QString &id, const QString &timestamp, const QString &author, bool offline, const QString &data);
    void mediaMessageReceived(const QString &jid, const QString &id, const QString &timestamp, const QString &author, bool offline, const AttributeList &attrs, const QByteArray &data);
    void textMessageSent(const QString &jid, const QString &id, const QString &timestamp, const QString &data);
    void messageSent(const QString &jid, const QString &msgId, const QString &timestamp);

    void messageReceipt(const QString &jid, const QString &msgId, const QString &participant, const QString &timestamp, const QString &type);
    void retryMessage(const QString &msgId, const QString &jid);

    void encryptionStatus(const QString &pjid, bool penc);

    void importFound(const QVariantMap &importData);

    void downloadCompleted(const QString &jid, const QString &dest, const QString &msgid);
    void downloadFailed(const QString &jid, const QString &msgid);
    void downloadProgress(const QString &jid, float progress, const QString &msgid);

    void groupParticipantAdded(const QString &gjid, const QString &jid);
    void groupParticipantRemoved(const QString &gjid, const QString &jid);
    void groupParticipantPromoted(const QString &gjid, const QString &jid);
    void groupParticipantDemoted(const QString &gjid, const QString &jid);
    void groupSubjectChanged(const QString &gjid, const QString &subject, const QString &s_o, const QString &s_t);
    void groupCreated(const QString &gjid, const QString &creation, const QString &creator, const QString &s_o, const QString &s_t, const QString &subject, const QStringList &participants, const QStringList &admins);
};

#endif // MAINCONNECTION_H
