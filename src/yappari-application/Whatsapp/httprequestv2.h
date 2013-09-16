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


#ifndef HTTPREQUESTV2_H
#define HTTPREQUESTV2_H

#include <QSslSocket>
#include <QByteArray>
#include <QHash>
#include <QUrl>

class HttpRequestv2 : public QObject
{
    Q_OBJECT
public:

    enum Method {
        GET,
        POST
    };

    explicit HttpRequestv2(QObject *parent = 0);

    void get(QUrl url);
    void post(QUrl url, const char *data, int length);
    void setHeader(QString header, QString value);
    QByteArray readAll();
    QString getHeader(QString header);
    void clearHeaders();

signals:
    void finished();
    void socketError(QAbstractSocket::SocketError);
    void progress(float p);
    void requestSent(qint64 bytes);
    void headersReceived(qint64 bytes);

public slots:
    void sendRequest();
    void readResponse();
    void socketErrorHandler(QAbstractSocket::SocketError err);
    void workerFinished();
    void encryptedBytesWritten(qint64 bytesWritten);

private:
    QUrl url;
    Method method;
    QHash<QString,QString> headers;
    const char *data;
    qint64 length;
    qint64 bytesWritten;

    void connectToHost();

protected:
    QSslSocket *socket;
    int errorCode;

};

#endif // HTTPREQUESTV2_H
