#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <QObject>

#include <libwa-qt4/waregistration.h>

class Registration : public QObject
{
    Q_OBJECT
public:
    explicit Registration(QObject *parent = 0);

    Q_INVOKABLE void codeRequest(const QString &cc, const QString &phone, const QString &mcc, const QString &mnc, const QString &method);
    Q_INVOKABLE void enterCode(const QString &cc, const QString &phone, const QString &mcc, const QString &mnc, const QString &code);

signals:
    void registrationSuccessful();
    void registrationReply(const QVariantMap &reply);
    void codeReceived(const QString &code);

private slots:
    void onRegReply(const QVariantMap &result);

private:
    WARegistration *wareg;

    QString m_id;
    QString m_device;
    QString m_useragent;

};

#endif // REGISTRATION_H
