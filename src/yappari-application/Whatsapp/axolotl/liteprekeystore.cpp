#include "liteprekeystore.h"
#include "../libaxolotl/whisperexception.h"

#include <QVariant>

LitePreKeyStore::LitePreKeyStore(const QSqlDatabase &db)
{
    _db = db;
    _db.exec("CREATE TABLE IF NOT EXISTS prekeys (_id INTEGER PRIMARY KEY AUTOINCREMENT, prekey_id INTEGER UNIQUE, sent_to_server BOOLEAN, record BLOB);");
}

void LitePreKeyStore::clear()
{
    _db.exec("DELETE FROM prekeys;");
}

PreKeyRecord LitePreKeyStore::loadPreKey(qulonglong preKeyId)
{
    QSqlQuery q(_db);
    q.prepare("SELECT record FROM prekeys WHERE prekey_id=(:prekey_id);");
    q.bindValue(":prekey_id", QVariant::fromValue(preKeyId));
    q.exec();
    if (q.next()) {
        QByteArray serialized = q.value(0).toByteArray();
        PreKeyRecord record(serialized);
        return record;
    }
    else {
        throw WhisperException(QString("No such prekeyRecord! %1").arg(preKeyId));
    }
}

void LitePreKeyStore::storePreKey(qulonglong preKeyId, const PreKeyRecord &record)
{
    QSqlQuery q(_db);
    q.prepare("INSERT INTO prekeys VALUES(NULL, :prekey_id, :sent_to_server, :record);");
    q.bindValue(":prekey_id", QVariant::fromValue(preKeyId));
    q.bindValue(":sent_to_server", false);
    q.bindValue(":record", record.serialize());
    q.exec();
}

bool LitePreKeyStore::containsPreKey(qulonglong preKeyId)
{
    QSqlQuery q(_db);
    q.prepare("SELECT record FROM prekeys WHERE prekey_id=(:prekey_id);");
    q.bindValue(":prekey_id", QVariant::fromValue(preKeyId));
    q.exec();
    return q.next();
}

void LitePreKeyStore::removePreKey(qulonglong preKeyId)
{
    QSqlQuery q(_db);
    q.prepare("DELETE FROM prekeys WHERE prekey_id=(:prekey_id);");
    q.bindValue(":prekey_id", QVariant::fromValue(preKeyId));
    q.exec();
}

int LitePreKeyStore::countPreKeys()
{
    QSqlQuery q("SELECT COUNT(*) FROM prekeys;", _db);
    q.exec();
    if (q.next()) {
        return q.value(0).toInt();
    }
    return 0;
}
