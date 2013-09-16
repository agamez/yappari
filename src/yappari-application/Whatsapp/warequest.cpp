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
 * are those of the author and should not be interpreted as representing
 * official policies, either expressed or implied, of the copyright holder.
 */

#include <QSslConfiguration>

#include "protocolexception.h"
#include "warequest.h"
#include "globalconstants.h"

#include "util/utilities.h"
#include "qt-json/json.h"

WARequest::WARequest(QObject *parent) : HttpRequestv2(parent)
{
}

void WARequest::getRequest()
{

#ifndef TEST
    QString url = URL_REGISTRATION_V2 + method +
                  "?" + QString::fromUtf8(writeBuffer.constData());
#else
    QString url;

    if (method == "exist")
        url = "https://192.168.100.103/v2/exist?";
    if (method == "code")
        url = "https://192.168.100.103/v2/code?";
    if (method == "register")
        url = "https://192.168.100.103/v2/register?";

    url += QString::fromUtf8(writeBuffer.constData());
#endif

    // Be professional
    if (url.right(1) == "&")
        url = url.left(url.length()-1);

    Utilities::logData("Request: " + url);

    connect(this,SIGNAL(finished()),
            this,SLOT(onResponse()));

    connect(this,SIGNAL(socketError(QAbstractSocket::SocketError)),
            this,SLOT(errorHandler(QAbstractSocket::SocketError)));

    get(url);
}

void WARequest::onResponse()
{
    if (errorCode != 200)
    {
        emit httpError(this, errorCode);
        return;
    }

    if (socket->bytesAvailable())
        readResult();

    connect(socket,SIGNAL(readyRead()),this,SLOT(readResult()));
}

void WARequest::readResult()
{
    QString jsonStr = QString::fromUtf8(socket->readAll().constData());

    // Debugging info
    Utilities::logData("Reply: " + jsonStr);

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

void WARequest::errorHandler(QAbstractSocket::SocketError error)
{
    if (error == QAbstractSocket::SslHandshakeFailedError)
    {
        // SSL error is a fatal error
        emit sslError(this);
    }
    else
    {
        // ToDo: Retry here
        Utilities::logData("Registration failed: Socket error " + QString::number(error));
        emit httpError(this, error);
    }
}
