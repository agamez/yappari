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

#include <QCryptographicHash>
#include <QImage>
#include <QBuffer>

#include "mediaupload.h"
#include "formdata.h"
#include "client.h"

#include "util/utilities.h"

#define MAX_SIZE        0xfc00

MediaUpload::MediaUpload(QObject *parent) :
    QObject(parent)
{
}

QString MediaUpload::generateMediaFilename(QString extension)
{
    QByteArray bytes;

    bytes.append(Client::phoneNumber.toUtf8());
    bytes.append(QString::number(QDateTime::currentMSecsSinceEpoch()).toUtf8());

    QByteArray hashed = QCryptographicHash::hash(bytes, QCryptographicHash::Sha1).toHex();
    hashed.append(QString("." + extension).toUtf8());

    return QString::fromUtf8(hashed);
}


void MediaUpload::sendPicture(QString jid, MediaDescriptor descriptor)
{
    QImage picture;

    picture.load(descriptor.localFileUri);

    picture = picture.scaled(100, 100, Qt::KeepAspectRatio);

    int quality = 80;
    do
    {
        descriptor.data.clear();
        QBuffer out(&descriptor.data);
        out.open(QIODevice::WriteOnly);
        picture.save(&out, "JPEG", quality);
        quality -= 10;
    } while ((quality > 10) && descriptor.data.size() > MAX_SIZE);

    sendMedia(jid, descriptor);
}


void MediaUpload::sendVideo(QString jid, MediaDescriptor descriptor)
{
    QString uri = "file://" + descriptor.localFileUri;

    QByteArray hashed = QCryptographicHash::hash(uri.toUtf8(), QCryptographicHash::Md5).toHex();

    QString thumbnail = "/home/user/.thumbnails/cropped/" + QString::fromUtf8(hashed) +
            ".jpeg";

    Utilities::logData("Thumbnail: " + thumbnail);

    // ToDo: Check the thumbnail exists, if it doesn't, request the thumbnail
    // while the media is uploaded

    QImage picture;

    picture.load(thumbnail);

    picture = picture.scaled(100, 100, Qt::KeepAspectRatio);

    int quality = 80;
    do
    {
        descriptor.data.clear();
        QBuffer out(&descriptor.data);
        out.open(QIODevice::WriteOnly);
        picture.save(&out, "JPEG", quality);
        quality -= 10;
    } while ((quality > 10) && descriptor.data.size() > MAX_SIZE);

    sendMedia(jid, descriptor);
}

void MediaUpload::sendMedia(QString jid, FMessage message)
{
    MediaDescriptor descriptor;

    descriptor.waType = (FMessage::MediaWAType) message.media_wa_type;
    descriptor.extension = Utilities::getExtension(message.media_name);
    descriptor.duration = message.media_duration_seconds;
    descriptor.contentType =
            message.media_mime_type.isEmpty() ?
                Utilities::guessMimeType(descriptor.extension) :
                message.media_mime_type;
    descriptor.localFileUri = message.media_name;
    descriptor.url = message.media_url;
    descriptor.upload = (message.status == FMessage::Uploading);
    descriptor.live = message.live;

    switch (descriptor.waType)
    {
        case FMessage::Image:
            sendPicture(jid, descriptor);
            break;

        case FMessage::Video:
            sendVideo(jid, descriptor);
            break;

        case FMessage::Audio:
            sendMedia(jid, descriptor);

        default:
            break;
    }
}

void MediaUpload::sendMedia(QString jid, MediaDescriptor descriptor)
{
    msg = FMessage(jid, true);

    QFile file(descriptor.localFileUri);

    msg.type = FMessage::MediaMessage;
    msg.data = descriptor.data;
    msg.media_size =  file.size();
    msg.media_wa_type = descriptor.waType;
    msg.media_mime_type = descriptor.contentType;
    msg.remote_resource = jid;
    msg.local_file_uri = descriptor.localFileUri;
    msg.media_name = generateMediaFilename(descriptor.extension);
    msg.media_duration_seconds = descriptor.duration;
    msg.live = descriptor.live;

    // This will be overwritten if a media upload is required with
    // the final URL
    msg.media_url = descriptor.url;

    // ToDo: Save tmp file for persistence sending.

    msg.status = FMessage::Uploading;
    emit readyToSendMessage(this,msg);

    if (descriptor.upload)
    {
        uploadMedia();
    }
    else
    {
        msg.status = FMessage::Uploaded;
        emit sendMessage(this,msg);
    }
}

void MediaUpload::uploadMedia()
{
    QList<FormData*> formData;

    FormDataString *str = new FormDataString();
    str->name = "from";
    str->content = Client::myJid;

    formData.append(str);

    FormDataString *str2 = new FormDataString();
    str2->name = "to";
    str2->content = msg.remote_resource;

    formData.append(str2);

    FormDataFile *file = new FormDataFile();
    file->name = "file";
    file->fileName = msg.media_name;
    file->uri = msg.local_file_uri;
    file->contentType = msg.media_mime_type;

    formData.append(file);

    MultiPartUploader *uploader = new MultiPartUploader(this);

    connect(uploader,SIGNAL(finished(MultiPartUploader*,QVariantMap)),
            this,SLOT(finished(MultiPartUploader*,QVariantMap)));
    connect(uploader,SIGNAL(socketError(QAbstractSocket::SocketError)),
            this,SLOT(errorHandler(QAbstractSocket::SocketError)));
    connect(uploader,SIGNAL(progress(float)),
            this,SLOT(updateProgress(float)));
    connect(uploader,SIGNAL(headersReceived(qint64)),
            this,SLOT(headersReceivedHandler(qint64)));
    connect(uploader,SIGNAL(requestSent(qint64)),
            this,SLOT(requestSentHandler(qint64)));


    // uploader->open("https://mms.whatsapp.net/client/iphone/upload.php", formData);
    uploader->open(msg.media_url, formData);
}

void MediaUpload::finished(MultiPartUploader *uploader, QVariantMap dictionary)
{
    disconnect(uploader, 0, 0, 0);
    uploader->deleteLater();

    if (dictionary.contains("url"))
    {
        msg.media_mime_type = dictionary.value("mimetype").toString();
        msg.media_name = dictionary.value("name").toString();
        msg.media_size = dictionary.value("size").toLongLong();
        msg.media_url = dictionary.value("url").toString();
        msg.media_duration_seconds = dictionary.value("duration").toInt();
        msg.status = FMessage::Uploaded;

        Utilities::logData("Upload finished: " + msg.media_name + " size: " +
                           QString::number(msg.media_size));
        Utilities::logData("Url: " + msg.media_url);

        emit sendMessage(this, msg);
    }
    else if (dictionary.contains("error"))
    {
        Utilities::logData("Upload failed: " + dictionary.value("error").toString());
        emit httpError(this);
    }
}


void MediaUpload::errorHandler(QAbstractSocket::SocketError error)
{
    if (error == QAbstractSocket::SslHandshakeFailedError)
    {
        // SSL error is a fatal error
        emit sslError(this);
    }
    else
    {
        // ToDo: Retry here
        Utilities::logData("Upload failed: Socket error " + QString::number(error));
        emit httpError(this);
    }
}

void MediaUpload::updateProgress(float p)
{
    emit progress(msg,p);
}

void MediaUpload::requestSentHandler(qint64 bytes)
{
    emit requestSent(bytes);
}

void MediaUpload::headersReceivedHandler(qint64 bytes)
{
    emit headersReceived(bytes);
}

