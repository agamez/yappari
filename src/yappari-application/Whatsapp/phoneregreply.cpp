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

#include "phoneregreply.h"

PhoneRegReply::PhoneRegReply(bool ok, QVariantMap result, QObject *parent) :
    QObject(parent)
{
    this->ok = ok;
    this->result = result;
}

bool PhoneRegReply::isValid()
{
    return ok;
}

QString PhoneRegReply::getReason()
{
    if (result.contains("reason"))
    {
        QString reason = result["reason"].toString();

        if (reason == "too_recent")
        {
            int minutes = result["retry_after"].toInt() / 60;
            if (minutes < 1)
                minutes = 1;

            return "Registration attempt too recent.\nPlease try again in " +
                    QString::number(minutes) + " minutes.";
        }
        else if (reason == "mismatch")
        {
            return "Registration code is invalid";
        }
        else if (reason.left(11) == "http_error_")
        {
            QString code = reason.right(reason.length()-11);
            return "Couldn't connect to registration server (error " + code + ")";
        }
        else if (reason == "ssl_error")
        {
            return "SSL Handshake Failed.\nPlease check time & date settings on your phone.";
        }

        return "Server response was: " + reason;
    }

    return "Reason unknown";
}
