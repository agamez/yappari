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

#ifndef CONTACTSYNCER_H
#define CONTACTSYNCER_H

#include <QByteArray>
#include <QMap>

#include "Whatsapp/httprequestv2.h"
#include "contactlist.h"
#include "contactroster.h"

class ContactSyncer : public HttpRequestv2
{
    Q_OBJECT

public:
    ContactSyncer(ContactRoster *roster,
                  QObject *parent = 0);

    bool isSynchronizing();

public slots:
    void sync();
    void syncContact(Contact *c);
    void onResponse();
    void fillBuffer();
    void parseResponse();
    void authResponse();
    void errorHandler(QAbstractSocket::SocketError error);
    void syncNextPhone();
    void increaseUploadCounter(qint64 bytes);
    void increaseDownloadCounter(qint64 bytes);

private:
    ContactRoster *roster;
    ContactList abook;
    QMap<QString,bool> deletedJids;
    QVariantList phoneList;
    int totalPhones, nextSignal;
    bool isSyncing;
    bool syncDataReceived;

    QByteArray readBuffer;
    QByteArray writeBuffer;
    qint64 totalLength;

    void syncAddressBook();

    QByteArray encode(QByteArray bytes);
    int encodeByte(int c);
    QString getAuthResponse(QString nonce);

    void freeAddressBook();
    void getAddressBook();
    void addParam(QString name, QString value);

signals:
    void statusChanged(QString jid, QString status);
    void photoRefresh(QString jid, QString expectedPhotoId, bool largeFormat);
    void syncFinished();
    void progress(int);
    void httpError(int);
    void sslError();
};

#endif // CONTACTSYNCER_H
