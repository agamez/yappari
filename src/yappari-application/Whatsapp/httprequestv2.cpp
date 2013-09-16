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


#include <QStringList>
#include <QThread>

#include "httprequestv2.h"

#include "util/utilities.h"

#include "globalconstants.h"

#define MAX_BUFFER      5120

HttpRequestv2::HttpRequestv2(QObject *parent) :
    QObject(parent)
{
    // Add default headers
    setHeader("User-Agent", QString(USER_AGENT).toUtf8());
    setHeader("Connection", "closed");

}

void HttpRequestv2::setHeader(QString header, QString value)
{
    headers.insert(header,value);
}

QByteArray HttpRequestv2::readAll()
{
    return socket->readAll();
}

QString HttpRequestv2::getHeader(QString header)
{
    return headers.value(header);
}


void HttpRequestv2::get(QUrl url)
{
    // Configure the GET connection
    this->url = url;
    this->method = GET;

    connectToHost();
}

void HttpRequestv2::post(QUrl url, const char *data, int length)
{
    // Configure the POST connection
    this->url = url;
    this->data = data;
    this->length = length;
    this->method = POST;

    connectToHost();
}

void HttpRequestv2::connectToHost() {

    // Connect the socket
    socket = new QSslSocket(this);

    connect(socket, SIGNAL(encrypted()), this, SLOT(sendRequest()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketErrorHandler(QAbstractSocket::SocketError)));

#ifdef Q_WS_SCRATCHBOX
    socket->setPeerVerifyMode(QSslSocket::VerifyNone);
#endif
    socket->connectToHostEncrypted(url.host(), url.port(443));
    Utilities::logData("HttpRequest(): Connecting to " + url.host() + ":" + QString::number(url.port(443)));
}

void HttpRequestv2::sendRequest() {

    Utilities::logData("HttpRequest(): Sending request...");

    // Send the request
    QByteArray *request = new QByteArray();

    request->append((method == GET) ? "GET" : "POST");
    request->append(' ');
    request->append(url.encodedPath());
    if (method == GET)
        request->append("?" + url.encodedQuery());
    request->append(" HTTP/1.1\r\n");
    request->append("Host: " + url.encodedHost() + "\r\n");

    // Add all the headers
    QList<QString> keys = headers.keys();

    foreach (QString key, keys)
        request->append(key.toUtf8() + ": " + headers.value(key).toUtf8() + "\r\n");

    if (method == POST)
        request->append("Content-Length: " + QString::number(length) + "\r\n");

    request->append("\r\n");

    if (method == POST)
        request->append(data,length);

    // Write data
    connect(socket, SIGNAL(encryptedBytesWritten(qint64)),
            this, SLOT(encryptedBytesWritten(qint64)));

    bytesWritten = 0;
    qint64 length = request->size();
    qint64 pos = 0;
    const char *constData = request->constData();
    while (pos < length)
    {
        int size = ((pos + MAX_BUFFER) < length ? MAX_BUFFER : length-pos);
        socket->write(constData, size);
        constData += size;
        pos += size;
    }

    delete request;

    emit requestSent(length);

    Utilities::logData("HttpRequest(): Waiting for response...");
    connect(socket,SIGNAL(readyRead()),this,SLOT(readResponse()));
}

void HttpRequestv2::workerFinished()
{
    emit requestSent(length);
}

void HttpRequestv2::readResponse() {

    Utilities::logData("HttpRequest(): Got response");

    // This function is only called once
    disconnect(socket,SIGNAL(readyRead()),this,SLOT(readResponse()));

    // Prepare the buffer
    QByteArray buffer;
    buffer.resize(MAX_BUFFER);

    // Read response
    qint64 length = socket->readLine(buffer.data(), MAX_BUFFER);
    qint64 totalLength = length;
    QString response = QString::fromUtf8(buffer.data());
    Utilities::logData(response.trimmed());

    errorCode = 0;
    if (response.left(8) != "HTTP/1.1")
    {
        emit headersReceived(totalLength);
        emit finished();
        return;
    }

    int space = response.indexOf(' ', 9);
    if (space < 0)
    {
        emit headersReceived(totalLength);
        emit finished();
        return;
    }

    errorCode = response.mid(9,(space-9)).toInt();
    if (errorCode != 200)
    {
        emit headersReceived(totalLength);
        emit finished();
        return;
    }

    // Read all headers
    headers.clear();
    bool end = false;
    while (socket->bytesAvailable() >= 0 && end == false)
    {
        if (socket->bytesAvailable() == 0)
            socket->waitForReadyRead();

        if (socket->bytesAvailable() > 0)
        {
            length = socket->readLine(buffer.data(), MAX_BUFFER);
            totalLength += length;
            QString header = QString::fromUtf8(buffer.data());

            Utilities::logData(header.trimmed());

            // A blank line means the end of headers and beginning of data
            if (header == "\r\n")
            {
                Utilities::logData("HttpRequest(): All headers read.  Available: " + QString::number(socket->bytesAvailable()));
                end = true;
            }
            else
            {
                QStringList list = header.trimmed().split(": ");
                setHeader(list[0],list[1]);
            }
        }
    }

    if (end)
    {
        emit headersReceived(totalLength);
        emit finished();
    }
}

void HttpRequestv2::socketErrorHandler(QAbstractSocket::SocketError err)
{
    Utilities::logData("HttpRequestv2() Socket Error " + QString::number(err));

    if (socket->isOpen())
        socket->close();

    emit socketError(err);
}

void HttpRequestv2::clearHeaders()
{
    headers.clear();
}

void HttpRequestv2::encryptedBytesWritten(qint64 written)
{
    bytesWritten += written;
    float p = ((float) bytesWritten * 100.0) / ((float)length);

    emit progress(p);
}

