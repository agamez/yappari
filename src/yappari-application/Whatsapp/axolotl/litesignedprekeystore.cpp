#include "litesignedprekeystore.h"
#include "../libaxolotl/whisperexception.h"

#include <QVariant>

LiteSignedPreKeyStore::LiteSignedPreKeyStore(const QSqlDatabase &db)
{
    _db = db;
    _db.exec("CREATE TABLE IF NOT EXISTS signed_prekeys (_id INTEGER PRIMARY KEY AUTOINCREMENT, prekey_id INTEGER UNIQUE, timestamp INTEGER, record BLOB);");
}

void LiteSignedPreKeyStore::clear()
{
    _db.exec("DELETE FROM signed_prekeys;");
}

SignedPreKeyRecord LiteSignedPreKeyStore::loadSignedPreKey(qulonglong signedPreKeyId)
{
    QSqlQuery q(_db);
    q.prepare("SELECT record FROM signed_prekeys WHERE prekey_id=(:prekey_id);");
    q.bindValue(":prekey_id", QVariant::fromValue(signedPreKeyId));
    q.exec();

    if (q.next()) {
        QByteArray serialized = q.value(0).toByteArray();
        SignedPreKeyRecord record(serialized);
        return record;
    }
    else {
        throw WhisperException(QString("No such signedprekeyrecord! %1").arg(signedPreKeyId));
    }
}

QList<SignedPreKeyRecord> LiteSignedPreKeyStore::loadSignedPreKeys()
{
    QSqlQuery q(_db);
    QList<SignedPreKeyRecord> recordsList;
    q.exec("SELECT record FROM signed_prekeys;");
    while (q.next()) {
        QByteArray serialized = q.value(0).toByteArray();
        SignedPreKeyRecord record(serialized);
        recordsList.append(record);
    }
    return recordsList;
}

void LiteSignedPreKeyStore::storeSignedPreKey(qulonglong signedPreKeyId, const SignedPreKeyRecord &record)
{
    QSqlQuery q(_db);
    q.prepare("INSERT INTO signed_prekeys VALUES (NULL, :prekey_id, :timestamp, :record);");
    q.bindValue(":prekey_id", QVariant::fromValue(signedPreKeyId));
    q.bindValue(":timestamp", 0);
    q.bindValue(":record", record.serialize());
    q.exec();
}

bool LiteSignedPreKeyStore::containsSignedPreKey(qulonglong signedPreKeyId)
{
    QSqlQuery q(_db);
    q.prepare("SELECT record FROM signed_prekeys WHERE prekey_id=(:prekey_id);");
    q.bindValue("prekey_id", QVariant::fromValue(signedPreKeyId));
    q.exec();
    return q.next();
}

void LiteSignedPreKeyStore::removeSignedPreKey(qulonglong signedPreKeyId)
{
    QSqlQuery q(_db);
    q.prepare("DELETE FROM signed_prekeys WHERE prekey_id=(:prekey_id);");
    q.bindValue("prekey_id", QVariant::fromValue(signedPreKeyId));
    q.exec();
}
