#ifndef LITESESSIONSTORE_H
#define LITESESSIONSTORE_H

#include "../libaxolotl/state/sessionstore.h"

#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>

class LiteSessionStore : public SessionStore
{
public:
    LiteSessionStore(const QSqlDatabase &db);
    void clear();

    SessionRecord *loadSession(qulonglong recipientId, int deviceId);
    QList<int> getSubDeviceSessions(qulonglong recipientId);
    void storeSession(qulonglong recipientId, int deviceId, SessionRecord *record);
    bool containsSession(qulonglong recipientId, int deviceId);
    void deleteSession(qulonglong recipientId, int deviceId);
    void deleteAllSessions(qulonglong recipientId);

private:
    QSqlDatabase _db;
};

#endif // LITESESSIONSTORE_H
