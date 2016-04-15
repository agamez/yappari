#include "registration.h"
#include "constants.h"

#include "regtools.h"
//#include "platform/smslistener.h"

#include <QCryptographicHash>
#include <QUuid>
#include <QDebug>

Registration::Registration(QObject *parent) :
    QObject(parent)
{
    //m_id = AccountSettings::GetInstance()->value("id", QString()).toString();
    if (m_id.isEmpty()) {
        m_id = QCryptographicHash::hash(QUuid::createUuid().toString().toAscii(), QCryptographicHash::Md5).toHex();
        //AccountSettings::GetInstance()->setValue("id", m_id);
    }

    //m_device = AccountSettings::GetInstance()->value("device", QString()).toString();
    if (m_device.isEmpty()) {
        m_device = RegTools::getDevice("S40");
        //AccountSettings::GetInstance()->setValue("device", m_device);
    }
    m_useragent = RegTools::getUseragent(m_device);

    qDebug() << "Registering with m_id:" << m_id << "as" << m_device << m_useragent;
    wareg = new WARegistration(this);
    QObject::connect(wareg, SIGNAL(finished(QVariantMap)), this, SLOT(onRegReply(QVariantMap)));
}

void Registration::enterCode(const QString &cc, const QString &phone, const QString &mcc, const QString &mnc, const QString &code)
{
    wareg->init(cc, phone, mcc, mnc, RegTools::getId(m_id, phone), m_useragent);
    wareg->enterCode(code);
}

void Registration::codeRequest(const QString &cc, const QString &phone, const QString &mcc, const QString &mnc, const QString &method)
{
    wareg->init(cc, phone, mcc, mnc, RegTools::getId(m_id, phone), m_useragent, method, RegTools::getToken(phone));
    wareg->codeRequest();
}

void Registration::onRegReply(const QVariantMap &result)
{
    qDebug() << result;
    QString status = result["status"].toString();
    if (status == "ok")
    {
        //AccountSettings::GetInstance()->setValue("login", result["login"]);
        //AccountSettings::GetInstance()->setValue("password", result["pw"]);
        //AccountSettings::GetInstance()->setValue("kind", result["kind"]);
        //AccountSettings::GetInstance()->setValue("expiration", result["expiration"]);
        Q_EMIT registrationSuccessful();
    }
    else {
        Q_EMIT registrationReply(result);

        if (status == "sent") {
              qDebug() << "SMS SENT";
//            SMSListener *smsListener = new SMSListener(this);
//            QObject::connect(smsListener, SIGNAL(codeReceived(QString)), this, SIGNAL(codeReceived(QString)));
        }
    }
}
