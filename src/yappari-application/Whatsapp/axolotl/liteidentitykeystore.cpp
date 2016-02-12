#include "liteidentitykeystore.h"
#include "../libaxolotl/whisperexception.h"

#include <QVariant>
#include <QDebug>

LiteIdentityKeyStore::LiteIdentityKeyStore(const QSqlDatabase &db)
{
    _db = db;
    _db.exec("CREATE TABLE IF NOT EXISTS identities (_id INTEGER PRIMARY KEY AUTOINCREMENT, recipient_id INTEGER UNIQUE, registration_id INTEGER, public_key BLOB, private_key BLOB, next_prekey_id INTEGER, timestamp INTEGER);");
}

void LiteIdentityKeyStore::clear()
{
    _db.exec("DELETE FROM identities;");
}

IdentityKeyPair LiteIdentityKeyStore::getIdentityKeyPair()
{
    QSqlQuery q(_db);
    q.prepare("SELECT public_key, private_key FROM identities WHERE recipient_id = -1;");
    q.exec();
    if (q.next()) {
        QByteArray publicBytes = q.value(0).toByteArray().mid(1);
        DjbECPublicKey publicKey(publicBytes);
        IdentityKey publicIdentity(publicKey);
        QByteArray privateBytes = q.value(1).toByteArray();
        DjbECPrivateKey privateKey(privateBytes);
        IdentityKeyPair keypair(publicIdentity, privateKey);
        return keypair;
    }
    else {
        throw WhisperException("Can't get IdentityKeyPair!");
    }
}

uint LiteIdentityKeyStore::getLocalRegistrationId()
{
    QSqlQuery q(_db);
    q.prepare("SELECT registration_id FROM identities WHERE recipient_id = -1;");
    q.exec();
    if (q.next()) {
        return q.value(0).toUInt();
    }
    else {
        throw WhisperException("Can't get LocalRegistrationId!");
    }
}

void LiteIdentityKeyStore::removeIdentity(qulonglong recipientId)
{
    QSqlQuery q(_db);
    q.prepare("DELETE FROM identities WHERE recipient_id=(:recipient_id);");
    q.bindValue(":recipient_id", QVariant::fromValue(recipientId));
    q.exec();
}

void LiteIdentityKeyStore::storeLocalData(qulonglong registrationId, const IdentityKeyPair identityKeyPair)
{
    QSqlQuery q(_db);
    q.prepare("INSERT INTO identities(recipient_id, registration_id, public_key, private_key) VALUES(-1, :registration_id, :public_key, :private_key);");
    q.bindValue(":registration_id", QVariant::fromValue(registrationId));
    q.bindValue(":public_key", identityKeyPair.getPublicKey().getPublicKey().serialize());
    q.bindValue(":private_key", identityKeyPair.getPrivateKey().serialize());
    q.exec();
}

void LiteIdentityKeyStore::saveIdentity(qulonglong recipientId, const IdentityKey &identityKey)
{
    qDebug() << recipientId;
    QSqlQuery d(_db);
    d.prepare("DELETE FROM identities WHERE recipient_id=(:recipient_id);");
    d.bindValue(":recipient_id", QVariant::fromValue(recipientId));
    d.exec();

    QSqlQuery q(_db);
    q.prepare("INSERT INTO identities (recipient_id, public_key) VALUES(:recipient_id, :public_key);");
    q.bindValue(":recipient_id", QVariant::fromValue(recipientId));
    q.bindValue(":public_key", identityKey.getPublicKey().serialize());
    q.exec();
}

bool LiteIdentityKeyStore::isTrustedIdentity(qulonglong recipientId, const IdentityKey &identityKey)
{
    qDebug() << recipientId;
    QSqlQuery q(_db);
    q.prepare("SELECT public_key from identities WHERE recipient_id=(:recipient_id);");
    q.bindValue(":recipient_id", QVariant::fromValue(recipientId));
    q.exec();
    if (q.next()) {
        QByteArray publicKey = q.value(0).toByteArray();
        return publicKey == identityKey.getPublicKey().serialize();
    }
    else {
        return true;
    }
}
