#ifndef LITESIGNEDPREKEYSTORE_H
#define LITESIGNEDPREKEYSTORE_H

#include "../libaxolotl/state/signedprekeystore.h"
#include "../libaxolotl/state/signedprekeyrecord.h"

#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>

class LiteSignedPreKeyStore : public SignedPreKeyStore
{
public:
    LiteSignedPreKeyStore(const QSqlDatabase &db);
    void clear();

    SignedPreKeyRecord loadSignedPreKey(qulonglong signedPreKeyId) ;
    QList<SignedPreKeyRecord> loadSignedPreKeys();
    void storeSignedPreKey(qulonglong signedPreKeyId, const SignedPreKeyRecord &record);
    bool containsSignedPreKey(qulonglong signedPreKeyId);
    void removeSignedPreKey(qulonglong signedPreKeyId);

private:
    QSqlDatabase _db;
};

#endif // LITESIGNEDPREKEYSTORE_H
