#include "regtools.h"
#include "constants.h"

#include <QCryptographicHash>
#include <QTime>
#include <QStringList>
#include <QDebug>

QString RegTools::getToken(const QString &phone, const QString &platform)
{
    qDebug() << QString("Constructing %1 token for %2").arg(platform).arg(phone);
    if (platform == "S40") {
        QString scratch = QString(S40_KEY) + QString(S40_BUILD) + phone;
        QByteArray bytes = QCryptographicHash::hash(scratch.toLatin1(), QCryptographicHash::Md5);
        return QString(bytes.toHex());
    }
    else if (platform == "Android") {
        QByteArray ipad = QByteArray::fromBase64(QByteArray(ANDROID_S1));
        QByteArray opad = QByteArray::fromBase64(QByteArray(ANDROID_S2));
        QByteArray data = QByteArray::fromBase64(QByteArray(ANDROID_S3));
        data.append(QByteArray::fromBase64(QByteArray(ANDROID_KEY)));
        data.append(phone.toLatin1());
        ipad.append(data);
        opad.append(QCryptographicHash::hash(ipad, QCryptographicHash::Sha1));
        return QCryptographicHash::hash(opad, QCryptographicHash::Sha1).toBase64();
    }

    return "";
}


QString RegTools::getId(const QString &id, const QString &phone)
{
    QCryptographicHash digest(QCryptographicHash::Md5);
    digest.addData(id.toLatin1());
    digest.addData("yapari");
    digest.addData(phone.toLatin1());
    return digest.result().toHex();
}

QString RegTools::getDevice(const QString &platform)
{
    QStringList devicesList;
    if (platform == "S40") {
        QTime midnight(0,0,0);
        qsrand(midnight.secsTo(QTime::currentTime()));

        devicesList << "Nokia2220s/2.0";
        devicesList << "Nokia3610/1.0";
        devicesList << "Nokia5130/2.0";
        devicesList << "Nokia5330-1d/5.0";
        devicesList << "Nokia6260s-1/2.0";
        devicesList << "Nokia6303ci/5.0";
        devicesList << "Nokia6700c-1/2.0";
        devicesList << "Nokia7100s-2/1.0";
        devicesList << "Nokia109/04.10";
        devicesList << "Nokia200/11.81";
        devicesList << "Nokia200/11.95";
        devicesList << "Nokia201/11.21";
        devicesList << "Nokia202/mad20.22";
        devicesList << "Nokia203/20.26";
        devicesList << "Nokia205/04.51";
        devicesList << "Nokia2055/03.20";
        devicesList << "Nokia206/ad3.59";
        devicesList << "Nokia208/03.39";
        devicesList << "Nokia210/04.12";
        devicesList << "Nokia300/06.97";
        devicesList << "Nokia301/pk104_11w23";
        devicesList << "Nokia302/gadCEX3.60";
        devicesList << "Nokia303/01.00";
        devicesList << "Nokia305/gadCEX3.60";
        devicesList << "Nokia306/2.0";
        devicesList << "Nokia308/tnd5.47";
        devicesList << "Nokia309/tn5.72";
        devicesList << "Nokia310/2.0";
        devicesList << "Nokia311/hwi3.24";
        devicesList << "Nokia311/03.81";
        devicesList << "Nokia500/010.029";
        devicesList << "Nokia501/1.0";
        devicesList << "Nokia502/11.0.8";
        devicesList << "Nokia503/1.1339.0";
        devicesList << "Nokia515/2.0";
        devicesList << "Nokia515.2/05.08";
        devicesList << "NokiaC2-01/10.50";
        devicesList << "NokiaC2-05/bx8.05";
        devicesList << "NokiaC3-01/05.68";
        devicesList << "NokiaX2-00/5.0";
        devicesList << "NokiaX2-01/5.0";
        devicesList << "NokiaX2-01/07.10";
        devicesList << "NokiaX2-05/08.35";
        devicesList << "NokiaX3-02/le6.32";

        return devicesList.at(qrand() % devicesList.size());
    }
    else if (platform == "Android") {
        return "unknown-Jolla/4.1.2";
    }
    return "";
}

QStringList RegTools::getServers()
{
    QStringList serversList;
    //serversList << "c.whatsapp.net";
    //serversList << "c1.whatsapp.net";
    //serversList << "c2.whatsapp.net";
    //serversList << "c3.whatsapp.net";
    serversList << "e1.whatsapp.net";
    serversList << "e2.whatsapp.net";
    serversList << "e3.whatsapp.net";
    serversList << "e4.whatsapp.net";
    serversList << "e5.whatsapp.net";
    serversList << "e6.whatsapp.net";
    serversList << "e7.whatsapp.net";
    serversList << "e8.whatsapp.net";
    serversList << "e9.whatsapp.net";
    serversList << "e10.whatsapp.net";
    serversList << "e11.whatsapp.net";
    serversList << "e12.whatsapp.net";
    serversList << "e13.whatsapp.net";
    serversList << "e14.whatsapp.net";
    serversList << "e15.whatsapp.net";
    serversList << "e16.whatsapp.net";

    return serversList;
}

QString RegTools::getUseragent(const QString &device, const QString &platform)
{
    QString version = platform == "S40" ? S40_VERSION : ANDROID_VERSION;
    return QString(USERAGENT_FMT_STRING).arg(device.split("/").first()).arg(device.split("/").last()).arg(version).arg(platform == "S40" ? "S40Version" : platform);
}

QString RegTools::getResource(const QString &platform)
{
    return platform == "S40" ? S40_RESOURCE : ANDROID_RESOURCE;
}

QString RegTools::getEncryptionAV(const QString &platform)
{
    return QString("%1/%2").arg(platform).arg(platform == "S40" ? S40_VERSION : ANDROID_VERSION);
}
