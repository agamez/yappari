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

#include <QUuid>

#include "phonereg.h"
#include "util/utilities.h"
#include "util/qtmd5digest.h"

#include <QTimer>

PhoneReg::PhoneReg(QString cc, QString number, QObject *parent) :
    QObject(parent)
{
    this->cc = cc;
    this->number = number;

    // Generate a new id

    QtMD5Digest digest;
    digest.reset();

    digest.update(QUuid::createUuid().toString().toUtf8());

    QByteArray bytes = digest.digest();

    this->id = QString::fromLatin1(bytes.toHex().constData()).left(20);
}

void PhoneReg::start()
{
    startExistRequest();
}

void PhoneReg::startExistRequest()
{
    Utilities::logData("reg/req/exists/start");
    WAExistsRequest *request = new WAExistsRequest(cc,number,id,this);
    connect(request,SIGNAL(finished(WARequest*,bool, QVariantMap)),
            this,SLOT(onExistRequestDone(WARequest *,bool, QVariantMap)));
    connect(request,SIGNAL(sslError(WARequest*)),
            this,SLOT(sslError(WARequest*)));
    connect(request,SIGNAL(httpError(WARequest*,int)),
            this,SLOT(errorHandler(WARequest*,int)));
    request->getRequest();
}

void PhoneReg::onExistRequestDone(WARequest *req, bool ok, QVariantMap result)
{
    // Dispose of the request
    disconnect(req,0,0,0);
    req->deleteLater();

    if (!ok || !result.contains("status"))
    {
        emit finished(new PhoneRegReply(false,result));
    }

    QString status = result["status"].toString();
    Utilities::logData("reg/req/exists/" + status);

    if (status == "ok")
    {
        Utilities::logData("User " + result["login"].toString() + " is registered.");
        emit finished(new PhoneRegReply(true,result));
    }
    else if (status == "expired")
    {
        emit expired(result);
    }
    else
    {
        Utilities::logData("User " + cc + number + " is not registered.");

        // Register phone
        startSMSRequest();
        // startSelfRequest();
    }

}

void PhoneReg::startSelfRequest()
{
    Utilities::logData("reg/req/self/start");
    WACodeRequest *request = new WACodeRequest(cc,number,"self",id,this);
    connect(request,SIGNAL(finished(WARequest*,bool,QVariantMap)),
            this,SLOT(onSelfRequestDone(WARequest *,bool,QVariantMap)));
    connect(request,SIGNAL(sslError(WARequest*)),
            this,SLOT(sslError(WARequest*)));
    connect(request,SIGNAL(httpError(WARequest*,int)),
            this,SLOT(errorHandler(WARequest*,int)));
    request->getRequest();
}

void PhoneReg::onSelfRequestDone(WARequest *req, bool ok, QVariantMap result)
{
    // Dispose of the request
    disconnect(req,0,0,0);
    req->deleteLater();

    if (!ok || !result.contains("status"))
    {
        emit finished(new PhoneRegReply(false,result));
    }

    QString status = result["status"].toString();
    Utilities::logData("reg/req/self/" + status);

    if (status == "attached" && result.contains("code"))
        startRegRequest(result["code"].toString());
    else if (status == "sent")
    {
        Utilities::logData("Status: 'sent' and this shouldn't have happened");
    }
    else if (status == "ok")
    {
        emit finished(new PhoneRegReply(true,result));
    }
    else
        startSMSRequest();
}

void PhoneReg::startSMSRequest()
{
    Utilities::logData("reg/req/sms/start");
    WACodeRequest *request = new WACodeRequest(cc,number,"sms",id,this);
    connect(request,SIGNAL(finished(WARequest*,bool,QVariantMap)),
            this,SLOT(onSMSRequestDone(WARequest *,bool,QVariantMap)));
    connect(request,SIGNAL(sslError(WARequest*)),
            this,SLOT(sslError(WARequest*)));
    connect(request,SIGNAL(httpError(WARequest*,int)),
            this,SLOT(errorHandler(WARequest*,int)));
    request->getRequest();
}


void PhoneReg::onSMSRequestDone(WARequest *req, bool ok, QVariantMap result)
{
    // Dispose of the request
    disconnect(req,0,0,0);
    req->deleteLater();

    if (!ok || !result.contains("status"))
    {
        emit finished(new PhoneRegReply(false,result));
    }

    QString status = result["status"].toString();
    Utilities::logData("reg/req/sms/" + status);

    if (status == "attached" && result.contains("code"))
        startRegRequest(result["code"].toString());
    else if (status == "sent")
    {
        SMSListener *listener = new SMSListener(this);
        connect(listener,SIGNAL(codeReceived(SMSListener*,QString)),
                this,SLOT(onSMSReceived(SMSListener*,QString)));
    }
    else if (status == "ok")
    {
        emit finished(new PhoneRegReply(true,result));
    }
    else
        emit finished(new PhoneRegReply(false,result));
}

void PhoneReg::onSMSRequestTimeout()
{
    Utilities::logData("onSMSRequestTimeout()");
    QVariantMap result;

    result.insert("reason","sms_timeout");
    emit finished(new PhoneRegReply(false,result));
}

void PhoneReg::onSMSReceived(SMSListener *listener, QString code)
{
    listener->deleteLater();
    startRegRequest(code);
}

void PhoneReg::startRegRequest(QString code)
{
    Utilities::logData("reg/req/register/start");
    WARegRequest *request = new WARegRequest(cc,number,code,id,this);
    connect(request,SIGNAL(finished(WARequest*,bool,QVariantMap)),
            this,SLOT(onRegRequestDone(WARequest *,bool,QVariantMap)));
    connect(request,SIGNAL(sslError(WARequest*)),
            this,SLOT(sslError(WARequest*)));
    connect(request,SIGNAL(httpError(WARequest*,int)),
            this,SLOT(errorHandler(WARequest*,int)));
    request->getRequest();
}

void PhoneReg::onRegRequestDone(WARequest *req, bool ok, QVariantMap result)
{
    // Dispose of the request
    disconnect(req,0,0,0);
    req->deleteLater();

    if (!ok || !result.contains("status"))
    {
        emit finished(new PhoneRegReply(false,result));
    }

    QString status = result["status"].toString();
    Utilities::logData("reg/req/register/" + status);

    if (status == "ok")
    {
        Utilities::logData("User has been registered succesfully.");
        emit finished(new PhoneRegReply(true,result));
    }
    else
    {
        Utilities::logData("Registration failed.");
        emit finished(new PhoneRegReply(false,result));
    }
}

void PhoneReg::startVoiceRequest()
{
    Utilities::logData("reg/req/voice/start");
    WACodeRequest *request = new WACodeRequest(cc,number,"voice",id,this);
    connect(request,SIGNAL(finished(WARequest*,bool,QVariantMap)),
            this,SLOT(onVoiceRequestDone(WARequest *,bool,QVariantMap)));
    connect(request,SIGNAL(sslError(WARequest*)),
            this,SLOT(sslError(WARequest*)));
    connect(request,SIGNAL(httpError(WARequest*,int)),
            this,SLOT(errorHandler(WARequest*,int)));
    request->getRequest();
}

void PhoneReg::onVoiceRequestDone(WARequest *req, bool ok, QVariantMap result)
{
    // Dispose of the request
    disconnect(req,0,0,0);
    req->deleteLater();

    if (!ok || !result.contains("status"))
    {
        emit finished(new PhoneRegReply(false,result));
    }

    QString status = result["status"].toString();
    Utilities::logData("reg/req/voice/" + status);

    if (status == "attached" && !result["code"].toString().isEmpty())
    {
        startRegRequest(result["code"].toString());
    }
    else if (status == "fail")
    {
        emit finished(new PhoneRegReply(false,result));
    }
}

void PhoneReg::errorHandler(WARequest *req, int code)
{
    QVariantMap result;

    // Dispose of the request
    disconnect(req,0,0,0);
    req->deleteLater();

    result.insert("reason","http_error_" + QString::number(code));
    emit finished(new PhoneRegReply(false,result));
}

void PhoneReg::sslError(WARequest *req)
{
    QVariantMap result;

    // Dispose of the request
    disconnect(req,0,0,0);
    req->deleteLater();

    result.insert("reason","ssl_error");
    emit finished(new PhoneRegReply(false,result));
}
