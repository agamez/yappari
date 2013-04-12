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

#include <QDateTime>
#include <QDomDocument>
#include "multipartuploader.h"
#include "client.h"

#include "qt-json/json.h"

#include "util/utilities.h"

MultiPartUploader::MultiPartUploader(QObject *parent)
    : HttpRequest(parent)
{
    connect(&manager,SIGNAL(finished(QNetworkReply*)),
            this,SLOT(onResponse(QNetworkReply*)));
}

QString MultiPartUploader::generateBoundary()
{
    QString boundary = QString::number(QDateTime::currentMSecsSinceEpoch()) +
                       QString::number(Client::seq);

    return boundary;
}

void MultiPartUploader::error(QNetworkReply::NetworkError error)
{
    QVariantMap map;
    QString errorStr = "Error: " + QString::number(error);
    Utilities::logData(errorStr);
    map.insert("error",errorStr);
    emit finished(this, map);
}


void MultiPartUploader::onResponse(QNetworkReply *reply)
{
    QString jsonStr = QString::fromUtf8(reply->readAll().constData());
    disconnect(reply, 0, 0, 0);
    reply->deleteLater();
    Utilities::logData("Reply: " + jsonStr);

    bool ok;
    QVariantMap mapResult = QtJson::parse(jsonStr, ok).toMap();

    emit finished(this, mapResult);
}

void MultiPartUploader::open(QString url, QList<FormData*>& formData)
{
    QString boundary = generateBoundary();

    postData(boundary,formData);

    Utilities::logData("Content Length: " + QString::number(writeBuffer.size()));
    // Utilities::logData(QString::fromLatin1(writeBuffer.data()));

    get(QUrl(url), true, boundary);
}


void MultiPartUploader::get(QUrl url, bool post, QString boundary)
{
    QNetworkRequest req;
    configureRequest(req,url);

    req.setRawHeader("Content-Type",
                     QString("multipart/form-data; boundary="+boundary).toUtf8());

    QNetworkReply *reply;

    reply = (post) ? manager.post(req,writeBuffer) : manager.get(req);

    connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),
            this,SLOT(error(QNetworkReply::NetworkError)));
}


void MultiPartUploader::postData(QString boundary, QList<FormData*> &formData)
{
    qint64 totalLength = 0;

    writeBuffer.clear();

    for (int i = 0; i < formData.length(); i++)
    {
        // totalLength += formData.at(i).length();

        FormData *data = formData.at(i);

        totalLength += data->length();

        writeBuffer.append("--");
        writeBuffer.append(boundary);
        writeBuffer.append("\r\n");
        writeBuffer.append("Content-Disposition: ");
        writeBuffer.append("form-data");
        writeBuffer.append("; name=\"");
        writeBuffer.append(data->name);
        writeBuffer.append("\"");

        if (!data->fileName.isEmpty())
        {
            writeBuffer.append("; filename=\"");
            writeBuffer.append(data->fileName);
            writeBuffer.append("\"");
        }
        writeBuffer.append("\r\n");

        if (!data->contentType.isEmpty())
        {
            writeBuffer.append("Content-Type: ");
            writeBuffer.append(data->contentType);
            writeBuffer.append("\r\n");
        }
        writeBuffer.append("\r\n");

        QBuffer dataBuffer;
        dataBuffer.open(QBuffer::WriteOnly);
        data->write(dataBuffer);

        writeBuffer.append(dataBuffer.data());
        writeBuffer.append("\r\n");

        delete data;
    }

    writeBuffer.append("--" + boundary + "--\r\n");
}

