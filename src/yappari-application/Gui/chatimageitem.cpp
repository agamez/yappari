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

#include <QMaemo5Style>

// #include <QtMultimediaKit/QMediaPlayer>
// #include <phonon/MediaObject>
// #include <phonon/AudioOutput>

#include <QDesktopServices>
#include <QPainter>

#include "Whatsapp/util/datetimeutilities.h"
#include "Whatsapp/util/utilities.h"

#include "Dbus/dbusnokiaimageviewerif.h"
#include "Dbus/dbusnokiamediaplayerif.h"

#include "Multimedia/audioplayer.h"

#include "globalconstants.h"
#include "client.h"

#include "chatimageitem.h"
#include "ui_chatimageitem.h"

ChatImageItem::ChatImageItem(FMessage message, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatImageItem)
{
    ui->setupUi(this);

    waiting = false;
    this->message = message;

    setImage();

    ui->progressBar->setRange(0,100);
    ui->progressBar->setValue(0);
    ui->progressBar->hide();

    ui->nameLabel->hide();

    ui->timeLabel->hide();

    connect(ui->viewImageButton,SIGNAL(clicked()),this,SLOT(downloadOrViewImage()));

    setButton();

    setNickname(message);
    setTimestamp(message);
}

void ChatImageItem::setImage()
{
    QImage image;

    if (message.media_wa_type == FMessage::Audio)
        image.load(
                    (message.live)
                        ? "/usr/share/yappari/icons/100x100/voice_overlay_icon.png"
                        : "/usr/share/yappari/icons/100x100/audio_overlay_icon.png"
                  );
    else if (message.data.isEmpty() && message.media_wa_type == FMessage::Video)
        image.load("/usr/share/yappari/icons/100x100/video_overlay_icon.png");
    else
        image = QImage::fromData(message.data);

    if (message.media_wa_type == FMessage::Image ||
        message.media_wa_type == FMessage::Location)
    {
        ui->image->setPixmap(QPixmap::fromImage(image));
        return;
    }


    // We need an overlay for the video and the audio

    // ToDo: PREVIEW MIGHT BE EMPTY!!!!


    QSize size = (image.isNull()) ? QSize(100,100) : image.size();

    QImage imageWithOverlay = QImage(size, QImage::Format_ARGB32_Premultiplied);

    QImage overlayImage;
    if (message.media_wa_type == FMessage::Video)
        overlayImage.load("/usr/share/yappari/icons/100x100/play_overlay_icon.png");

    QPainter painter(&imageWithOverlay);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(image.rect(), Qt::transparent);

    if (!image.isNull())
    {
        painter.setCompositionMode((QPainter::CompositionMode_SourceOver));
        painter.drawImage(0, 0, image);
    }

    if (message.media_wa_type == FMessage::Video)
    {
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(0, 0, overlayImage);
    }

    QFont font = painter.font();
    font.setPointSize(14);
    font.setBold(true);

    painter.setFont(font);

    QPen pen(QColor(Qt::white));
    painter.setPen(pen);

    QRect rect = QRect(0, 80, 100, 20);

    if (message.media_duration_seconds > 0)
    {
        QString duration;
        int minutes = message.media_duration_seconds / 60;
        int seconds = message.media_duration_seconds % 60;
        painter.drawText(rect, Qt::AlignCenter,
                         duration.sprintf("%02d:%02d",
                         minutes,
                         seconds));
    }
    painter.end();
    ui->image->setPixmap(QPixmap::fromImage(imageWithOverlay));


}


void ChatImageItem::setNickname(FMessage message)
{
    bool from_me = message.key.from_me;

    QString from = (from_me)
            ? "You"
            : message.notify_name.replace("<","&lt;").replace(">","&gt;");

    QColor color = QMaemo5Style::standardColor("ActiveTextColor");
    QString nickcolor = Client::nickcolor.isEmpty() ? color.name() : Client::nickcolor;

    QString mycolor = Client::mycolor.isEmpty() ? "#333333" : Client::mycolor;

    ui->groupBox->setStyleSheet("QGroupBox { border-color: "
                                + mycolor +
                                "; border-width: 1px; "
                                "border-style: solid; }");

    QString html = "<style type=\"text/css\">"
                   ".nick { color:" + nickcolor + "; }"
                   "</style>"
                   "<span class=\"nick\">" +
                   Utilities::WATextToHtml(from + ":",32) +
                   "</span>&nbsp;";

    ui->nickname->setTextFormat(Qt::RichText);
    ui->nickname->setAlignment(Qt::AlignCenter|Qt::AlignLeft);
    ui->nickname->setText(html);
}


void ChatImageItem::setTimestamp(FMessage message)
{
    bool from_me = message.key.from_me;

    QString day;

    if (!DateTimeUtilities::isSameDate(message.timestamp,
                                       QDateTime::currentMSecsSinceEpoch()))
        day = DateTimeUtilities::shortDayFormat(message.timestamp);


    QString time = DateTimeUtilities::shortTimeFormat(message.timestamp);

    QColor color = QMaemo5Style::standardColor("SecondaryTextColor");

    QString html = "<div style=\"font-size:18px;color:" + color.name() + "\">"
                    + day + " </div>";

    if (from_me)
    {
        html.append("<div align=\"right\">");

        if (message.status == FMessage::ReceivedByServer)
        {
            waiting = false;
            ui->progressBar->hide();
            html.append(CHECK);
        }
        else if (message.status == FMessage::ReceivedByTarget)
            html.append(DOUBLECHECK);
        else if (message.status == FMessage::Played)
        {
            html.append(VOICEPLAYED);
            html.append(DOUBLECHECK);
        }
        else if (message.status == FMessage::Uploading)
        {
            ui->progressBar->show();
            html.append(UPLOADING);
        }
        else
           html.append(GRAYCHECK);

        html.append("</div>");
    }

    html.append("<div align=\"right\" style=\"font-size:18px;color:" + color.name() + "\">" + time +
                "</div>");
    if (waiting)
    {
        html.append("<div align=\"right\" style=\"font-size:14px;color:" + color.name() + ";\">Please wait...</div>");
        html.append("<div align=\"right\" style=\"font-size:14px;color:" + color.name() + ";\">Might take several minutes</div>");
    }

    ui->timestamp->setTextFormat(Qt::RichText);
    ui->timestamp->setAlignment(Qt::AlignTop);
    ui->timestamp->setText(html);

    // Update local message
    this->message.status = message.status;
    this->message.timestamp = message.timestamp;
}

void ChatImageItem::setButton()
{
    if (message.media_wa_type == FMessage::Location)
    {
        ui->nameLabel->setText(message.media_name);
        ui->nameLabel->show();
        ui->viewImageButton->setText("Go");
        ui->viewImageButton->setEnabled(true);
    }
    else if (message.local_file_uri.isEmpty())
    {
        ui->viewImageButton->setText("Download");
        if (message.media_size <= (Client::automaticDownloadBytes * 1024))
            QTimer::singleShot(50,this,SLOT(downloadOrViewImage()));
    }
    else
    {
        if (message.media_wa_type == FMessage::Image)
            ui->viewImageButton->setText("View");
        else
            ui->viewImageButton->setText("Play");
        ui->viewImageButton->setEnabled(true);
    }
}

ChatImageItem::~ChatImageItem()
{
    delete ui;
}

void ChatImageItem::downloadOrViewImage()
{
    if (message.media_wa_type == FMessage::Location)
    {
        QString url = (message.media_url.isEmpty())
                ? URL_LOCATION_SHARING +
                    QString::number(message.latitude) + "," +
                    QString::number(message.longitude) + "+(" +
                    message.notify_name.replace("<","&lt;").replace(">","&gt;") + ")"
                : message.media_url;
        QDesktopServices::openUrl(QUrl(url));
    }
    else if (!message.local_file_uri.isEmpty())
    {
        QString uri = "file://" + message.local_file_uri;

        QDBusConnection dbus = QDBusConnection::sessionBus();


        switch (message.media_wa_type)
        {
            case FMessage::Audio:
                if (message.live)
                {
                    AudioPlayer *player = new AudioPlayer(this);

                    connect(player,SIGNAL(progress(int)),this,SLOT(updateTime(int)));
                    connect(player,SIGNAL(finished()),this,SLOT(finishedAudioPlay()));

                    ui->viewImageButton->setEnabled(false);

                    player->play(uri);

                    // We need to notificate the sender that we played the audio
                    emit voiceNotePlayed(message);
                    break;
                }

            case FMessage::Video:
                {
                    DBusNokiaMediaPlayerIf *mediaPlayerBus =
                            new DBusNokiaMediaPlayerIf(NOKIA_MEDIAPLAYER_DBUS_NAME,
                                                       NOKIA_MEDIAPLAYER_DBUS_PATH,
                                                       dbus,this);

                    mediaPlayerBus->mime_open(uri);
                }
                break;

            case FMessage::Image:
                {
                    DBusNokiaImageViewerIf *imageViewerBus =
                    new DBusNokiaImageViewerIf(NOKIA_IMAGEVIEWER_DBUS_NAME,
                                               NOKIA_IMAGEVIEWER_DBUS_PATH,
                                               dbus,this);

                    imageViewerBus->mime_open(uri);
                }
                break;
        }
    }
    else
    {
        // Download media

        Utilities::logData("ChatImageItem(): Requesting download");
        ui->viewImageButton->setEnabled(false);
        emit mediaDownload(message);
    }
}

void ChatImageItem::updateTimestamp()
{
    Utilities::logData("ChatImageItem: updateTimestamp()");
    setTimestamp(message);
}

void ChatImageItem::updateProgress(float p)
{
    if (p >= 100.0)
        p = 100;

    ui->progressBar->show();
    ui->progressBar->setValue((int)p);

    if (p == 100 && message.key.from_me)
    {
        waiting = true;
        setTimestamp(message);
    }
}

void ChatImageItem::updateTime(int current)
{
    QString duration;
    int minutes = current / 60;
    int seconds = current % 60;
    ui->timeLabel->show();
    ui->timeLabel->setText(duration.sprintf("%02d:%02d",minutes,seconds));
}

void ChatImageItem::finishedAudioPlay()
{
    ui->timeLabel->hide();
    ui->viewImageButton->setEnabled(true);
}

void ChatImageItem::updateUri(QString uri)
{
    ui->progressBar->hide();
    message.local_file_uri = uri;
    setButton();
}

void ChatImageItem::resetButton()
{
    ui->progressBar->hide();
    ui->viewImageButton->setEnabled(true);
}

void ChatImageItem::updateImage(FMessage message)
{
    this->message = message;
    setImage();
}
