#ifndef LITEIDENTITYKEYSTORE_H
#define LITEIDENTITYKEYSTORE_H

#include "../libaxolotl/state/identitykeystore.h"

#include <QSqlDatabase>
#include <QSqlQuery>

class LiteIdentityKeyStore : public IdentityKeyStore
{
public:
    LiteIdentityKeyStore(const QSqlDatabase &db);
    void clear();

    IdentityKeyPair getIdentityKeyPair();
    uint getLocalRegistrationId();
    void            storeLocalData(qulonglong registrationId, const IdentityKeyPair identityKeyPair);
    void            saveIdentity(qulonglong recipientId, const IdentityKey &identityKey);
    bool            isTrustedIdentity(qulonglong recipientId, const IdentityKey &identityKey);
    void            removeIdentity(qulonglong recipientId);

private:
    QSqlDatabase _db;
};

#endif // LITEIDENTITYKEYSTORE_H
