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

#include "wacoderequest.h"

#include "util/utilities.h"

WACodeRequest::WACodeRequest(QString cc, QString in, QString method,
                             QString id, QObject *parent) :
    WARequest(parent)
{
    this->method = "code";

    QSystemNetworkInfo networkInfo(this);
    QSystemInfo systemInfo(this);
    // QSystemDeviceInfo deviceInfo(this);

#ifdef Q_WS_SCRATCHBOX
    QString language = "en";
    QString country = "ZZ";
    QString mcc = "734";
    QString mnc = "002";
#else
    QString language = systemInfo.currentLanguage();
    QString country = systemInfo.currentCountryCode();
    QString mcc = networkInfo.currentMobileCountryCode();
    QString mnc = networkInfo.currentMobileNetworkCode();
#endif

    if (mcc.length() < 3)
        mcc = mcc.rightJustified(3, '0');

    if (mnc.length() < 3)
        mnc = mnc.rightJustified(3, '0');

    addParam("cc", cc);
    addParam("in", in);
    addParam("lc", country.isEmpty() ?  "GB" : country);
    addParam("lg", language.isEmpty() ? "en" : language);
    addParam("sim_mcc", mcc);
    addParam("sim_mnc", mnc);
    addParam("method", method);
    addParam("token", Utilities::getToken(in));
    addParam("id",id);
}
