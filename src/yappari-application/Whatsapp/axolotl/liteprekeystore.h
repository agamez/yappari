#ifndef LITEPREKEYSTORE_H
#define LITEPREKEYSTORE_H

#include "../libaxolotl/state/prekeystore.h"
#include "../libaxolotl/state/prekeyrecord.h"

#include <QSqlDatabase>
#include <QSqlQuery>

class LitePreKeyStore : public PreKeyStore
{
public:
    LitePreKeyStore(const QSqlDatabase &db);
    void clear();

    PreKeyRecord loadPreKey(qulonglong preKeyId);
    void         storePreKey(qulonglong preKeyId, const PreKeyRecord &record);
    bool         containsPreKey(qulonglong preKeyId);
    void         removePreKey(qulonglong preKeyId);
    int          countPreKeys();

private:
    QSqlDatabase _db;
};

#endif // LITEPREKEYSTORE_H
