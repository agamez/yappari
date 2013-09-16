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

#include <QMaemo5InformationBox>
#include <QMessageBox>

#include "registrationwindow.h"

#include "Whatsapp/util/utilities.h"

#include "Gui/accountinfowindow.h"
#include "Gui/phonenumberwidget.h"
#include "Gui/voiceregistrationwidget.h"

#include "client.h"

RegistrationWindow::RegistrationWindow(QWidget *parent) :
    QMainWindow(parent)
{
    QString cc;

    // Registration
    Utilities::logData("Registration started");
    voiceRegistration = false;

    // Get the country of the phone
#ifndef Q_WS_SCRATCHBOX
    QSystemInfo deviceInfo;
    cc = deviceInfo.currentCountryCode();
#else
    cc = "US";
#endif
    Utilities::logData("Country code: " + cc);

    PhoneNumberWidget *widget = new PhoneNumberWidget(cc,this);

    widget->setAttribute(Qt::WA_DeleteOnClose);

    connect(widget,SIGNAL(accept(QString,QString)),
            this,SLOT(phoneNumberEntered(QString,QString)));

    setCentralWidget(widget);
}

void RegistrationWindow::phoneNumberEntered(QString cc, QString number)
{
    // Phone number and country code entered successfully

    this->cc = cc;
    this->number = number;

    // Start registration
    reg = new PhoneReg(cc,number);

    connect(reg,SIGNAL(finished(PhoneRegReply*)),
            this,SLOT(registrationFinished(PhoneRegReply *)));

    connect(reg,SIGNAL(expired(QVariantMap)),
            this,SLOT(expired(QVariantMap)));


    // Show progress dialog
    progressWidget = new RegistrationProgressWidget(this);

    progressWidget->setAttribute(Qt::WA_DeleteOnClose);

    connect(&registrationTimeoutTimer,SIGNAL(timeout()),
            progressWidget,SLOT(increaseProgress()));

    registrationTimeoutTimer.start(1000);

    connect(progressWidget,SIGNAL(timeout()),reg,SLOT(onSMSRequestTimeout()));

    setCentralWidget(progressWidget);

    QTimer::singleShot(500, reg, SLOT(start()));
}

void RegistrationWindow::registrationFinished(PhoneRegReply *reply)
{
    registrationTimeoutTimer.stop();

    if (reply->isValid())
    {
        QVariantMap result = reply->result;
        result.insert("cc", this->cc);
        result.insert("number", this->number);
        emit accept(result);

        reg->deleteLater();
        close();
    }
    else if (!voiceRegistration)
    {
        voiceRegistration = true;

        VoiceRegistrationWidget *widget = new VoiceRegistrationWidget(this);

        connect(widget,SIGNAL(requestCall()),this,SLOT(requestCall()));
        connect(widget,SIGNAL(codeReceived(QString)),reg,SLOT(startRegRequest(QString)));

        setCentralWidget(widget);
    } else {

        QMessageBox msg(this);

        msg.setText("Registration couldn't be completed.\n" + reply->getReason());
        msg.exec();
    }

    reply->deleteLater();
}

void RegistrationWindow::requestCall()
{
    QMaemo5InformationBox::information(this,"A call is being requested");

    reg->startVoiceRequest();
}

void RegistrationWindow::expired(QVariantMap result)
{
    registrationTimeoutTimer.stop();

    QMaemo5InformationBox::information(this,
                                       "Your account has expired.\n\n"\
                                       "You have to pay to WhatsApp Inc. to renew your account",
                                       QMaemo5InformationBox::NoTimeout);

    // Refresh some values

    Client::phoneNumber = result["login"].toString();
    Client::accountstatus = result["status"].toString();
    Client::kind = result["kind"].toString();
    Client::expiration = result["expiration"].toString();

    AccountInfoWindow *window = new AccountInfoWindow(this);
    window->setAttribute(Qt::WA_Maemo5StackedWindow);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowFlags(window->windowFlags() | Qt::Window);
    window->show();

    connect(window,SIGNAL(destroyed()),this,SLOT(close()));
}
