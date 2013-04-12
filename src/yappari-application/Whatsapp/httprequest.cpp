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

#include "httprequest.h"
#include "globalconstants.h"

#include "util/utilities.h"

HttpRequest::HttpRequest(QObject *parent) :
    QObject(parent)
{
    this->userAgent = USER_AGENT;
}

void HttpRequest::error(QNetworkReply::NetworkError error)
{
    Utilities::logData("Error: " + QString::number(error));
}

void HttpRequest::onResponse(QNetworkReply *reply)
{
    Q_UNUSED(reply);
}

void HttpRequest::configureRequest(QNetworkRequest& req, QUrl url)
{
    req.setUrl(QUrl(url));
    req.setRawHeader("User-Agent", userAgent.toUtf8());
    req.setRawHeader("Connection", "close");

    QList<QString> keys = headers.keys();

    foreach (QString key, keys)
    {
        req.setRawHeader(key.toUtf8(), headers.value(key).toUtf8());
    }

    headers.clear();

#ifdef Q_WS_SCRATCHBOX
    QSslConfiguration sslconf;
    sslconf.setPeerVerifyMode(QSslSocket::VerifyNone);
    req.setSslConfiguration(sslconf);
#endif

}

void HttpRequest::get(QUrl url, bool post)
{
    QNetworkRequest req;
    configureRequest(req,url);

    QNetworkReply *reply;

    reply = (post) ? manager.post(req,writeBuffer) :manager.get(req);

    connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),
            this,SLOT(error(QNetworkReply::NetworkError)));
}

void HttpRequest::setUserAgent(QString userAgent)
{
    this->userAgent = userAgent;
}

void HttpRequest::addHeader(QString header, QString value)
{
    headers.insert(header,value);
}
