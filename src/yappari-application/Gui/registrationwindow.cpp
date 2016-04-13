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
#include <QSystemNetworkInfo>

#include "registrationwindow.h"

#include "util/utilities.h"
#include "util/qtmd5digest.h"

#include "Gui/accountinfowindow.h"
#include "Gui/phonenumberwidget.h"
#include "Gui/voiceregistrationwidget.h"

#include "globalconstants.h"

#include "client.h"

RegistrationWindow::RegistrationWindow(QWidget *parent) :
    QMainWindow(parent)
{
    QString cc;

    // Registration
    Utilities::logData("Registration started");
    voiceRegistration = false;

    // Get the country of the phone
    QSystemInfo deviceInfo;
    cc = deviceInfo.currentCountryCode();
    Utilities::logData("Country code: " + cc);

    PhoneNumberWidget *widget = new PhoneNumberWidget(cc,this);

    widget->setAttribute(Qt::WA_DeleteOnClose);

    connect(widget,SIGNAL(accept(QString,QString)),
            this,SLOT(phoneNumberEntered(QString,QString)));

    setCentralWidget(widget);
}

void RegistrationWindow::phoneNumberEntered(const QString &_cc, const QString &_number)
{
    // Phone number and country code entered successfully
    cc = _cc;
    number = _number;

    /* TODO: move to platform/MobileInfo */
    QSystemInfo systemInfo(this);
    QSystemNetworkInfo networkInfo(this);
    QString language = systemInfo.currentLanguage();
    QString country = systemInfo.currentCountryCode();
    mcc = networkInfo.currentMobileCountryCode();
    mnc = networkInfo.currentMobileNetworkCode();
    if (mcc.length() < 3)
        mcc = mcc.rightJustified(3, '0');
    if (mnc.length() < 3)
        mnc = mnc.rightJustified(3, '0');
    /* */

    if (m_id.isEmpty()) {
        m_id = QCryptographicHash::hash(QUuid::createUuid().toString().toAscii(), QCryptographicHash::Md5).toHex();
        //AccountSettings::GetInstance()->setValue("id", m_id);
    }

    //m_device = AccountSettings::GetInstance()->value("device", QString()).toString();
    if (m_device.isEmpty()) {
        m_device = RegTools::getDevice("S40");
        //AccountSettings::GetInstance()->setValue("device", m_device);
    }
    m_useragent = RegTools::getUseragent(m_device);

    qDebug() << "Registering with m_id:" << m_id << "as" << m_device << m_useragent;

    // Start registration
    reg = new WARegistration(this);
    connect(reg, SIGNAL(finished(QVariantMap)),
            this, SLOT(onRegReply(QVariantMap)));

    reg->init(cc, number, mcc, mnc, RegTools::getId(m_id, number), m_useragent, "sms", RegTools::getToken(number));
    reg->codeRequest();

    // Show progress dialog
    progressWidget = new RegistrationProgressWidget(this);
    progressWidget->setAttribute(Qt::WA_DeleteOnClose);
    connect(&registrationTimeoutTimer,SIGNAL(timeout()),
            progressWidget,SLOT(increaseProgress()));
    connect(progressWidget,SIGNAL(timeout()),
            this,SLOT(onSMSRequestTimeout()));

    registrationTimeoutTimer.start(1000);

    setCentralWidget(progressWidget);
}

void RegistrationWindow::onRegReply(const QVariantMap &result)
{
    qDebug() << "Registration::onRegReply" << result;
    QString status = result["status"].toString();
    if (status == "sent" && result["method"].toString() == "sms") {
        qDebug() << "Whatsapp has sent SMS. Starting SMS Listener";
        SMSListener *smsListener = new SMSListener(this);
        connect(smsListener,SIGNAL(codeReceived(QString)),
                this,SLOT(codeReceived(QString)));
    } else if (status == "ok") {
        //AccountSettings::GetInstance()->setValue("login", result["login"]);
        //AccountSettings::GetInstance()->setValue("password", result["pw"]);
        //AccountSettings::GetInstance()->setValue("kind", result["kind"]);
        //AccountSettings::GetInstance()->setValue("expiration", result["expiration"]);
        QVariantMap reply = result;
        reply.insert("cc", this->cc);
        reply.insert("number", this->number);
        emit accept(reply);
        close();
    } else if (status == "fail") {
        QString fail_reason= result["reason"].toString();
        if(fail_reason == "sms_timeout") {
            VoiceRegistrationWidget *widget = new VoiceRegistrationWidget(this);
            connect(widget,SIGNAL(codeReceived(QString)),
                    this,SLOT(codeReceived(QString)));
            connect(widget,SIGNAL(requestCall()),
                    this,SLOT(requestCall()));
            setCentralWidget(widget);
        } else if(fail_reason == "old_version") {
        } else if(fail_reason == "mismatch") {
        } else if(fail_reason == "too_recent") {
        }
    }
}

void RegistrationWindow::onSMSRequestTimeout()
{
    QVariantMap result;
    result.insert("status", "fail");
    result.insert("reason", "sms_timeout");
    this->onRegReply(result);
}

void RegistrationWindow::codeReceived(const QString &code)
{
    reg->enterCode(code);
}

void RegistrationWindow::requestCall()
{
    reg->init(cc, number, mcc, mnc, RegTools::getId(m_id, number), m_useragent, "voice", RegTools::getToken(number));
    reg->codeRequest();
}
