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

#ifndef FMESSAGE_H
#define FMESSAGE_H

#include <QObject>
#include <QString>
#include <QMutex>

#include "key.h"

class FMessage
{
public:

    enum Status {
        Unsent,
        Uploading,
        Uploaded,
        SentByClient,
        ReceivedByServer,
        ReceivedByTarget,
        NeverSent,
        ServerBounce,
        Played
    };

    enum ContentType {
        ComposingMessage,
        PausedMessage,
        BodyMessage,
        MediaMessage,
        UndefinedMessage,
        RequestMediaMessage
    };

    enum MediaWAType {
        Undefined,
        Image,
        Audio,
        Video,
        Contact,
        Location,
        Divider,
        System,
        Voice
    };

    FMessage();
    FMessage(QString remote_jid, bool from_me);
    FMessage(QString remote_jid, bool from_me, QString id);
    FMessage(QString remote_jid, QByteArray data, QString thumb_image);
    FMessage(QString remote_jid, QString data, QString thumb_image);

    void setKey(Key k);
    void setRemoteJid(QString remote_jid);
    void setData(QByteArray data);
    void setData(QString data);
    void setTimestamp(qint64 timestamp);
    void setThumbImage(QString thumb_image);
    void setMediaWAType(QString type);
    QString getMediaWAType();

    QByteArray data;
    QString thumb_image;
    qint64 timestamp;
    Key key;
    Status status;
    QString notify_name;
    QString remote_resource;
    ContentType type;

    double latitude;
    double longitude;

    bool live;

    qint32 media_duration_seconds;
    QString media_mime_type;
    QString media_name;
    qint64 media_size;
    QString media_url;
    int media_wa_type;

    QString local_file_uri;


private:
    static QMutex mutex;

    void init(QString remote_jid, bool from_me);
    void generateTimestamp();
    void generateID();
};



#endif // FMESSAGE_H
