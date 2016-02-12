#ifndef LITEAXOLOTLSTORE_H
#define LITEAXOLOTLSTORE_H

#include "../libaxolotl/state/axolotlstore.h"
#include "liteidentitykeystore.h"
#include "liteprekeystore.h"
#include "litesessionstore.h"
#include "litesignedprekeystore.h"

#include <QSqlDatabase>

class LiteAxolotlStore : public AxolotlStore
{
public:
    LiteAxolotlStore(const QString &connection);
    void setDatabaseName(const QString &name);
    void clear();

    IdentityKeyPair getIdentityKeyPair();
    uint            getLocalRegistrationId();
    void            storeLocalData(qulonglong registrationId, const IdentityKeyPair identityKeyPair);
    void            saveIdentity(qulonglong recipientId, const IdentityKey &identityKey);
    bool            isTrustedIdentity(qulonglong recipientId, const IdentityKey &identityKey);
    void            removeIdentity(qulonglong recipientId);

    PreKeyRecord loadPreKey(qulonglong preKeyId);
    void         storePreKey(qulonglong preKeyId, const PreKeyRecord &record);
    bool         containsPreKey(qulonglong preKeyId);
    void         removePreKey(qulonglong preKeyId);
    int          countPreKeys();

    SessionRecord *loadSession(qulonglong recipientId, int deviceId);
    QList<int>     getSubDeviceSessions(qulonglong recipientId);
    void           storeSession(qulonglong recipientId, int deviceId, SessionRecord *record);
    bool           containsSession(qulonglong recipientId, int deviceId);
    void           deleteSession(qulonglong recipientId, int deviceId);
    void           deleteAllSessions(qulonglong recipientId);

    SignedPreKeyRecord        loadSignedPreKey(qulonglong signedPreKeyId);
    QList<SignedPreKeyRecord> loadSignedPreKeys();
    void                      storeSignedPreKey(qulonglong signedPreKeyId, const SignedPreKeyRecord &record);
    bool                      containsSignedPreKey(qulonglong signedPreKeyId);
    void                      removeSignedPreKey(qulonglong signedPreKeyId);

private:
    void initStore();

    QSqlDatabase _db;
    QString _connection;

    LiteIdentityKeyStore    *identityKeyStore;
    LitePreKeyStore         *preKeyStore;
    LiteSessionStore        *sessionStore;
    LiteSignedPreKeyStore   *signedPreKeyStore;
};

#endif // LITEAXOLOTLSTORE_H
