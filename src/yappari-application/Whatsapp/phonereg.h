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

#ifndef PHONEREG_H
#define PHONEREG_H

#include <QObject>
#include <QString>
#include <QThread>

#include "phoneregreply.h"
#include "waexistsrequest.h"
#include "waregrequest.h"
#include "wacoderequest.h"
#include "smslistener.h"

class PhoneReg : public QObject
{
    Q_OBJECT
public:
    explicit PhoneReg(QString cc, QString number, QObject *parent = 0);

signals:
    void finished(PhoneRegReply *);
    void expired(QVariantMap result);

public slots:
    void onExistRequestDone(WARequest *req, bool ok, QVariantMap result);
    void onSelfRequestDone(WARequest *req, bool ok, QVariantMap result);
    void onSMSRequestDone(WARequest *req, bool ok, QVariantMap result);
    void onSMSRequestTimeout();
    void onSMSReceived(SMSListener *listener,QString code);
    void onRegRequestDone(WARequest *req, bool ok, QVariantMap result);
    void onVoiceRequestDone(WARequest *req, bool ok, QVariantMap result);
    void start();
    void startVoiceRequest();
    void startRegRequest(QString code);
    void errorHandler(WARequest *req, int code);
    void sslError(WARequest *req);

private:
    QString number;
    QString cc;
    QString id;

    void startExistRequest();
    void startSelfRequest();
    void startSMSRequest();
};

#endif // PHONEREG_H
