#include "litesessionstore.h"

#include <QVariant>
#include <QDebug>

LiteSessionStore::LiteSessionStore(const QSqlDatabase &db)
{
    _db = db;
    _db.exec("CREATE TABLE IF NOT EXISTS sessions (_id INTEGER PRIMARY KEY AUTOINCREMENT, recipient_id INTEGER UNIQUE, device_id INTEGER, record BLOB, timestamp INTEGER);");
}

void LiteSessionStore::clear()
{
    _db.exec("DELETE FROM sessions;");
}

SessionRecord *LiteSessionStore::loadSession(qulonglong recipientId, int deviceId)
{
    QSqlQuery q(_db);
    q.prepare("SELECT record FROM sessions WHERE recipient_id=(:recipient_id) AND device_id=(:device_id);");
    q.bindValue(":recipient_id", QVariant::fromValue(recipientId));
    q.bindValue(":device_id", deviceId);
    q.exec();
    if (q.next()) {
        qDebug() << "Loaded session" << recipientId << deviceId;
        QByteArray serialized = q.value(0).toByteArray();
        return new SessionRecord(serialized);
    }
    else {
        qDebug() << "New session session" << recipientId << deviceId;
        return new SessionRecord();
    }
}

QList<int> LiteSessionStore::getSubDeviceSessions(qulonglong recipientId)
{
    QSqlQuery q(_db);
    QList<int> deviceIds;
    q.prepare("SELECT device_id from sessions WHERE recipient_id=(:recipient_id);");
    q.bindValue(":recipient_id", QVariant::fromValue(recipientId));
    q.exec();
    while (q.next()) {
        int deviceId = q.value(0).toInt();
        deviceIds.append(deviceId);
    }
    return deviceIds;
}

void LiteSessionStore::storeSession(qulonglong recipientId, int deviceId, SessionRecord *record)
{
    qDebug() << recipientId << deviceId;
    deleteSession(recipientId, deviceId);
    QSqlQuery q(_db);
    q.prepare("INSERT INTO sessions VALUES (NULL, :recipient_id, :device_id, :record, :timestamp);");
    q.bindValue(":recipient_id", QVariant::fromValue(recipientId));
    q.bindValue(":device_id", deviceId);
    q.bindValue(":record", record->serialize());
    q.bindValue(":timestamp", 0);
    q.exec();
}

bool LiteSessionStore::containsSession(qulonglong recipientId, int deviceId)
{
    qDebug() << recipientId << deviceId;
    QSqlQuery q(_db);
    q.prepare("SELECT record FROM sessions WHERE recipient_id=(:recipient_id) AND device_id=(:device_id);");
    q.bindValue(":recipient_id", QVariant::fromValue(recipientId));
    q.bindValue(":device_id", deviceId);
    q.exec();
    return q.next();
}

void LiteSessionStore::deleteSession(qulonglong recipientId, int deviceId)
{
    QSqlQuery q(_db);
    q.prepare("DELETE FROM sessions WHERE recipient_id=(:recipient_id) AND device_id=(:device_id);");
    q.bindValue(":recipient_id", QVariant::fromValue(recipientId));
    q.bindValue(":device_id", deviceId);
    q.exec();
}

void LiteSessionStore::deleteAllSessions(qulonglong recipientId)
{
    QSqlQuery q(_db);
    q.prepare("DELETE FROM sessions WHERE recipient_id=(:recipient_id);");
    q.bindValue(":recipient_id", QVariant::fromValue(recipientId));
    q.exec();
}
