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

#include <QDateTime>

#include "client.h"
#include "funstore.h"
#include "key.h"
#include "fmessage.h"
#include "util/utilities.h"

QMutex FMessage::mutex;

FMessage::FMessage()
{
    generateTimestamp();
    this->status = Unsent;
    this->type = UndefinedMessage;
}

FMessage::FMessage(QString remote_jid, QByteArray data, QString thumb_image)
{
    generateTimestamp();
    generateID();
    init(remote_jid, true);
    this->data = data;
    this->thumb_image = thumb_image;
    this->type = UndefinedMessage;
}

FMessage::FMessage(QString remote_jid, QString data, QString thumb_image)
{
    generateTimestamp();
    generateID();
    init(remote_jid, true);
    this->data = data.toUtf8();
    this->thumb_image = thumb_image;
    this->type = UndefinedMessage;
}

FMessage::FMessage(QString remote_jid, bool from_me, QString id)
{
    generateTimestamp();
    init(remote_jid, from_me);
    this->key.id = id;
    this->type = UndefinedMessage;
}

FMessage::FMessage(QString remote_jid, bool from_me)
{
    generateTimestamp();
    generateID();
    init(remote_jid,from_me);
}

void FMessage::init(QString remote_jid, bool from_me)
{
    // this should be synchronized, I dunno why
    // to not create two messages with the same ID

    this->key.remote_jid = remote_jid;
    this->key.from_me = from_me;
    this->status = Unsent;
}

void FMessage::generateTimestamp()
{
    mutex.lock();
    timestamp = QDateTime::currentMSecsSinceEpoch();
    mutex.unlock();
}

void FMessage::generateID()
{
    this->key.id = QString::number(timestamp / 1000) + "-" +
                   QString::number(Client::seq);

    Client::seq++;
}

void FMessage::setData(QByteArray data)
{
    this->data = data;
}

void FMessage::setData(QString data)
{
    this->data = data.toUtf8();
}

void FMessage::setTimestamp(qint64 timestamp)
{
    this->timestamp = timestamp;
}

void FMessage::setThumbImage(QString thumb_image)
{
    this->thumb_image = thumb_image;
}

void FMessage::setKey(Key k)
{
    this->key = k;
}

void FMessage::setMediaWAType(QString type)
{
    if (type.toLower() == "image")
        media_wa_type = Image;
    else if (type.toLower() == "system")
        media_wa_type = System;
    else if (type.toLower() == "audio")
        media_wa_type = Audio;
    else if (type.toLower() == "video")
        media_wa_type = Video;
    else if (type.toLower() == "vcard")
        media_wa_type = Contact;
    else if (type.toLower() == "location")
        media_wa_type = Location;
    else
        media_wa_type = Undefined;
}

QString FMessage::getMediaWAType()
{
    switch(media_wa_type)
    {
        case Image:
            return "image";

        case System:
            return "system";

        case Audio:
            return "audio";

        case Video:
            return "video";

        case Contact:
            return "vcard";

        case Location:
            return "location";
    }

    return "undefined";
}

