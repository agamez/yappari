/* Copyright 2013 Naikel Aparicio. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL EELI REILIN OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Eeli Reilin.
 */

#include <QSslConfiguration>
#include <QDomDocument>
#include <QDomElement>

#include "protocolexception.h"
#include "warequest.h"
#include "globalconstants.h"

#include "util/utilities.h"
#include "qt-json/json.h"

WARequest::WARequest(QObject *parent) : HttpRequest(parent)
{
    connect(&manager,SIGNAL(finished(QNetworkReply*)),
            this,SLOT(onResponse(QNetworkReply*)));
}

void WARequest::getRequest()
{

#ifndef TEST
    QString url = URL_REGISTRATION_V2 + method +
                  "?" + QString::fromUtf8(writeBuffer.constData());


#else
    QString url;

    if (method == "exist")
        url = "http://192.168.100.100:9090/fail-exists.xml?";
    if (method == "code")
        url = "http://192.168.100.100:9090/fail-too-recent.xml?";
    if (method == "register")
        url = "http://192.168.100.100:9090/success-registration.xml?";

    url += QString::fromUtf8(writeBuffer.constData());
#endif

    Utilities::logData("Request: " + url);

    get(url,false);
}

void WARequest::onResponse(QNetworkReply *reply)
{
    QString jsonStr = QString::fromUtf8(reply->readAll().constData());
    disconnect(reply, 0, 0, 0);
    reply->deleteLater();

    // Debugging info
    Utilities::logData("Reply: " + jsonStr);
    QList<QByteArray> headers = reply->rawHeaderList();

    for (int i = 0; i < headers.length(); i++)
    {
        Utilities::logData(QString::fromLatin1(headers.at(i).constData()) + ": " +
                           QString::fromLatin1(reply->rawHeader(headers.at(i)).constData()));
    }
    // End of debugging info

    bool ok;
    QVariantMap mapResult = QtJson::parse(jsonStr, ok).toMap();

    emit finished(this, ok, mapResult);
}

void WARequest::addParam(QString name, QString value)
{
    writeBuffer.append(QUrl::toPercentEncoding(name));
    writeBuffer.append('=');
    writeBuffer.append(QUrl::toPercentEncoding(value));
    writeBuffer.append('&');
}
