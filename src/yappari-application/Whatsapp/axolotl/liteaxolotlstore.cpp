#include "liteaxolotlstore.h"

#include <QSqlError>
#include <QDebug>

LiteAxolotlStore::LiteAxolotlStore(const QString &connection)
{
    _db = QSqlDatabase::database(connection);
    if (_db.isOpen()) {
        qDebug() << "Axolotl active connection" << _db.databaseName();
        initStore();
    }
    else {
        _connection = connection;
    }
}

void LiteAxolotlStore::setDatabaseName(const QString &name)
{
    if (!_db.isOpen()) {
        qDebug() << "Axolotl using connection" << _connection;
        _db = QSqlDatabase::addDatabase("QSQLITE", _connection);
        qDebug() << "Axolotl database valid:" << _db.isValid();
        qDebug() << "Axolotl database name:" << name;
        _db.setDatabaseName(name);
        if (_db.open()) {
            qDebug() << "Axolotl database connected";
            initStore();
        }
        else {
            qWarning() << "Failed to open database" << _db.lastError().driverText() << _db.lastError().databaseText() << _db.lastError().text();
        }
    }
}

void LiteAxolotlStore::initStore()
{
    identityKeyStore = new LiteIdentityKeyStore(_db);
    preKeyStore = new LitePreKeyStore(_db);
    sessionStore = new LiteSessionStore(_db);
    signedPreKeyStore = new LiteSignedPreKeyStore(_db);
}

void LiteAxolotlStore::clear()
{
    identityKeyStore->clear();
    preKeyStore->clear();
    sessionStore->clear();
    signedPreKeyStore->clear();
}

IdentityKeyPair LiteAxolotlStore::getIdentityKeyPair()
{
    return identityKeyStore->getIdentityKeyPair();
}

uint LiteAxolotlStore::getLocalRegistrationId()
{
    return identityKeyStore->getLocalRegistrationId();
}

void LiteAxolotlStore::removeIdentity(qulonglong recipientId)
{
    identityKeyStore->removeIdentity(recipientId);
}

void LiteAxolotlStore::storeLocalData(qulonglong registrationId, const IdentityKeyPair identityKeyPair)
{
    identityKeyStore->storeLocalData(registrationId, identityKeyPair);
}

void LiteAxolotlStore::saveIdentity(qulonglong recipientId, const IdentityKey &identityKey)
{
    identityKeyStore->saveIdentity(recipientId, identityKey);
}

bool LiteAxolotlStore::isTrustedIdentity(qulonglong recipientId, const IdentityKey &identityKey)
{
    return identityKeyStore->isTrustedIdentity(recipientId, identityKey);
}

PreKeyRecord LiteAxolotlStore::loadPreKey(qulonglong preKeyId)
{
    return preKeyStore->loadPreKey(preKeyId);
}

void LiteAxolotlStore::storePreKey(qulonglong preKeyId, const PreKeyRecord &record)
{
    preKeyStore->storePreKey(preKeyId, record);
}

bool LiteAxolotlStore::containsPreKey(qulonglong preKeyId)
{
    return preKeyStore->containsPreKey(preKeyId);
}

void LiteAxolotlStore::removePreKey(qulonglong preKeyId)
{
    preKeyStore->removePreKey(preKeyId);
}

int LiteAxolotlStore::countPreKeys()
{
    return preKeyStore->countPreKeys();
}

SessionRecord *LiteAxolotlStore::loadSession(qulonglong recipientId, int deviceId)
{
    return sessionStore->loadSession(recipientId, deviceId);
}

QList<int> LiteAxolotlStore::getSubDeviceSessions(qulonglong recipientId)
{
    return sessionStore->getSubDeviceSessions(recipientId);
}

void LiteAxolotlStore::storeSession(qulonglong recipientId, int deviceId, SessionRecord *record)
{
    sessionStore->storeSession(recipientId, deviceId, record);
}

bool LiteAxolotlStore::containsSession(qulonglong recipientId, int deviceId)
{
    return sessionStore->containsSession(recipientId, deviceId);
}

void LiteAxolotlStore::deleteSession(qulonglong recipientId, int deviceId)
{
    sessionStore->deleteSession(recipientId, deviceId);
}

void LiteAxolotlStore::deleteAllSessions(qulonglong recipientId)
{
    sessionStore->deleteAllSessions(recipientId);
}

SignedPreKeyRecord LiteAxolotlStore::loadSignedPreKey(qulonglong signedPreKeyId)
{
    return signedPreKeyStore->loadSignedPreKey(signedPreKeyId);
}

QList<SignedPreKeyRecord> LiteAxolotlStore::loadSignedPreKeys()
{
    return signedPreKeyStore->loadSignedPreKeys();
}

void LiteAxolotlStore::storeSignedPreKey(qulonglong signedPreKeyId, const SignedPreKeyRecord &record)
{
    signedPreKeyStore->storeSignedPreKey(signedPreKeyId, record);
}

bool LiteAxolotlStore::containsSignedPreKey(qulonglong signedPreKeyId)
{
    return signedPreKeyStore->containsSignedPreKey(signedPreKeyId);
}

void LiteAxolotlStore::removeSignedPreKey(qulonglong signedPreKeyId)
{
    signedPreKeyStore->removeSignedPreKey(signedPreKeyId);
}
