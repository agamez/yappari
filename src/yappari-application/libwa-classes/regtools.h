#ifndef REGTOOLS_H
#define REGTOOLS_H

#include <QString>

class RegTools
{
public:
    static QString getToken(const QString &phone, const QString &platform);
    static QString getId(const QString &id, const QString &phone);
    static QString getDevice(const QString &platform);
    static QStringList getServers();
    static QString getUseragent(const QString &device, const QString &platform);
    static QString getResource(const QString &platform);
    static QString getEncryptionAV(const QString &platform);
};

#endif // REGTOOLS_H
